#include "locks.h"

void read_lock(Args args) {

    pthread_mutex_lock(args.mtx);

    while (*args.writing + *args.writers_waiting > 0) {
        (*args.readers_waiting)++;
        pthread_cond_wait(args.cond_read, args.mtx);
        (*args.readers_waiting)--;
    }

    (*args.readers_active)++;

    pthread_mutex_unlock(args.mtx);

}

void read_unlock(Args args) {

    pthread_mutex_lock(args.mtx);
    
    (*args.readers_active)--;

    if (*args.readers_active == 0)
        pthread_cond_signal(args.cond_write);

    pthread_mutex_unlock(args.mtx);
}

void write_lock(Args args) {

    pthread_mutex_lock(args.mtx);

    while (*args.readers_active > 0 || *args.writing == 1) {
        (*args.writers_waiting)++;
        pthread_cond_wait(args.cond_write, args.mtx);
        (*args.writers_waiting)--;
    }

    *args.writing = 1;
}

void write_unlock(Args args) {
   
    *args.writing = 0;

    if (*args.writers_waiting > 0)
        pthread_cond_signal(args.cond_write);
    else if (*args.readers_waiting > 0)
        pthread_cond_broadcast(args.cond_read);

    pthread_mutex_unlock(args.mtx);
}