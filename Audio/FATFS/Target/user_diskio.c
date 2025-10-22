#include "user_diskio.h"
#include "main.h"
#include "spi.h"
#include "ff_gen_drv.h"
#include "diskio.h"
#include <string.h>

extern SPI_HandleTypeDef hspi2;
#define SD_SPI hspi2

#define SD_CS_GPIO_Port CS_SD_GPIO_Port
#define SD_CS_Pin       CS_SD_Pin
#define CS_LOW()        HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET)
#define CS_HIGH()       HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET)

static volatile DSTATUS Stat = STA_NOINIT;
static BYTE CardType;

static void deselect(void) { CS_HIGH(); HAL_SPI_Transmit(&SD_SPI, (uint8_t[]){0xFF}, 1, 10); }
static int select(void) { CS_LOW(); uint8_t b; for (uint32_t t=0;t<500;t++){ HAL_SPI_TransmitReceive(&SD_SPI,(uint8_t[]){0xFF},&b,1,10); if(b==0xFF) return 1; HAL_Delay(1);} deselect(); return 0; }
static void send_clocks(void){ uint8_t buf[10]; memset(buf,0xFF,10); CS_HIGH(); HAL_SPI_Transmit(&SD_SPI,buf,10,100); }

static BYTE send_cmd(BYTE cmd, DWORD arg){
  BYTE n,res;
  if(cmd & 0x80){cmd &= 0x7F; res = send_cmd(CMD55,0); if(res>1) return res;}
  deselect();
  if(!select()) return 0xFF;
  uint8_t buf[6]={cmd,arg>>24,arg>>16,arg>>8,arg,0x01};
  if(cmd==CMD0) buf[5]=0x95;
  if(cmd==CMD8) buf[5]=0x87;
  HAL_SPI_Transmit(&SD_SPI,buf,6,100);
  for(n=0;n<10;n++){HAL_SPI_TransmitReceive(&SD_SPI,(uint8_t[]){0xFF},&res,1,100); if(!(res&0x80)) break;}
  return res;
}

DSTATUS USER_initialize(BYTE pdrv){
  (void)pdrv;
  BYTE ty=0;
  send_clocks();
  if(send_cmd(CMD0,0)==1){
    BYTE ocr[4];
    if(send_cmd(CMD8,0x1AA)==1){
      for(int i=0;i<4;i++) HAL_SPI_TransmitReceive(&SD_SPI,(uint8_t[]){0xFF},&ocr[i],1,100);
      if(ocr[2]==0x01&&ocr[3]==0xAA){
        for(uint32_t t=0;t<1000;t++){if(send_cmd(ACMD41,1UL<<30)==0) break; HAL_Delay(1);}
        if(send_cmd(CMD58,0)==0){for(int i=0;i<4;i++) HAL_SPI_TransmitReceive(&SD_SPI,(uint8_t[]){0xFF},&ocr[i],1,100);
          ty=(ocr[0]&0x40)?6:2;}
      }
    } else {
      BYTE cmd;
      if(send_cmd(ACMD41,0)<=1){ty=2;cmd=ACMD41;}else{ty=1;cmd=CMD1;}
      for(uint32_t t=0;t<1000;t++){if(send_cmd(cmd,0)==0) break; HAL_Delay(1);}
      if(send_cmd(CMD16,512)!=0) ty=0;
    }
  }
  CardType=ty; deselect();
  if(ty) Stat&=~STA_NOINIT; else Stat=STA_NOINIT;
  return Stat;
}

DSTATUS USER_status(BYTE pdrv){(void)pdrv; return Stat;}

DRESULT USER_read(BYTE pdrv, BYTE*buff, LBA_t sector, UINT count){
  (void)pdrv;
  if(!count) return RES_PARERR;
  if(Stat&STA_NOINIT) return RES_NOTRDY;
  if(!(CardType&4)) sector*=512;
  if(count==1){
    if(send_cmd(CMD17,sector)==0){
      uint8_t token;
      for(uint32_t t=0;t<200;t++){HAL_SPI_TransmitReceive(&SD_SPI,(uint8_t[]){0xFF},&token,1,100); if(token==0xFE) break;}
      if(token!=0xFE){deselect(); return RES_ERROR;}
      HAL_SPI_Receive(&SD_SPI,buff,512,100);
      uint8_t crc[2]; HAL_SPI_Receive(&SD_SPI,crc,2,100);
      deselect(); return RES_OK;
    }
  }
  deselect(); return RES_ERROR;
}

#if FF_FS_READONLY==0
DRESULT USER_write(BYTE pdrv,const BYTE*buff,LBA_t sector,UINT count){
  (void)pdrv; if(!count) return RES_PARERR; if(Stat&STA_NOINIT) return RES_NOTRDY;
  if(!(CardType&4)) sector*=512;
  if(count==1){
    if(send_cmd(CMD24,sector)==0){
      uint8_t token=0xFE; HAL_SPI_Transmit(&SD_SPI,&token,1,10);
      HAL_SPI_Transmit(&SD_SPI,(uint8_t*)buff,512,500);
      uint8_t crc[2]={0xFF,0xFF}; HAL_SPI_Transmit(&SD_SPI,crc,2,100);
      uint8_t resp; HAL_SPI_TransmitReceive(&SD_SPI,(uint8_t[]){0xFF},&resp,1,100);
      deselect(); if((resp&0x1F)==0x05) return RES_OK;
    }
  }
  deselect(); return RES_ERROR;
}
#endif

DRESULT USER_ioctl(BYTE pdrv,BYTE cmd,void*buff){
  (void)pdrv; if(Stat&STA_NOINIT) return RES_NOTRDY;
  switch(cmd){case CTRL_SYNC: if(select()){deselect();return RES_OK;}break; default:break;}
  return RES_PARERR;
}

Diskio_drvTypeDef USER_Driver={
  USER_initialize,
  USER_status,
  USER_read,
#if FF_FS_READONLY==0
  USER_write,
#endif
  USER_ioctl
};
