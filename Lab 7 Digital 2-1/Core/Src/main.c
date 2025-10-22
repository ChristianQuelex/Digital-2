/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fatfs_sd.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
FATFS fs;
FATFS *pfs;
FIL fil;
FRESULT fres;
DWORD fre_clust;
uint32_t totalSpace, freeSpace;

char     buffer[100];     // lectura de líneas desde archivo
uint8_t  bufferUART[1];   // 1 byte recibido por UART

// ¡OJO con mayúsculas/minúsculas! (tu tercer archivo es "dibujo3b.txt")
const char* images[3] = { "dibujo1B.txt", "dibujo2B.txt", "dibujo3b.txt" };
uint8_t state = 0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */
static void menu(void);
static void showIm(const char *image);
static const char* fresult_str(FRESULT r);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();

  HAL_Delay(500);

  // RX por interrupción (1 byte)
  HAL_UART_Receive_IT(&huart2, bufferUART, 1);

  // Menú inicial
  menu();

  // Monta primero en 0:, y si falla intenta 1:
  fres = f_mount(&fs, "0:", 0);
  if (fres == FR_OK) {
    HAL_UART_Transmit(&huart2, (uint8_t*)"Micro SD card is mounted (0:)\r\n", strlen("Micro SD card is mounted (0:)\r\n"), 200);
  } else {
    char msg[64];
    snprintf(msg, sizeof(msg), "Mount 0: error: %s\r\n", fresult_str(fres));
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 200);

    fres = f_mount(&fs, "1:", 0);
    if (fres == FR_OK) {
      HAL_UART_Transmit(&huart2, (uint8_t*)"Micro SD card is mounted (1:)\r\n", strlen("Micro SD card is mounted (1:)\r\n"), 200);
    } else {
      snprintf(msg, sizeof(msg), "Mount 1: error: %s\r\n", fresult_str(fres));
      HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 200);
    }
  }

  while (1)
  {
    if (state == 1) {
      state = 0;
      showIm(images[0]);
    } else if (state == 2) {
      state = 0;
      showIm(images[1]);
    } else if (state == 3) {
      state = 0;
      showIm(images[2]);
    } else if (state == 255) {
      state = 0;
      fres = f_mount(NULL, "", 1); // desmonta cualquier unidad activa
      if (fres == FR_OK)
        HAL_UART_Transmit(&huart2, (uint8_t*)"File unmounted\r\n", strlen("File unmounted\r\n"), 200);
      else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Unmount error: %s\r\n", fresult_str(fres));
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 200);
      }
    } else {
      state = 0;
    }
    HAL_Delay(1);
  }
}

/* ----------------- Clock ----------------- */
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
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }
}

/* ----------------- SPI ----------------- */
static void MX_SPI1_Init(void)
{
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK) { Error_Handler(); }
}

/* ----------------- UART ----------------- */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK) { Error_Handler(); }
}

/* ----------------- GPIO ----------------- */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(SD_SS_GPIO_Port, SD_SS_Pin, GPIO_PIN_SET);

  // Botón de usuario (si aplica)
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  // Chip Select de la SD
  GPIO_InitStruct.Pin = SD_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(SD_SS_GPIO_Port, &GPIO_InitStruct);
}

/* ===================== USER CODE ===================== */

static void menu(void)
{
  HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n****Menu****\r\n", strlen("\r\n****Menu****\r\n"), 200);
  HAL_UART_Transmit(&huart2, (uint8_t*)"1. Abrir el archivo Dibujo1\r\n", strlen("1. Abrir el archivo Dibujo1\r\n"), 200);
  HAL_UART_Transmit(&huart2, (uint8_t*)"2. Abrir el archivo Dibujo2\r\n", strlen("2. Abrir el archivo Dibujo2\r\n"), 200);
  HAL_UART_Transmit(&huart2, (uint8_t*)"3. Abrir el archivo Dibujo3\r\n", strlen("3. Abrir el archivo Dibujo3\r\n"), 200);
  HAL_UART_Transmit(&huart2, (uint8_t*)"0. Salir y desmontar\r\n",        strlen("0. Salir y desmontar\r\n"),        200);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  (void)huart;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    uint8_t c = bufferUART[0];

    // Ignora CR/LF que envía el Enter del monitor serie
    if (c == '\r' || c == '\n') {
      HAL_UART_Receive_IT(&huart2, bufferUART, 1);
      return;
    }

    // Eco del carácter válido
    HAL_UART_Transmit(&huart2, &c, 1, 200);
    HAL_UART_Transmit(&huart2, (uint8_t*)" <-- Recibido\r\n", strlen(" <-- Recibido\r\n"), 200);

    // Decodificación
    if      (c == '1' && state == 0) state = 1;
    else if (c == '2' && state == 0) state = 2;
    else if (c == '3' && state == 0) state = 3;
    else if (c == '0' && state == 0) state = 255;
    else HAL_UART_Transmit(&huart2, (uint8_t*)"No hay archivo :(\r\n", strlen("No hay archivo :(\r\n"), 250);

    // Reinicia la recepción
    HAL_UART_Receive_IT(&huart2, bufferUART, 1);
  }
}

