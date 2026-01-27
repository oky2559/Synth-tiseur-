// src/common.h
#ifndef COMMON_H
#define COMMON_H

#include <gtk/gtk.h>
#include <stdio.h>
#include <pthread.h>

#define SAMPLING_RATE 44100
#define PI 3.14159265358979323846
#define BUFFER_SIZE 2048

typedef struct {
    int type;
    float start;
    float duration;
    float freq;
} SoundNote;

typedef struct {
    GtkWidget *main_window;
    GtkWidget *file_entry;
    GtkWidget *status_label;
    GtkWidget *drawing_area;
    GtkWidget *play_button;
    GtkWidget *stop_button;
    float *buffer;
    int buffer_size;
    int actual_samples;
    float total_duration;
    int is_playing;
    int current_sample;
    pthread_t audio_thread;
    FILE *audio_device;
} AppWindow;

#endif