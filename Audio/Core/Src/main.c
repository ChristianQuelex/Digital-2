/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : DAC audio (TIM6+DMA) + SPI2 + FATFS (SD)
  ******************************************************************************
  * NOTA: El código entre bloques USER CODE no se pierde al regenerar CubeMX.
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "fatfs.h"      // generado por CubeMX (FATFS/App)
#include "ff.h"         // FatFs core
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
  float    freq_hz;    // 0 => silencio
  uint8_t  dur_beats;  // 4=negra, 8=blanca, 2=corchea...
} Note;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define AUDIO_FS_HZ        20000U   // Fs = 20 kHz
#define WT_SIZE            256U
#define AUDIO_BUF_SAMPLES  512U     // doble buffer
#define DAC_MAX_12B        4095U
#define DAC_OFFSET_12B     2048U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac1;
SPI_HandleTypeDef hspi2;
TIM_HandleTypeDef htim6;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* Audio engine */
static uint16_t audio_buf[AUDIO_BUF_SAMPLES];
static uint16_t wavetable[WT_SIZE];
static uint32_t phase_q16 = 0;
static uint32_t phase_inc_q16 = 0;
static uint8_t  audio_running = 0;
static float    current_freq_hz = 0.0f;

/* FATFS */
static FATFS   s_fs;      // objeto de sistema de archivos
static uint8_t sd_mounted = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM6_Init(void);
static void MX_SPI2_Init(void);
/* USER CODE BEGIN PFP */
/* Audio */
static void WT_InitSine(void);
static inline void Audio_SetFrequency(float freq_hz);
static void Audio_Start(void);
static void Audio_Stop(void);
static void FillHalf(uint32_t off, uint32_t len);
/* SD/FATFS */
static void SD_Mount(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* ==== Audio helpers ==== */
static void WT_InitSine(void) {
  for (uint32_t i = 0; i < WT_SIZE; ++i) {
    float s = sinf(2.0f * 3.14159265359f * (float)i / (float)WT_SIZE);
    uint16_t v = (uint16_t)((s * 0.49f + 0.5f) * (float)DAC_MAX_12B);
    wavetable[i] = v;
  }
}

static inline void Audio_SetFrequency(float freq_hz) {
  current_freq_hz = freq_hz;
  if (freq_hz <= 0.0f) {
    phase_inc_q16 = 0;
    return;
  }
  float step = (freq_hz * (float)WT_SIZE) / (float)AUDIO_FS_HZ;
  phase_inc_q16 = (uint32_t)(step * 65536.0f);
}

static void FillHalf(uint32_t off, uint32_t len) {
  if (phase_inc_q16 == 0) {
    for (uint32_t i = 0; i < len; ++i) audio_buf[off + i] = DAC_OFFSET_12B;
    return;
  }
  for (uint32_t i = 0; i < len; ++i) {
    uint32_t idx = (phase_q16 >> 16) & (WT_SIZE - 1);
    audio_buf[off + i] = wavetable[idx];
    phase_q16 += phase_inc_q16;
  }
}

static void Audio_Start(void) {
  if (audio_running) return;

  WT_InitSine();
  phase_q16 = 0;
  for (uint32_t i = 0; i < AUDIO_BUF_SAMPLES; ++i) {
    uint32_t idx = (phase_q16 >> 16) & (WT_SIZE - 1);
    audio_buf[i] = wavetable[idx];
    phase_q16 += phase_inc_q16;
  }

  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1,
                    (uint32_t*)audio_buf, AUDIO_BUF_SAMPLES, DAC_ALIGN_12B_R);
  HAL_TIM_Base_Start(&htim6);
  audio_running = 1;
}

static void Audio_Stop(void) {
  if (!audio_running) return;
  HAL_TIM_Base_Stop(&htim6);
  HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
  HAL_DAC_Stop(&hdac, DAC_CHANNEL_1);
  audio_running = 0;
}

/* DMA callbacks del DAC – mantener dentro de USER CODE para no perderlos */
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdacx) {
  (void)hdacx;
  FillHalf(0, AUDIO_BUF_SAMPLES/2);
}
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdacx) {
  (void)hdacx;
  FillHalf(AUDIO_BUF_SAMPLES/2, AUDIO_BUF_SAMPLES/2);
}

/* ==== FATFS helpers ==== */
static void SD_Mount(void) {
  FRESULT fr = f_mount(&s_fs, "", 1);  // montar en unidad por defecto
  sd_mounted = (fr == FR_OK);
}

/* USER CODE END 0 */

int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_DAC_Init();
  MX_TIM6_Init();
  MX_SPI2_Init();
  MX_FATFS_Init();   /* genera los “glue” de FatFs */

  /* USER CODE BEGIN 2 */
  /* Monta SD (requiere alimentación + SPI2 conectado) */
  SD_Mount();

  /* Arranca motor de audio en silencio */
  Audio_SetFrequency(0.0f);
  Audio_Start();

  /* Tono de prueba breve si la SD está montada */
  if (sd_mounted) {
    Audio_SetFrequency(440.0f);  // A4
    HAL_Delay(300);
    Audio_SetFrequency(0.0f);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  while (1)
  {
    /* USER CODE BEGIN WHILE */
    // Coloca aquí tu lógica de reproducción/carga desde SD
    HAL_Delay(10);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4; // SYSCLK=84MHz
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1| RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;  // 84 MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;    // 42 MHz (TIMx x2 = 84)
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;    // 84 MHz
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
}

/* ================== Inits generados (mantener firma) ================== */

static void MX_DAC_Init(void)
{
  DAC_ChannelConfTypeDef sConfig = {0};
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK) { Error_Handler(); }

  /* TRIGGER por TIM6 TRGO */
  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK) {
    Error_Handler();
  }
}

static void MX_TIM6_Init(void)
{
  /* Fs=20 kHz con TIM6clk=84 MHz:
     84e6/(PSC+1)/(ARR+1) = 20000  -> PSC=83 (1 MHz), ARR=49 */
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 83;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 49;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK) { Error_Handler(); }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }
}

static void MX_SPI2_Init(void)
{
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;                 // usas PB12 como GPIO CS
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256; // seguro para SD
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK) { Error_Handler(); }
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits   = UART_STOPBITS_1;
  huart2.Init.Parity     = UART_PARITY_NONE;
  huart2.Init.Mode       = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK) { Error_Handler(); }
}

static void MX_DMA_Init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();
  /* DMA1 Stream5 (DAC1 CH1 típico) */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* PB12: CS de la SD en alto por defecto */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */
/* Si quieres printf por UART2, agrega retarget en syscalls.c o aquí */
/* USER CODE END 4 */

/**
  * @brief  Error Handler
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1) { }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file; (void)line;
}
#endif
