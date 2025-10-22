#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* Botón de usuario (si tu Nucleo lo tiene) */
#define B1_Pin              GPIO_PIN_13
#define B1_GPIO_Port        GPIOC

/* UART2 */
#define USART_TX_Pin        GPIO_PIN_2
#define USART_TX_GPIO_Port  GPIOA
#define USART_RX_Pin        GPIO_PIN_3
#define USART_RX_GPIO_Port  GPIOA

/* LED de la placa */
#define LD2_Pin             GPIO_PIN_5
#define LD2_GPIO_Port       GPIOA

/* SD Card (SPI2) */
#define SD_CS_Pin           GPIO_PIN_12
#define SD_CS_GPIO_Port     GPIOB
#define SPI2_SCK_Pin        GPIO_PIN_13
#define SPI2_SCK_GPIO_Port  GPIOB
#define SPI2_MISO_Pin       GPIO_PIN_14
#define SPI2_MISO_GPIO_Port GPIOB
#define SPI2_MOSI_Pin       GPIO_PIN_15
#define SPI2_MOSI_GPIO_Port GPIOB

/* Pines SWD */
#define TMS_Pin             GPIO_PIN_13
#define TMS_GPIO_Port       GPIOA
#define TCK_Pin             GPIO_PIN_14
#define TCK_GPIO_Port       GPIOA
#define SWO_Pin             GPIO_PIN_3
#define SWO_GPIO_Port       GPIOB

/* === Trigger de reproducción ===
   PC2 -> Disparo de MELODIA2 (activo en nivel bajo) */
#define TRIG1_Pin           GPIO_PIN_2
#define TRIG1_GPIO_Port     GPIOC

/* (Opcional) Si más adelante quieres reactivar PC3/PC4, descomenta:
#define TRIG2_Pin           GPIO_PIN_3
#define TRIG2_GPIO_Port     GPIOC
#define TRIG3_Pin           GPIO_PIN_4
#define TRIG3_GPIO_Port     GPIOC
*/

#ifdef __cplusplus
}
#endif
#endif /* __MAIN_H */
