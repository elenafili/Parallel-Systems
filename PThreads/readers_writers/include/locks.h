#pragma once

#include <pthread.h>

typedef struct {
    int* readers_active;
    int* readers_waiting;
    int* writing;
    int* writers_waiting;

    pthread_mutex_t* mtx;
    pthread_cond_t* cond_read;
    pthread_cond_t* cond_write;
} Args;


void read_lock(Args args);

void read_unlock(Args args);

void write_lock(Args args);

void write_unlock(Args args);