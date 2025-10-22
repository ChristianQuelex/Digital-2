/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.c
  * @brief   Disk I/O driver for microSD in SPI mode (STM32 HAL) + UART debug.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "ff_gen_drv.h"
#include "main.h"
#include <string.h>
#include "stm32f4xx_hal.h"

/* === Config === */
#define SD_DUMMY_BYTE           0xFFU
#define SD_TOKEN_START_BLOCK    0xFEU
#define SD_READY_TOKEN          0xFFU

#define SD_CMD_TIMEOUT_MS       500U
#define SD_RW_TIMEOUT_MS        500U
#define SD_INIT_TIMEOUT_MS      1000U

#define CT_MMC    0x01
#define CT_SD1    0x02
#define CT_SD2    0x04
#define CT_BLOCK  0x08

extern SPI_HandleTypeDef hspi2;
extern UART_HandleTypeDef huart2;  /* para logs */

/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;
static BYTE CardType = 0;

/* ---------- helpers ---------- */
static inline void CS_LOW(void)  { HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET); }
static inline void CS_HIGH(void) { HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);   }

static void ulog(const char* s) {
  HAL_UART_Transmit(&huart2, (uint8_t*)s, (uint16_t)strlen(s), 200);
}
static void ulog_hex(const char* pfx, uint8_t v, const char* sfx) {
  char b[16];
  int n = snprintf(b, sizeof(b), "%s%02X%s", pfx, v, sfx);
  HAL_UART_Transmit(&huart2, (uint8_t*)b, (uint16_t)n, 200);
}

static void     sd_spi_tx(const uint8_t* data, uint16_t len)   { HAL_SPI_Transmit(&hspi2,(uint8_t*)data,len,HAL_MAX_DELAY); }
static void     sd_spi_rx(uint8_t* data, uint16_t len)         { HAL_SPI_Receive (&hspi2, data,len,HAL_MAX_DELAY); }
static uint8_t  sd_spi_txrx(uint8_t byte)                      { uint8_t rx=0xFF; HAL_SPI_TransmitReceive(&hspi2,&byte,&rx,1,HAL_MAX_DELAY); return rx; }

static void sd_send_dummy_clocks(uint8_t nbytes) { CS_HIGH(); uint8_t ff=0xFF; for (uint8_t i=0;i<nbytes;i++) sd_spi_tx(&ff,1); }

static int sd_wait_ready(uint32_t timeout_ms) {
  uint32_t t0 = HAL_GetTick();
  do { if (sd_spi_txrx(SD_DUMMY_BYTE)==SD_READY_TOKEN) return 1; }
  while ((HAL_GetTick()-t0) < timeout_ms);
  return 0;
}

static uint8_t sd_send_cmd(uint8_t cmd, uint32_t arg)
{
  uint8_t buf[6];
  CS_HIGH(); sd_spi_txrx(SD_DUMMY_BYTE); CS_LOW();

  if (!sd_wait_ready(SD_CMD_TIMEOUT_MS)) { CS_HIGH(); return 0xFF; }

  buf[0] = 0x40 | cmd;
  buf[1] = (uint8_t)(arg >> 24);
  buf[2] = (uint8_t)(arg >> 16);
  buf[3] = (uint8_t)(arg >> 8);
  buf[4] = (uint8_t)(arg);
  buf[5] = 0x01;                    /* dummy CRC */
  if (cmd==0) buf[5]=0x95;
  if (cmd==8) buf[5]=0x87;

  sd_spi_tx(buf,6);

  uint8_t r1;
  for (int i=0;i<8;i++){ r1=sd_spi_txrx(SD_DUMMY_BYTE); if(!(r1&0x80)) break; }
  return r1;
}

static int sd_rcvr_datablock(uint8_t* buff, uint32_t btr)
{
  uint32_t t0 = HAL_GetTick();
  uint8_t token;
  do {
    token = sd_spi_txrx(SD_DUMMY_BYTE);
    if (token==SD_TOKEN_START_BLOCK) break;
  } while ((HAL_GetTick()-t0)<SD_RW_TIMEOUT_MS);
  if (token!=SD_TOKEN_START_BLOCK) return 0;
  sd_spi_rx(buff,(uint16_t)btr);
  sd_spi_txrx(SD_DUMMY_BYTE); sd_spi_txrx(SD_DUMMY_BYTE);
  return 1;
}

static int sd_xmit_datablock(const uint8_t* buff, uint8_t token)
{
  if (!sd_wait_ready(SD_RW_TIMEOUT_MS)) return 0;
  sd_spi_tx(&token,1);
  if (token!=0xFD){
    sd_spi_tx((uint8_t*)buff,512);
    uint8_t crc[2]={0xFF,0xFF}; sd_spi_tx(crc,2);
    uint8_t resp = sd_spi_txrx(SD_DUMMY_BYTE);
    if ((resp & 0x1F)!=0x05) return 0;
  }
  return 1;
}

