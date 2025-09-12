/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define BTN1_Pin GPIO_PIN_0
#define BTN1_GPIO_Port GPIOC
#define BTN2_Pin GPIO_PIN_1
#define BTN2_GPIO_Port GPIOC
#define P2L4_Pin GPIO_PIN_0
#define P2L4_GPIO_Port GPIOA
#define P2L3_Pin GPIO_PIN_1
#define P2L3_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define P2L2_Pin GPIO_PIN_4
#define P2L2_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define P1L4_Pin GPIO_PIN_6
#define P1L4_GPIO_Port GPIOA
#define P1L3_Pin GPIO_PIN_7
#define P1L3_GPIO_Port GPIOA
#define P2L1_Pin GPIO_PIN_0
#define P2L1_GPIO_Port GPIOB
#define P2L8_Pin GPIO_PIN_1
#define P2L8_GPIO_Port GPIOB
#define P1L5_Pin GPIO_PIN_2
#define P1L5_GPIO_Port GPIOB
#define L2_Pin GPIO_PIN_10
#define L2_GPIO_Port GPIOB
#define L1P6_Pin GPIO_PIN_12
#define L1P6_GPIO_Port GPIOB
#define P2L5_Pin GPIO_PIN_13
#define P2L5_GPIO_Port GPIOB
#define P2L6_Pin GPIO_PIN_14
#define P2L6_GPIO_Port GPIOB
#define P2L7_Pin GPIO_PIN_15
#define P2L7_GPIO_Port GPIOB
#define G1_Pin GPIO_PIN_6
#define G1_GPIO_Port GPIOC
#define P1L1_Pin GPIO_PIN_7
#define P1L1_GPIO_Port GPIOC
#define G2_Pin GPIO_PIN_8
#define G2_GPIO_Port GPIOC
#define L1_Pin GPIO_PIN_8
#define L1_GPIO_Port GPIOA
#define L1P7_Pin GPIO_PIN_11
#define L1P7_GPIO_Port GPIOA
#define P1L8_Pin GPIO_PIN_12
#define P1L8_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define L3_Pin GPIO_PIN_4
#define L3_GPIO_Port GPIOB
#define P1L2_Pin GPIO_PIN_6
#define P1L2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
