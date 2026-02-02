#include "custom_math.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

float generate_sample(int type, float phase)
{
    switch (type)
    {
    case 0:
        return sinf(2.0f * PI * phase); // onde sinusoïdale
    case 1:
        return (sinf(2.0f * PI * phase) >= 0) ? 1.0f : -1.0f; // onde carrée
    case 2:
        return 2.0f * (phase - floorf(phase + 0.5f)); // onde en dents de scie
    case 3:
        return 2.0f * fabsf(2.0f * (phase - floorf(phase + 0.5f))) - 1.0f; // onde triangulaire
    default:
        return 0.0f;
    }
}

float apply_automatic_envelope(float time, float duration)
{
    float attack = fminf(0.1f, duration * 0.15f);
    float decay = fminf(0.1f, duration * 0.15f);
    float release = fminf(0.2f, duration * 0.2f);

    // Securites
    if (attack < 0.001f)
        attack = 0.001f;
    if (decay < 0.001f)
        decay = 0.001f;
    if (release < 0.001f)
        release = 0.001f;

    // Pour les notes trop courte
    float total_adr = attack + decay + release;
    if (total_adr > duration)
    {
        float scale = duration / total_adr;
        attack *= scale;
        decay *= scale;
        release *= scale;
        total_adr = duration;
    }

    float sustain_level = 0.8f;
    float sustain_time = duration - total_adr;

    if (time < attack)
        return time / attack;

    if (time < attack + decay)
        return 1.0f - (1.0f - sustain_level) * ((time - attack) / decay);

    if (time < attack + decay + sustain_time)
        return sustain_level;

    if (time < duration)
    {
        float release_start = attack + decay + sustain_time;
        return sustain_level * (1.0f - (time - release_start) / release);
    }

    return 0.0f;
}

void write_wav_header(FILE *f, int data_size)
{
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

float *generate_audio_buffer(const char *input_file, int *sample_count, float *duration)
{
    FILE *inputFile = fopen(input_file, "r");

    if (!inputFile)
        return NULL;

    float max_end_time = 0.0f;
    SoundNote temp_note;

    while (fscanf(inputFile, "%d %f %f %f", &temp_note.type, &temp_note.start, &temp_note.duration, &temp_note.freq) == 4)
    {
        float end_time = temp_note.start + temp_note.duration;
        if (end_time > max_end_time)
        {
            max_end_time = end_time;
        }
    }

    if (max_end_time <= 0.0f)
    {
        fclose(inputFile);
        return NULL;
    }

    int allocated_samples = (int)(max_end_time * SAMPLING_RATE) + 100;
    float *buffer = calloc(allocated_samples, sizeof(float));

    if (!buffer)
    {
        fclose(inputFile);
        return NULL;
    }

    rewind(inputFile);

    float total_duration = 0;
    SoundNote note;

    while (fscanf(inputFile, "%d %f %f %f", &note.type, &note.start, &note.duration, &note.freq) == 4)
    {
        int start_sample = (int)(note.start * SAMPLING_RATE);
        int num_samples = (int)(note.duration * SAMPLING_RATE);

        for (int i = 0; i < num_samples; i++)
        {
            // Vérification pour ne pas écrire hors du buffer alloué
            if (start_sample + i < allocated_samples)
            {
                float phase = (note.freq * i) / SAMPLING_RATE;
                float t = (float)i / SAMPLING_RATE;
                float env = apply_automatic_envelope(t, note.duration);
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

int save_audio_to_wav(const char *filename, float *buffer, int sample_count)
{
    FILE *f = fopen(filename, "wb");
    if (!f)
        return 0;

    int data_size = sample_count * sizeof(int16_t);

    write_wav_header(f, data_size);

    int16_t *pcm_buffer = malloc(data_size);
    if (!pcm_buffer)
    {
        fclose(f);
        return 0;
    }

    for (int i = 0; i < sample_count; i++)
    {
        float s = buffer[i];
        if (s > 1.0f)
            s = 1.0f;
        else if (s < -1.0f)
            s = -1.0f;

        // Conversion float -> int16
        pcm_buffer[i] = (int16_t)(s * 32767);
    }

    fwrite(pcm_buffer, sizeof(int16_t), sample_count, f);

    free(pcm_buffer);
    fclose(f);
    return 1;
}