/* Intenta abrir con 0:/, luego 1:/ y por último sin prefijo.
   Reporta el código y nombre del error (FRESULT). */
static void showIm(const char *image)
{
  char path[40];
  char msg[96];

  // 1) 0:/archivo
  snprintf(path, sizeof(path), "0:/%s", image);
  fres = f_open(&fil, (TCHAR*)path, FA_READ);
  if (fres != FR_OK) {
    snprintf(msg, sizeof(msg), "Open 0: %s -> %s\r\n", image, fresult_str(fres));
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 200);

    // 2) 1:/archivo
    snprintf(path, sizeof(path), "1:/%s", image);
    fres = f_open(&fil, (TCHAR*)path, FA_READ);
    if (fres != FR_OK) {
      snprintf(msg, sizeof(msg), "Open 1: %s -> %s\r\n", image, fresult_str(fres));
      HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 200);

      // 3) archivo sin prefijo
      fres = f_open(&fil, (TCHAR*)image, FA_READ);
      if (fres != FR_OK) {
        snprintf(msg, sizeof(msg), "Open no prefix: %s -> %s\r\n", image, fresult_str(fres));
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 200);
        menu();
        return;
      }
    }
  }

  HAL_UART_Transmit(&huart2, (uint8_t*)"File opened for reading\r\n", strlen("File opened for reading\r\n"), 200);

  while (f_gets(buffer, sizeof(buffer), &fil))
  {
    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 200);
  }

  fres = f_close(&fil);
  if (fres == FR_OK)
    HAL_UART_Transmit(&huart2, (uint8_t*)"File closed\r\n", strlen("File closed\r\n"), 200);
  else {
    snprintf(msg, sizeof(msg), "Close error: %s\r\n", fresult_str(fres));
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 200);
  }

  menu();
}

/* Traducción de FRESULT a texto para depurar en terminal */
static const char* fresult_str(FRESULT r)
{
  switch (r) {
    case FR_OK:                   return "FR_OK";
    case FR_DISK_ERR:             return "FR_DISK_ERR";
    case FR_INT_ERR:              return "FR_INT_ERR";
    case FR_NOT_READY:            return "FR_NOT_READY";
    case FR_NO_FILE:              return "FR_NO_FILE";
    case FR_NO_PATH:              return "FR_NO_PATH";
    case FR_INVALID_NAME:         return "FR_INVALID_NAME";
    case FR_DENIED:               return "FR_DENIED";
    case FR_EXIST:                return "FR_EXIST";
    case FR_INVALID_OBJECT:       return "FR_INVALID_OBJECT";
    case FR_WRITE_PROTECTED:      return "FR_WRITE_PROTECTED";
    case FR_INVALID_DRIVE:        return "FR_INVALID_DRIVE";
    case FR_NOT_ENABLED:          return "FR_NOT_ENABLED";
    case FR_NO_FILESYSTEM:        return "FR_NO_FILESYSTEM";
    case FR_MKFS_ABORTED:         return "FR_MKFS_ABORTED";
    case FR_TIMEOUT:              return "FR_TIMEOUT";
    case FR_LOCKED:               return "FR_LOCKED";
    case FR_NOT_ENOUGH_CORE:      return "FR_NOT_ENOUGH_CORE";
    case FR_TOO_MANY_OPEN_FILES:  return "FR_TOO_MANY_OPEN_FILES";
    case FR_INVALID_PARAMETER:    return "FR_INVALID_PARAMETER";
    default:                      return "FR_Unknown";
  }
}

/* ===================================================== */
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
#endif /* USE_FULL_ASSERT */
