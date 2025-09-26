/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "stm32f4xx_it.h"

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

/******************************************************************************/
/* Cortex-M4 Processor Interruption and Exception Handlers                    */
/******************************************************************************/
void NMI_Handler(void){ while (1){} }
void HardFault_Handler(void){ while (1){} }
void MemManage_Handler(void){ while (1){} }
void BusFault_Handler(void){ while (1){} }
void UsageFault_Handler(void){ while (1){} }
void SVC_Handler(void){}
void DebugMon_Handler(void){}
void PendSV_Handler(void){}
void SysTick_Handler(void){ HAL_IncTick(); }

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/******************************************************************************/

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart1);
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);
}
