#include "pitches.h"

/* -------------------------------------------------------------------------- */
/* TRON Legacy - Main Theme (Piano) 0:00–1:06 — línea melódica                */
/* Cuantización: negra ≈ 255 ms (usa tempo_ms = 255 en main.c)                */
/* Figuras: 2=corchea, 3=corchea., 4=negra, 6=negra., 8=blanca, 12=blanca.    */
/* -------------------------------------------------------------------------- */
const Note TRON_PIANO[] = {
  {REST,12},
  {NOTE_G4,6}, {NOTE_D5,6}, {NOTE_G4,4}, {NOTE_G4,2}, {NOTE_D5,8}, {NOTE_D5,2},
  {NOTE_G4,4}, {NOTE_G4,4}, {NOTE_D5,6}, {NOTE_D5,3}, {NOTE_G4,3}, {NOTE_F4,12},

  {NOTE_D5,4}, {NOTE_DS4,3}, {NOTE_D5,2}, {NOTE_G4,4}, {NOTE_G4,2}, {NOTE_G4,6},
  {NOTE_G4,4}, {NOTE_D5,3}, {NOTE_G4,3}, {NOTE_G4,4}, {NOTE_D5,6}, {NOTE_D5,2},

  {NOTE_G4,6}, {NOTE_G4,2}, {NOTE_GS4,3}, {NOTE_G4,6}, {NOTE_G4,4}, {NOTE_DS4,2},
  {NOTE_G4,12}, {NOTE_G4,2}, {NOTE_FS4,12}, {NOTE_GS3,2}, {NOTE_FS4,6},

  {NOTE_FS3,3}, {NOTE_FS3,2}, {NOTE_FS4,6}, {NOTE_FS4,2}, {NOTE_G4,4}, {NOTE_G4,3},
  {NOTE_AS3,4}, {NOTE_GS4,4}, {NOTE_G4,2}, {NOTE_G4,6}, {NOTE_G4,2}, {NOTE_G4,4},

  {NOTE_D5,3}, {NOTE_G4,6}, {NOTE_G4,2}, {NOTE_GS4,3}, {NOTE_G4,6}, {NOTE_G4,4},
  {NOTE_DS4,2}, {NOTE_G4,12}, {NOTE_G4,2}, {NOTE_FS4,12}, {NOTE_GS3,2}, {NOTE_FS4,6},

  {NOTE_FS3,3}, {NOTE_FS3,2}, {NOTE_FS4,6}, {NOTE_G4,2}, {NOTE_G4,4}, {NOTE_G4,3},
  {NOTE_AS3,4}, {NOTE_GS4,4}, {NOTE_G4,2}, {NOTE_G4,6}, {NOTE_G4,2}, {NOTE_G4,4},

  {NOTE_D5,3}, {NOTE_G4,6}, {NOTE_G4,2}, {NOTE_GS4,3}, {NOTE_G4,6}, {NOTE_G4,4},
  {NOTE_DS4,2}, {NOTE_G4,12}, {NOTE_G4,2}, {NOTE_FS4,12}, {NOTE_GS3,2}, {NOTE_FS4,6},

  {NOTE_FS3,3}, {NOTE_FS3,2}, {NOTE_FS4,6}, {NOTE_FS4,2}, {NOTE_G4,4}, {NOTE_G4,3},
  {NOTE_AS3,4}, {NOTE_GS4,4}, {NOTE_G4,2}, {NOTE_G4,6}, {NOTE_G4,2}, {NOTE_G4,4},

  {NOTE_D5,3}, {NOTE_G4,6}, {NOTE_G4,2}, {NOTE_GS4,3}, {NOTE_G4,6}, {NOTE_G4,4},
  {NOTE_DS4,2}, {NOTE_G4,12}, {NOTE_G4,2}, {NOTE_FS4,12}, {NOTE_GS3,2}, {NOTE_FS4,6},

  {NOTE_FS3,3}, {NOTE_FS3,2}, {NOTE_FS4,6}, {NOTE_G4,2}, {NOTE_G4,4}, {NOTE_G4,3},
  {NOTE_AS3,4}, {NOTE_GS4,4}, {NOTE_G4,2}, {NOTE_G4,6}, {NOTE_G4,2}, {NOTE_G4,4},

  {NOTE_D5,3}, {NOTE_G4,6}, {NOTE_G4,2}, {NOTE_GS4,3}, {NOTE_G4,6}, {NOTE_G4,4},
  {NOTE_DS4,2}, {NOTE_G4,12}, {NOTE_G4,2}, {NOTE_FS4,12}
};

const unsigned int TRON_PIANO_LEN = sizeof(TRON_PIANO)/sizeof(TRON_PIANO[0]);
