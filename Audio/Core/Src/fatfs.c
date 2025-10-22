#include "fatfs.h"
#include "ff_gen_drv.h"
#include "user_diskio.h"

uint8_t retUSER;
char USERPath[4];
FATFS USERFatFS;
FIL USERFile;

void MX_FATFS_Init(void)
{
  retUSER = FATFS_LinkDriver(&USER_Driver, USERPath);
}

void MX_FATFS_DeInit(void)
{
  FATFS_UnLinkDriver(USERPath);
}
