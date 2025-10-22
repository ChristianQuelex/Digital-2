/* USER CODE BEGIN Header */
/**
  ******************************************************************************
//UNIVERSIDAD DEL VALLE DE GUATEMALA
//IE3054: Electrónica Digital 2
//Archivo: Laboratorio 7 – Almacenamiento SD
//AUTOR: Christian Quelex
//Laboratorio 7
//HARDWARE: STEM32 - NUCELO 64
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fatfs_sd.h"
#include "string.h"
#include "stdio.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
SPI_HandleTypeDef hspi1;
FATFS fs;
FATFS *pfs;
FIL fil;
FRESULT fres;
DWORD fre_clust;
uint32_t totalSpace, freeSpace;
char buffer[100];
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
//	---> Configuracion de las variables
char received;			// Selección del usuario
int salida = 0;			//Se configura la variable de salida
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Función para enviar texto por UART
void transmit_uart(char *string) {
	uint8_t len = strlen(string);
	HAL_UART_Transmit(&huart2, (uint8_t*) string, len, 200);
}

// Función para probar comunicación SPI
void test_spi_communication(void) {
    transmit_uart("\n--- TEST COMUNICACION SPI ---\n");

    // Configurar CS como output
    HAL_GPIO_WritePin(SD_SS_GPIO_Port, SD_SS_Pin, GPIO_PIN_SET);
    HAL_Delay(10);

    // Test 1: Enviar bytes de prueba por SPI
    uint8_t test_byte = 0xFF;
    uint8_t received_byte = 0;

    HAL_GPIO_WritePin(SD_SS_GPIO_Port, SD_SS_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);

    HAL_SPI_TransmitReceive(&hspi1, &test_byte, &received_byte, 1, 100);

    HAL_GPIO_WritePin(SD_SS_GPIO_Port, SD_SS_Pin, GPIO_PIN_SET);

    char spi_msg[100];
    sprintf(spi_msg, "SPI Test - Enviado: 0x%02X, Recibido: 0x%02X\n", test_byte, received_byte);
    transmit_uart(spi_msg);

    if (received_byte == 0xFF) {
        transmit_uart("✓ Comunicacion SPI funcionando\n");
    } else {
        transmit_uart("✗ Problema con comunicacion SPI\n");
    }

    transmit_uart("--- FIN TEST SPI ---\n");
}

// Función para verificar el estado de la SD a bajo nivel
int check_sd_status(void) {
    transmit_uart("Inicializando disco...\n");
    DSTATUS status = disk_initialize(0);

    char status_msg[100];
    sprintf(status_msg, "Estado disco: %d\n", status);
    transmit_uart(status_msg);

    if (status == 0) {
        transmit_uart("✓ Disco inicializado correctamente\n");
        return 1;
    } else if (status & STA_NOINIT) {
        transmit_uart("✗ Disco NO inicializado\n");
        return 0;
    } else if (status & STA_NODISK) {
        transmit_uart("✗ No hay disco presente\n");
        return 0;
    } else if (status & STA_PROTECT) {
        transmit_uart("✗ Disco protegido contra escritura\n");
        return 0;
    } else {
        transmit_uart("✗ Error desconocido del disco\n");
        return 0;
    }
}

//Función para desplegar las opciones dle Menu
void menu() {
	transmit_uart("\n=== MENU PRINCIPAL ===\n");
    transmit_uart("1. Leer dibujo1.txt\n");
    transmit_uart("2. Leer dibujo2.txt\n");
    transmit_uart("3. Leer dibujo3.txt\n");
    transmit_uart("4. Diagnostico completo SD\n");
    transmit_uart("5. Reinicializar SD\n");
    transmit_uart("6. Test Hardware SPI\n");
    transmit_uart("7. Salir\n");
    transmit_uart("Selecciona una opcion: ");
}

// Función para leer un archivo de texto desde la SD y enviarlo por UART
void read_text_file(const char *filename) {
    char full_filename[50];
    sprintf(full_filename, "%s.txt", filename);

    transmit_uart("\nIntentando abrir: ");
    transmit_uart(full_filename);
    transmit_uart("\n");

    // Verificar primero que la SD esté lista
    if (!check_sd_status()) {
        transmit_uart("ERROR: SD no esta lista. Use opcion 5 para reinicializar.\n");
        return;
    }

    fres = f_open(&fil, full_filename, FA_READ);
    if (fres == FR_OK) {
        transmit_uart("✓ Archivo abierto correctamente\n");
        transmit_uart("=== CONTENIDO ===\n");

        // Leer y mostrar el contenido del archivo
        while (f_gets(buffer, sizeof(buffer), &fil) != NULL) {
            transmit_uart(buffer);
        }

        fres = f_close(&fil);
        if (fres == FR_OK) {
            transmit_uart("\n✓ Archivo cerrado correctamente\n");
        } else {
            char error_msg[50];
            sprintf(error_msg, "\n✗ Error al cerrar archivo: %d\n", fres);
            transmit_uart(error_msg);
        }
    } else {
        char error_msg[100];
        sprintf(error_msg, "✗ Error al abrir archivo: %d\n", fres);
        transmit_uart(error_msg);
    }
    transmit_uart("=== FIN ARCHIVO ===\n");
}

// Función para diagnosticar completamente la SD
void full_sd_diagnostic(void) {
    transmit_uart("\n*** DIAGNOSTICO COMPLETO SD ***\n");

    // 1. Test comunicación SPI
    test_spi_communication();

    // 2. Verificar estado físico del disco
    transmit_uart("\n1. VERIFICACION FISICA DEL DISCO:\n");
    if (check_sd_status()) {
        transmit_uart("✓ SD detectada fisicamente\n");
    } else {
        transmit_uart("✗ FALLA: SD no detectada fisicamente\n");
        transmit_uart("SOLUCIONES:\n");
        transmit_uart("1. Verifique que la SD este INSERTADA correctamente\n");
        transmit_uart("2. Verifique conexiones del modulo SD\n");
        transmit_uart("3. Verifique que la SD este FORMATEADA en FAT32\n");
        transmit_uart("4. Use la opcion 6 para test de hardware\n");
        return;
    }

    // 3. Verificar montaje del sistema de archivos
    transmit_uart("\n2. SISTEMA DE ARCHIVOS:\n");
    fres = f_mount(&fs, "", 1);
    if (fres == FR_OK) {
        transmit_uart("✓ Sistema de archivos montado\n");
    } else {
        char error_msg[100];
        sprintf(error_msg, "✗ FALLA: Error montando sistema de archivos: %d\n", fres);
        transmit_uart(error_msg);

        if (fres == FR_NO_FILESYSTEM) {
            transmit_uart("   - La SD no tiene sistema de archivos FAT32\n");
            transmit_uart("   - FORMATEE la SD en FAT32 desde una computadora\n");
        } else if (fres == FR_NOT_READY) {
            transmit_uart("   - El disco no esta listo\n");
            transmit_uart("   - Verifique conexiones de alimentacion\n");
        }
        return;
    }

    // 4. Verificar espacio
    transmit_uart("\n3. ESPACIO EN DISCO:\n");
    FATFS *fs_ptr = &fs;
    fres = f_getfree("", &fre_clust, &fs_ptr);
    if (fres == FR_OK) {
        totalSpace = (uint32_t)((fs_ptr->n_fatent - 2) * fs_ptr->csize * 0.5);
        freeSpace = (uint32_t)(fre_clust * fs_ptr->csize * 0.5);

        char space_info[100];
        sprintf(space_info, "✓ Total: %lu KB, Libre: %lu KB\n", totalSpace, freeSpace);
        transmit_uart(space_info);
    } else {
        transmit_uart("✗ Error verificando espacio\n");
    }

    // 5. Verificar archivos
    transmit_uart("\n4. VERIFICACION DE ARCHIVOS:\n");
    const char* test_files[] = {"dibujo1.txt", "dibujo2.txt", "dibujo3.txt"};
    int files_found = 0;

    for (int i = 0; i < 3; i++) {
        FIL file;
        FRESULT file_result = f_open(&file, test_files[i], FA_READ);
        if (file_result == FR_OK) {
            char msg[50];
            sprintf(msg, "✓ %s encontrado\n", test_files[i]);
            transmit_uart(msg);
            f_close(&file);
            files_found++;
        } else {
            char msg[50];
            sprintf(msg, "✗ %s no encontrado (Error: %d)\n", test_files[i], file_result);
            transmit_uart(msg);
        }
    }

    transmit_uart("\n*** FIN DIAGNOSTICO ***\n");
}

// Función para reinicializar la SD
void reinitialize_sd(void) {
    transmit_uart("\n*** REINICIALIZANDO SD ***\n");

    // 1. Desmontar si estaba montado
    f_mount(NULL, "", 0);
    transmit_uart("1. Sistema desmontado\n");

    // 2. Pequeña pausa
    HAL_Delay(1000);
    transmit_uart("2. Esperando 1 segundo...\n");

    // 3. Reinicializar disco a bajo nivel
    transmit_uart("3. Reinicializando disco...\n");

    // Intentar múltiples veces
    int attempts = 0;
    DSTATUS disk_status;

    for (attempts = 0; attempts < 5; attempts++) {
        disk_status = disk_initialize(0);
        if (disk_status == 0) {
            transmit_uart("✓ Disco reinicializado correctamente\n");
            break;
        }
        HAL_Delay(500);
    }

    if (disk_status != 0) {
        transmit_uart("✗ Error reinicializando disco despues de 5 intentos\n");
        transmit_uart("   Verifique conexiones fisicas de la SD\n");
        return;
    }

    // 4. Remontar sistema de archivos
    transmit_uart("4. Montando sistema de archivos...\n");
    fres = f_mount(&fs, "", 1);

    if (fres == FR_OK) {
        transmit_uart("✓ Sistema de archivos montado\n");
        transmit_uart("✓ SD lista para usar\n");
    } else {
        char error_msg[100];
        sprintf(error_msg, "✗ Error montando sistema: %d\n", fres);
        transmit_uart(error_msg);
    }

    transmit_uart("*** FIN REINICIALIZACION ***\n");
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  // Esperar inicialización de hardware
  HAL_Delay(2000);

  transmit_uart("\n\n========================================\n");
  transmit_uart("LABORATORIO 7 - SISTEMA DE ARCHIVOS SD\n");
  transmit_uart("========================================\n");
  transmit_uart("Inicializando sistema...\n");

  // Dar tiempo para que el hardware se estabilice
  transmit_uart("Esperando inicializacion de hardware...\n");
  HAL_Delay(1000);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	 menu();

	 //Se espera hasta que el usuario ingrese opción
	 HAL_UART_Receive(&huart2, (uint8_t*)&received, 1, HAL_MAX_DELAY);

	 switch (received) {
	 	 case '1':
	 	     read_text_file("dibujo1");
	 	     break;

	 	 case '2':
	 	     read_text_file("dibujo2");
	 	     break;

	 	 case '3':
	 	    read_text_file("dibujo3");
	 	    break;

	 	 case '4':
	 	    full_sd_diagnostic();
	 	    break;

	 	 case '5':
	 	    reinitialize_sd();
	 	    break;

	 	 case '6':
	 	    test_spi_communication();
	 	    break;

	 	 case '7':
	 	    transmit_uart("\nSaliendo del sistema...\n");
	 	    f_mount(NULL, "", 1);
	 	    transmit_uart("SD desmontada. Sistema finalizado.\n");
	 	    while(1) {
	 	        HAL_Delay(1000);
	 	    }
	 	    break;

	 	 default:
	 	     transmit_uart("\nOpcion Invalida. Ingresa 1-7\n");
	 }

	 // Pequeña pausa antes de mostrar el menú again
	 HAL_Delay(100);

	 //Se reinicia la opción del Usuario
	 received = 0;

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
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
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SD_SS_GPIO_Port, SD_SS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_SS_Pin */
  GPIO_InitStruct.Pin = SD_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(SD_SS_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
