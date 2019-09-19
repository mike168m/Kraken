#define KRAKEN_SCHEDULER    0x01
#define KRAKEN_MAX_THREADS  0x04
#define KRAKEN_DEBUG


#include "kraken.h"
#include <stdio.h>


__attribute__((regparm(1), noinline))
void t1 (struct kraken_runtime* runtime)
{
    //kraken_print_state(runtime, true);
    printf("Yielding to thread 2\n");
    kraken_yield(runtime); 
}

__attribute__((regparm(1), noinline))
void t2 (struct kraken_runtime* runtime)
{
    //kraken_print_state(runtime, true);
    printf("Yielding to runtime\n");
    kraken_yield(runtime); 
}


int main (void)
{
    struct kraken_runtime* runtime = kraken_initialize_runtime();

    if ( 0 > kraken_start_thread(runtime, t1))
    {
        printf("Couldn't start thread.\n");
    }

    if ( 0 > kraken_start_thread(runtime, t2))
    {
        printf("Couldn't start thread.\n");
    }

    kraken_run(runtime, 0);
}



