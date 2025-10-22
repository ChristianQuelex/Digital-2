/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for FATFS applications
  ******************************************************************************
  */
/* USER CODE END Header */

#include "fatfs.h"

/* USER CODE BEGIN Includes */
#include "ff_gen_drv.h"
#include "user_diskio.h" /* De FATFS/Target */
/* USER CODE END Includes */

uint8_t retUSER;    /* Return value for USER */
char USERPath[4];   /* USER logical drive path */
FATFS USERFatFS;    /* File system object for USER logical drive */
FIL USERFile;       /* File object for USER */

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  Initializes the FatFs module
  * @retval None
  */
void MX_FATFS_Init(void)
{
  /*## FatFS: Link the USER driver ###########################*/
  retUSER = FATFS_LinkDriver(&USER_Driver, USERPath);
  /* USERPath quedará como "0:" típicamente */
}

/* USER CODE BEGIN Application */
/* USER CODE END Application */
