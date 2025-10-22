#ifndef PITCHES_H
#define PITCHES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ===== Notas en Hz (float) =====
   Incluye las octavas que pide la melodía TRON: G3/GS3/AS3, F3, FS3, ...
   y en 4ª/5ª: DS4, F4, FS4, G4, GS4, D5. */
#define NOTE_C3   130.81f
#define NOTE_CS3  138.59f
#define NOTE_D3   146.83f
#define NOTE_DS3  155.56f
#define NOTE_E3   164.81f
#define NOTE_F3   174.61f
#define NOTE_FS3  185.00f
#define NOTE_G3   196.00f
#define NOTE_GS3  207.65f
#define NOTE_A3   220.00f
#define NOTE_AS3  233.08f
#define NOTE_B3   246.94f

#define NOTE_C4   261.63f
#define NOTE_CS4  277.18f
#define NOTE_D4   293.66f
#define NOTE_DS4  311.13f
#define NOTE_E4   329.63f
#define NOTE_F4   349.23f
#define NOTE_FS4  369.99f
#define NOTE_G4   392.00f
#define NOTE_GS4  415.30f
#define NOTE_A4   440.00f
#define NOTE_AS4  466.16f
#define NOTE_B4   493.88f

#define NOTE_C5   523.25f
#define NOTE_CS5  554.37f
#define NOTE_D5   587.33f
#define NOTE_DS5  622.25f
#define NOTE_E5   659.25f
#define NOTE_F5   698.46f
#define NOTE_FS5  739.99f
#define NOTE_G5   783.99f
#define NOTE_GS5  830.61f
#define NOTE_A5   880.00f
#define NOTE_AS5  932.33f
#define NOTE_B5   987.77f

#define REST      0.0f

typedef struct {
  float    freq_hz;   // frecuencia (Hz); REST = 0.0f
  uint8_t  dur_beats; // 4=negra; 2=corchea; 6=negra.; 8=blanca; 12=blanca.
} Note;

/* Declaración de la melodía TRON (definida en pitches.c) */
extern const Note TRON_PIANO[];
extern const unsigned int TRON_PIANO_LEN;

#ifdef __cplusplus
}
#endif
#endif /* PITCHES_H */
