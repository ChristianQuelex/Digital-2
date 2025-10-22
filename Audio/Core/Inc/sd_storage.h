#ifndef SD_STORAGE_H
#define SD_STORAGE_H

#include <stdint.h>
#include "ff.h"        // Tipos FRESULT, FIL, etc.
#include "pitches.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CS_SD_GPIO_Port
  #define CS_SD_GPIO_Port GPIOB
#endif
#ifndef CS_SD_Pin
  #define CS_SD_Pin       GPIO_PIN_12
#endif

FRESULT SD_InitAndMount(void);
FRESULT SD_LoadMelodyCSV(const char *path, Note *mel, uint32_t max_notes, uint32_t *out_len);
FRESULT SD_SaveMelodyCSV(const char *path, const Note *mel, uint32_t count);

#ifdef __cplusplus
}
#endif
#endif /* SD_STORAGE_H */
