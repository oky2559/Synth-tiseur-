#ifndef MATH_H
#define MATH_H

#include <stdio.h>
#include <stdint.h>

#define PI 3.14159265358979323846
#define SAMPLING_RATE 44100

typedef struct {
    int type;
    float start;
    float duration;
    float freq;
} SoundNote;

float generate_sample(int type, float phase);

float apply_automatic_envelope(float time, float duration);

float *generate_audio_buffer(const char *input_file, int *sample_count, float *duration);
void write_wav_header(FILE *f, int data_size);
int save_audio_to_wav(const char *filename, float *buffer, int sample_count);

int launch_gui(int argc, char *argv[]);

#endif
