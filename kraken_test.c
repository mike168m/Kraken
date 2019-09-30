#define KRAKEN_SCHEDULER    0x01
#define KRAKEN_MAX_THREADS  0x04
#define KRAKEN_DEBUG


#include "kraken.h"
#include <stdio.h>


KRAKEN_THREAD_FUNCTION( t1,
{
    static int i = 0;
    printf("Yielding to thread 2\n");
    for (; i < 10; i++) 
    {
        printf("Yielding to thread 2\n");
        kraken_yield(runtime); 
    }
})



KRAKEN_THREAD_FUNCTION( t2,
{
    //kraken_print_state(runtime, true);
    static int i = 0; 
    printf("Yielding to thread 1 or runtime.\n");
    for (; i < 10; i++) 
    {
  printf("Yielding to thread 1 or runtime.\n");
   
        kraken_yield(runtime); 
    }
})


int main (void)
{
    struct kraken_runtime* runtime =
        kraken_initialize_runtime();

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



