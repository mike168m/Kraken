#define KRAKEN_SCHEDULER    0x01
#define KRAKEN_MAX_THREADS  0x04
#define KRAKEN_DEBUG


#include "kraken.h"
#include <stdio.h>


KRAKEN_AVR_THREAD_FUNCTION( t1,
{
    static int i = 0;
    for ( ; i < 10; i++ ) 
    {
        printf( "In thread 1.\n" );
        kraken_yield( runtime ); 
    }
})


KRAKEN_AVR_THREAD_FUNCTION( t2,
{
    static int i = 0; 
    for (; i < 10; i++ ) 
    {
        printf( "In thread 2.\n" );
        kraken_yield( runtime ); 
    }
})


int main
(
    void
)
{
    struct kraken_runtime* runtime =
        kraken_initialize_runtime();

    KRAKEN_SCHEDULE_THREAD( runtime, t1 );
    KRAKEN_SCHEDULE_THREAD( runtime, t2 );

    kraken_run( runtime, 0 );
}



