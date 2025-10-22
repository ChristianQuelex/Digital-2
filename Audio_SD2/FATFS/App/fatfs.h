#ifndef __FATFS_H__
#define __FATFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ff.h"
#include "ff_gen_drv.h"

extern uint8_t retUSER;
extern char USERPath[4];
extern FATFS USERFatFS;
extern FIL USERFile;

void MX_FATFS_Init(void);

#ifdef __cplusplus
}
#endif
#endif