/* --------- FatFs interface ---------- */
DSTATUS USER_initialize(BYTE pdrv)
{
  if (pdrv != 0) return STA_NOINIT;

  ulog("SD init...\r\n");
  CS_HIGH();

  /* Baja velocidad para init */
  __HAL_SPI_DISABLE(&hspi2);
  MODIFY_REG(hspi2.Instance->CR1, SPI_CR1_BR, SPI_BAUDRATEPRESCALER_256);
  __HAL_SPI_ENABLE(&hspi2);

  sd_send_dummy_clocks(10);

  CardType = 0;
  Stat = STA_NOINIT;

  /* CMD0 */
  uint8_t r = sd_send_cmd(0,0);
  ulog_hex("CMD0 R1=0x", r, "\r\n");
  if (r != 0x01) { ulog("CMD0 fail\r\n"); CS_HIGH(); return Stat; }

  /* CMD8 */
  uint8_t ocr[4]={0};
  r = sd_send_cmd(8, 0x000001AA);
  ulog_hex("CMD8 R1=0x", r, "\r\n");
  if (r == 0x01) {
    for (int i=0;i<4;i++) ocr[i]=sd_spi_txrx(0xFF);
    ulog_hex("CMD8 OCR3=0x", ocr[2], " "); ulog_hex("OCR4=0x", ocr[3], "\r\n");
    if (ocr[2]==0x01 && ocr[3]==0xAA) {
      /* ACMD41 HCS loop */
      uint32_t t0 = HAL_GetTick();
      do {
        sd_send_cmd(55,0);
        r = sd_send_cmd(41, 1UL<<30);
        if (r==0) break;
      } while ((HAL_GetTick()-t0) < SD_INIT_TIMEOUT_MS);
      ulog_hex("ACMD41 R1=0x", r, "\r\n");

      if (r==0 && sd_send_cmd(58,0)==0){
        for (int i=0;i<4;i++) ocr[i]=sd_spi_txrx(0xFF);
        ulog_hex("CMD58 OCR1=0x", ocr[0], "\r\n");
        CardType = (ocr[0]&0x40)? (CT_SD2|CT_BLOCK) : CT_SD2;
      }
    }
  } else {
    /* SDv1/MMC */
    uint8_t cmd;
    if (sd_send_cmd(55,0)<=1 && sd_send_cmd(41,0)<=1) { CardType=CT_SD1; cmd=41; }
    else { CardType=CT_MMC; cmd=1; }
    uint32_t t0 = HAL_GetTick();
    do {
      r = sd_send_cmd(cmd,0);
      if (r==0) break;
    } while ((HAL_GetTick()-t0)<SD_INIT_TIMEOUT_MS);
    ulog_hex("v1/MMC R1=0x", r, "\r\n");
    if (r==0 && sd_send_cmd(16,512)==0) { /* ok */ } else { CardType=0; }
  }

  CS_HIGH(); sd_spi_txrx(0xFF);

  if (CardType) {
    Stat &= ~STA_NOINIT;
    ulog("SD init OK\r\n");
  } else {
    Stat = STA_NOINIT;
    ulog("SD init FAIL\r\n");
  }

  /* Sube velocidad operación */
  __HAL_SPI_DISABLE(&hspi2);
  MODIFY_REG(hspi2.Instance->CR1, SPI_CR1_BR, SPI_BAUDRATEPRESCALER_16);
  __HAL_SPI_ENABLE(&hspi2);

  return Stat;
}

DSTATUS USER_status(BYTE pdrv)
{
  if (pdrv != 0) return STA_NOINIT;
  return Stat;
}

DRESULT USER_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
  if (pdrv!=0 || !count) return RES_PARERR;
  if (Stat & STA_NOINIT)  return RES_NOTRDY;

  if (!(CardType & CT_BLOCK)) sector *= 512U;

  uint8_t r;
  if (count==1){
    r = sd_send_cmd(17, sector);
    if (r==0 && sd_rcvr_datablock(buff,512)) { CS_HIGH(); sd_spi_txrx(0xFF); return RES_OK; }
  } else {
    r = sd_send_cmd(18, sector);
    if (r==0){
      do {
        if (!sd_rcvr_datablock(buff,512)) break;
        buff += 512;
      } while (--count);
      sd_send_cmd(12,0);
    }
  }
  CS_HIGH(); sd_spi_txrx(0xFF);
  return RES_ERROR;
}

#if _USE_WRITE == 1
DRESULT USER_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
  if (pdrv!=0 || !count) return RES_PARERR;
  if (Stat & STA_NOINIT)  return RES_NOTRDY;

  if (!(CardType & CT_BLOCK)) sector *= 512U;

  uint8_t r;
  if (count==1){
    r = sd_send_cmd(24, sector);
    if (r==0 && sd_xmit_datablock(buff,0xFE)) { CS_HIGH(); sd_spi_txrx(0xFF); return RES_OK; }
  } else {
    r = sd_send_cmd(25, sector);
    if (r==0){
      do {
        if (!sd_xmit_datablock(buff,0xFC)) break;
        buff += 512;
      } while (--count);
      sd_xmit_datablock(0,0xFD);
    }
  }
  CS_HIGH(); sd_spi_txrx(0xFF);
  return RES_ERROR;
}
#endif

#if _USE_IOCTL == 1
DRESULT USER_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
  if (pdrv != 0) return RES_PARERR;
  if (Stat & STA_NOINIT) return RES_NOTRDY;

  DRESULT res = RES_ERROR;
  switch (cmd) {
    case CTRL_SYNC:
      if (sd_wait_ready(SD_RW_TIMEOUT_MS)) res = RES_OK;
      break;
    case GET_SECTOR_SIZE:
      *(WORD*)buff = 512; res = RES_OK; break;
    case GET_BLOCK_SIZE:
      *(DWORD*)buff = 1;  res = RES_OK; break;
    case GET_SECTOR_COUNT:
      res = RES_PARERR;   break;
    default:
      res = RES_PARERR;   break;
  }
  CS_HIGH(); sd_spi_txrx(0xFF);
  return res;
}
#endif

/* Driver table required by FatFs */
Diskio_drvTypeDef  USER_Driver =
{
  USER_initialize,
  USER_status,
  USER_read,
#if  _USE_WRITE
  USER_write,
#endif
#if  _USE_IOCTL == 1
  USER_ioctl,
#endif
};
