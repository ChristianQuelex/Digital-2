/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fatfs.h
  * @brief   Header for fatfs applications
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __FATFS_H__
#define __FATFS_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "ff.h"            /* FatFs core types */
#include "diskio.h"        /* FatFs lower layer API */
#include "ff_gen_drv.h"    /* FatFs generic driver */

/* Exported variables --------------------------------------------------------*/
extern uint8_t retUSER;     /* Return value for USER */
extern char USERPath[4];    /* USER logical drive path */
extern FATFS USERFatFS;     /* File system object for USER logical drive */
extern FIL USERFile;        /* File object for USER */

/* Exported functions prototypes ---------------------------------------------*/
void MX_FATFS_Init(void);
void MX_FATFS_DeInit(void);

/* USER CODE BEGIN Prototypes */
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /* __FATFS_H__ */
