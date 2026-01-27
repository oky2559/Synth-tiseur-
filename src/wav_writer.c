#include "wav_writer.h"
#include "common.h"
#include <stdint.h>

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