#include "custom_math.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

double taylor_sin(double x)
{
    while (x > PI)
        x -= 2.0 * PI;
    while (x < -PI)
        x += 2.0 * PI;

    double term = x;
    double result = x;
    double x2 = x * x;

    for (int n = 1; n <= 10; n++) {
        term *= -x2 / ((2 * n) * (2 * n + 1));
        result += term;
    }

    return result;
}

float generate_sample(int type, float phase) {
    switch (type) {
        case 0: return sinf(2.0f * PI * phase);
        case 1: return (sinf(2.0f * PI * phase) >= 0) ? 1.0f : -1.0f;
        case 2: return 2.0f * fabsf(2.0f * (phase - floorf(phase + 0.5f))) - 1.0f;
        case 3: return 2.0f * (phase - floorf(phase + 0.5f));
        default: return 0.0f;
    }
}

float apply_automatic_envelope(float time, float duration) {
    float min_attack = 0.01f, min_decay = 0.01f, min_release = 0.02f;
    float attack = fmaxf(min_attack, fminf(0.1f, duration * 0.15f));
    float decay = fmaxf(min_decay, fminf(0.1f, duration * 0.15f));
    float release = fmaxf(min_release, fminf(0.2f, duration * 0.2f));
    float sustain = 0.9f;
    float sustain_time = duration - (attack + decay + release);
    if (sustain_time < 0) sustain_time = 0;

    if (time < 0) return 0.0f;
    if (time < attack) return time / attack;
    if (time < attack + decay) return 1.0f - (1.0f - sustain) * ((time - attack) / decay);
    if (time < attack + decay + sustain_time) return sustain;
    if (time < duration) return sustain * (1.0f - (time - (duration - release)) / release);
    return 0.0f;
}

float envelope(float t, float duration) {
    float attack = 0.05f * duration;
    float decay = 0.1f * duration;
    float release = 0.15f * duration;
    float sustain = 0.8f;
    if (t < 0) return 0.0f;
    if (t < attack) return t / attack;
    if (t < attack + decay) return 1.0f - (1.0f - sustain) * ((t - attack) / decay);
    if (t < duration - release) return sustain;
    if (t < duration) return sustain * (1.0f - (t - (duration - release)) / release);
    return 0.0f;
}

void write_wav_header(FILE *f, int data_size) {
    int total_size = 36 + data_size;
    short format = 1;
    short channels = 1;
    int sample_rate = SAMPLING_RATE;
    int byte_rate = SAMPLING_RATE * sizeof(int16_t);
    short block_align = sizeof(int16_t);
    short bits_per_sample = 16;

    fwrite("RIFF", 1, 4, f);
    fwrite(&total_size, 4, 1, f);
    fwrite("WAVEfmt ", 1, 8, f);
    int fmt_chunk_size = 16;
    fwrite(&fmt_chunk_size, 4, 1, f);
    fwrite(&format, 2, 1, f);
    fwrite(&channels, 2, 1, f);
    fwrite(&sample_rate, 4, 1, f);
    fwrite(&byte_rate, 4, 1, f);
    fwrite(&block_align, 2, 1, f);
    fwrite(&bits_per_sample, 2, 1, f);
    fwrite("data", 1, 4, f);
    fwrite(&data_size, 4, 1, f);
}

float *generate_audio_buffer(const char *input_file, int *sample_count, float *duration) {
    FILE *inputFile = fopen(input_file, "r");
    if (!inputFile) return NULL;

    int max_samples = SAMPLING_RATE * 10;
    float *buffer = calloc(max_samples, sizeof(float));
    if (!buffer) {
        fclose(inputFile);
        return NULL;
    }

    float total_duration = 0;
    SoundNote note;

    while (fscanf(inputFile, "%d %f %f %f", &note.type, &note.start, &note.duration, &note.freq) == 4) {
        int start_sample = (int)(note.start * SAMPLING_RATE);
        int num_samples = (int)(note.duration * SAMPLING_RATE);

        for (int i = 0; i < num_samples; i++) {
            if (start_sample + i < max_samples) {
                float phase = (note.freq * i) / SAMPLING_RATE;
                float t = (float)i / SAMPLING_RATE;
                float env = envelope(t, note.duration);
                buffer[start_sample + i] += generate_sample(note.type, phase) * env;
            }
        }

        if (note.start + note.duration > total_duration)
            total_duration = note.start + note.duration;
    }

    fclose(inputFile);
    *sample_count = (int)(total_duration * SAMPLING_RATE);
    *duration = total_duration;
    return buffer;
}
