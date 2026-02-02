#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h> // pour usleep
#include "../include/custom_math.h"

#define PLAYBACK_BUFFER 2048

typedef struct {
    GtkWidget *main_window;
    GtkWidget *file_entry;
    GtkWidget *status_label;
    GtkWidget *drawing_area;
    GtkWidget *play_button;
    GtkWidget *stop_button;
    GtkWidget *volume_slider;
    float *buffer;
    int actual_samples;
    float total_duration;
    int is_playing;
    int current_sample;
    float volume_level;
    pthread_t audio_thread;
    gint64 start_time;
    gboolean is_paused;
    double accumulated_time;
} AppWindow;

void update_status(AppWindow *app, const char *message) {
    gtk_label_set_text(GTK_LABEL(app->status_label), message);
}


// Thread simple pour jouer le son via la commande 'play' (soX)
void* audio_playback_thread(void *arg) {
    AppWindow *app = (AppWindow *)arg;
    
    FILE *audio_dev = popen("play -t raw -r 44100 -b 16 -c 1 -e signed-integer -q -", "w");
    if (!audio_dev) {
        // Fallback pour Linux (aplay) ou macOS (sox/play requis)
        audio_dev = popen("aplay -f cd -", "w"); 
    }

    if (!audio_dev) {
        // Dernier recours, on essaye juste de ne pas crasher, mais pas de son
        app->is_playing = 0;
        return NULL;
    }
    
    while (app->is_playing && app->current_sample < app->actual_samples) {
        int samples_to_write = (app->actual_samples - app->current_sample);
        if (samples_to_write > PLAYBACK_BUFFER) samples_to_write = PLAYBACK_BUFFER;
        
        int16_t pcm_buffer[PLAYBACK_BUFFER];
        for (int i = 0; i < samples_to_write; i++) {
            float sample = app->buffer[app->current_sample + i];
            sample *= app->volume_level;
            if (sample > 1.0f) sample = 1.0f;
            else if (sample < -1.0f) sample = -1.0f;
            pcm_buffer[i] = (int16_t)(sample * 32767);
        }
        
        if (fwrite(pcm_buffer, sizeof(int16_t), samples_to_write, audio_dev) < (size_t)samples_to_write) {
            break;
        }
        fflush(audio_dev);
        app->current_sample += samples_to_write;
    }
    
    pclose(audio_dev);
    app->is_playing = 0;
    return NULL;
}

gboolean on_drawing_area_draw(GtkWidget *widget, cairo_t *cr, AppWindow *app) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;
    
    // Fond blanc
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    
    if (!app->buffer || app->actual_samples == 0) {
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 14);
        cairo_move_to(cr, width / 2 - 100, height / 2);
        cairo_show_text(cr, "Aucune onde chargée");
        return FALSE;
    }
    
    // Axes
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, 10, height/2);
    cairo_line_to(cr, width - 10, height/2); // Axe X (temps)
    cairo_stroke(cr);
    
    // Dessin de l'onde (bleu)
    cairo_set_source_rgb(cr, 0.2, 0.4, 0.8);
    cairo_set_line_width(cr, 1);
    
    int skip = app->actual_samples / width; 
    if (skip < 1) skip = 1;
    
    cairo_move_to(cr, 10, height/2);
    for (int i = 0; i < width - 20; i++) {
        int sample_idx = i * skip;
        if (sample_idx >= app->actual_samples) break;
        
        float val = app->buffer[sample_idx];
        int y = (height / 2) - (int)(val * (height / 2 - 10));
        cairo_line_to(cr, 10 + i, y);
    }
    cairo_stroke(cr);
    
    // Curseur de lecture (rouge)
    if (app->is_playing) {
        gint64 now = g_get_monotonic_time();
        double elapsed = (now - app->start_time) / 1000000.0;
        int estimated_sample = (int)(elapsed * 44100);
        if (estimated_sample > app->actual_samples) {
            estimated_sample = app->actual_samples;
        }
        float progress = (float)estimated_sample / app->actual_samples;
        int cx = 10 + (int)(progress * (width - 20));
        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_move_to(cr, cx, 0);
        cairo_line_to(cr, cx, height);
        cairo_stroke(cr);
        if (estimated_sample >= app->actual_samples) {
            app->is_playing = 0;
        }
        if (estimated_sample >= app->actual_samples) {
        app->is_playing = 0;      
        update_status(app, "Prêt");   
        gtk_widget_queue_draw(widget);
    }
    }
    
    
    return FALSE;
}

