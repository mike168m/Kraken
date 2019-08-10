#define KRAKEN_SCHEDULER    0x01
#define KRAKEN_MAX_THREADS  0x04
#define KRAKEN_DEBUG

#include "kraken.h"
#include <stdio.h>

void thread_function (struct kraken_runtime* runtime) {
    static int x = 0;
    int id = ++x;
    for (int i = 0; i < 10; i++) {
        kraken_print_state(runtime, true);
        
        printf("Before yield\n");
        printf("Hi from thread %d\n", id);

        kraken_yield(runtime);

        printf("After yield\n");
    }
}

int main (void) {
    struct kraken_runtime* runtime = kraken_initialize_runtime();

    if (kraken_start_thread(runtime, thread_function) < 0)
        printf("Couldn't start thread.\n");
    if (kraken_start_thread(runtime, thread_function) < 0)
        printf("Couldn't start thread.\n");

    kraken_run(runtime, 0);
}



