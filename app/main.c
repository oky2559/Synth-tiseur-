#include <stdio.h>
#include <stdlib.h>
#include "../include/custom_math.h"

int main(void) {
    int sample_count = 0;
    float duration = 0;

    printf("Génération du signal audio...\n");

    // On appelle la fonction de calcul qui est dans src/custom_math.c
    // Cette même fonction est utilisée par le GUI !
    float *buffer = generate_audio_buffer("input.txt", &sample_count, &duration);

    if (!buffer) {
        fprintf(stderr, "Erreur lors de la génération du buffer.\n");
        return 1;
    }

    // On exporte en WAV en utilisant la fonction utilitaire de src/custom_math.c
    FILE *f = fopen("output.wav", "wb");
    if (f) {
        int data_size = sample_count * sizeof(int16_t);
        write_wav_header(f, data_size);
        
        for (int i = 0; i < sample_count; i++) {
            float s = buffer[i];
            if (s > 1.0f) s = 1.0f; else if (s < -1.0f) s = -1.0f;
            int16_t pcm = (int16_t)(s * 32767);
            fwrite(&pcm, sizeof(int16_t), 1, f);
        }
        fclose(f);
        printf("Succès : output.wav généré (%.2fs).\n", duration);
    }

    free(buffer);
    return 0;
}