#define KRAKEN_SCHEDULER    0x01
#define KRAKEN_MAX_THREADS  0x04
#define KRAKEN_DEBUG

#include "kraken.h"
#include <stdio.h>

void thread_1 () {
    static int x = 0;
    int id = ++x;
    for (int i = 0; i < 10; i++) {
        printf("Hi from thread %d\n", id);
        kraken_yield();
    }
}

int main (void) {
    kraken_initialize_runtime();

    for (int i = 0; i < KRAKEN_MAX_THREADS; i++) {
        if (kraken_start_thread(thread_1) < 0) 
            printf("Couldn't start thread %d\n", i);
    }

    kraken_run(0);
}