void on_generate_clicked(GtkWidget *widget, AppWindow *app) {
    (void)widget;
    const char *input_path = gtk_entry_get_text(GTK_ENTRY(app->file_entry));
    
    if (!input_path || strlen(input_path) == 0) {
        update_status(app, "Erreur: Entrez un nom de fichier.");
        return;
    }

    update_status(app, "Génération en cours...");
    
    // Nettoyage précédent
    if (app->buffer) {
        free(app->buffer);
        app->buffer = NULL;
    }

    // Appel à la bibliothèque pour générer le son
    app->buffer = generate_audio_buffer(input_path, &app->actual_samples, &app->total_duration);

    if (!app->buffer) {
        update_status(app, "Erreur: Fichier introuvable ou invalide.");
        return;
    }

    // Sauvegarde automatique
    const char *download_dir = g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);
    char *output_path = g_build_filename(download_dir, "output.wav", NULL);
    if (save_audio_to_wav(output_path, app->buffer, app->actual_samples)) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Succès: %.2fs générés dans %s", app->total_duration, output_path);
        update_status(app, msg);
    } else {
        update_status(app, "Erreur sauvegarde WAV.");
    }
    
    g_free(output_path);
    gtk_widget_queue_draw(app->drawing_area);
}

void on_play_clicked(GtkWidget *widget, AppWindow *app) {
    (void)widget;
    if (!app->buffer) return;
    if (app->is_playing) return;
    
    app->is_playing = 1;
    app->current_sample = 0;
    app->start_time = g_get_monotonic_time();
    update_status(app, "Lecture...");
    pthread_create(&app->audio_thread, NULL, audio_playback_thread, app);
}

void on_pause_clicked(GtkWidget *widget, AppWindow *app) {
    if (!app->is_playing) return;

    if (app->is_paused) {

        app->is_paused = FALSE;
        app->start_time = g_get_monotonic_time();
        gtk_button_set_label(GTK_BUTTON(widget), "Pause");
        update_status(app, "Lecture en cours...");
    } else {
        app->is_paused = TRUE;
        gint64 now = g_get_monotonic_time();
        app->accumulated_time += (now - app->start_time) / 1000000.0;
        
        gtk_button_set_label(GTK_BUTTON(widget), "Reprendre");
        update_status(app, "En Pause");
    }
}

void on_stop_clicked(GtkWidget *widget, AppWindow *app) {
    (void)widget;
    app->is_playing = 0;
    update_status(app, "Arrêté.");
    gtk_widget_queue_draw(app->drawing_area);
}

void on_volume_changed(GtkRange *range, AppWindow *app) {
    app->volume_level = gtk_range_get_value(range);
}

void on_file_chooser_clicked(GtkWidget *widget, AppWindow *app) {
    (void)widget;
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Ouvrir", GTK_WINDOW(app->main_window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Annuler", GTK_RESPONSE_CANCEL,
                                                    "_Ouvrir", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_entry_set_text(GTK_ENTRY(app->file_entry), filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

gboolean update_display(gpointer data) {
    AppWindow *app = (AppWindow *)data;
    if (app->is_playing) {
        gtk_widget_queue_draw(app->drawing_area);
    }
    return TRUE;
}

int launch_gui(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    AppWindow app = {0};
    app.volume_level = 1.0f;

    app.main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app.main_window), "Synthétiseur C");
    gtk_window_set_default_size(GTK_WINDOW(app.main_window), 800, 500);
    g_signal_connect(app.main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(app.main_window), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    // Zone fichier
    GtkWidget *hbox_file = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    app.file_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app.file_entry), "Fichier d'entrée (ex: game.txt)");
    GtkWidget *btn_browse = gtk_button_new_with_label("...");
    g_signal_connect(btn_browse, "clicked", G_CALLBACK(on_file_chooser_clicked), &app);
    gtk_box_pack_start(GTK_BOX(hbox_file), app.file_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_file), btn_browse, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_file, FALSE, FALSE, 0);

    // Boutons
    GtkWidget *hbox_ctrl = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn_gen = gtk_button_new_with_label("Générer");
    app.play_button = gtk_button_new_with_label("Lire");
    app.stop_button = gtk_button_new_with_label("Stop");
    g_signal_connect(btn_gen, "clicked", G_CALLBACK(on_generate_clicked), &app);
    g_signal_connect(app.play_button, "clicked", G_CALLBACK(on_play_clicked), &app);
    g_signal_connect(app.stop_button, "clicked", G_CALLBACK(on_stop_clicked), &app);
    gtk_box_pack_start(GTK_BOX(hbox_ctrl), btn_gen, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_ctrl), app.play_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_ctrl), app.stop_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_ctrl, FALSE, FALSE, 0);

    // Volume
    app.volume_slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 1, 0.01);
    gtk_range_set_value(GTK_RANGE(app.volume_slider), 1.0);
    g_signal_connect(app.volume_slider, "value-changed", G_CALLBACK(on_volume_changed), &app);
    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Volume"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), app.volume_slider, FALSE, FALSE, 0);

    // Dessin
    app.drawing_area = gtk_drawing_area_new();
    g_signal_connect(app.drawing_area, "draw", G_CALLBACK(on_drawing_area_draw), &app);
    gtk_box_pack_start(GTK_BOX(vbox), app.drawing_area, TRUE, TRUE, 0);

    // Status
    app.status_label = gtk_label_new("Prêt");
    gtk_box_pack_start(GTK_BOX(vbox), app.status_label, FALSE, FALSE, 0);

    g_timeout_add(100, update_display, &app);

    gtk_widget_show_all(app.main_window);
    gtk_main();

    if (app.buffer) free(app.buffer);
    return 0;
}
