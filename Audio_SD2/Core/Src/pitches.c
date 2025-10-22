#include "pitches.h"

/* Imperial March (frase principal) */
const Note IMPERIAL_MARCH[] = {
  {NOTE_A4,8}, {NOTE_A4,8}, {NOTE_A4,8}, {NOTE_F4,6}, {NOTE_C5,2},
  {NOTE_A4,8}, {NOTE_F4,6}, {NOTE_C5,2}, {NOTE_A4,12},

  {NOTE_E5,8}, {NOTE_E5,8}, {NOTE_E5,8}, {NOTE_F5,6}, {NOTE_C5,2},
  {NOTE_GS4,8}, {NOTE_F4,6}, {NOTE_C5,2}, {NOTE_A4,12},
};
const unsigned int IMPERIAL_MARCH_LEN = sizeof(IMPERIAL_MARCH)/sizeof(IMPERIAL_MARCH[0]);

/* Harry Potter (Hedwig's Theme) - versión compacta */
const Note HARRY_POTTER[] = {
  {NOTE_B4,3}, {NOTE_E5,4}, {NOTE_G5,2}, {NOTE_FS5,3}, {NOTE_E5,6}, {NOTE_B5,3}, {NOTE_A5,12}, {NOTE_FS5,12},
  {NOTE_E5,4}, {NOTE_G5,2}, {NOTE_FS5,3}, {NOTE_DS5,6}, {NOTE_F5,3}, {NOTE_B4,12},
  {NOTE_B4,3}, {NOTE_E5,4}, {NOTE_G5,2}, {NOTE_FS5,3}, {NOTE_E5,6}, {NOTE_B5,3}, {NOTE_D6,6}, {NOTE_CS6,3},
  {NOTE_C6,6}, {NOTE_GS5,3}, {NOTE_C6,4}, {NOTE_B5,2}, {NOTE_AS5,3}, {NOTE_AS4,6}, {NOTE_G5,3}, {NOTE_E5,12},
};
const unsigned int HARRY_POTTER_LEN = sizeof(HARRY_POTTER)/sizeof(HARRY_POTTER[0]);
