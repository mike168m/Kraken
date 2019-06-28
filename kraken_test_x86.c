#define KRAKEN_SCHEDULER 0x01
#define KRAKEN_ARCH 0x12

#include "kraken.h"
#include <stdio.h>

void thread() {
    static int x;
    int i, id;

    id = ++x;
    for (i = 0; i < 10; i++) {
        printf("%d %d\n", id, i);
        kraken_yield();
    }
}


int main(void) {
    kraken_initialize_runtime();

    if (kraken_start_thread(thread) < 0) printf("Couldn't start thread 1\n");
    if (kraken_start_thread(thread) < 0) printf("Couldn't start thread 2\n");

    kraken_run(0);
}