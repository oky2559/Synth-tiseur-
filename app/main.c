#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/custom_math.h"
#include <glib.h>

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "--cli") == 0)
    {
        const char *input_file = (argc > 2) ? argv[2] : "input.txt";
        int sample_count = 0;
        float duration = 0;

        printf("Lecture de %s...\n", input_file);

        // Génération à partir de la bibliothèque
        float *buffer = generate_audio_buffer(input_file, &sample_count, &duration);

        if (!buffer)
        {
            fprintf(stderr, "Erreur : Impossible de générer l'audio (fichier manquant ?)\n");
            return 1;
        }

        printf("Génération réussie : %.2f secondes.\n", duration);

        // Sauvegarde à partir de la bibliothèque
        const char *download_dir = g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);
        char *output_path = g_build_filename(download_dir, "output.wav", NULL);
        if (!download_dir) download_dir = ".";

        if (save_audio_to_wav(output_path, buffer, sample_count))
        {
            printf("Fichier output.wav créé avec succès.\n");
        }
        else
        {
            fprintf(stderr, "Erreur lors de l'écriture de output.wav\n");
        }
        g_free(output_path);
        free(buffer);
        return 0;
    }

    return launch_gui(argc, argv);
}