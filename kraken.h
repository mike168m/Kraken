// Copyright 2019 Michael Osei
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/// <link rel="stylesheet" href="./kraken_doc_style.css">
/// # Kraken ![](https://img.shields.io/travis/mike168m/Kraken?label=x86&style=flat-square)  ![](https://img.shields.io/travis/mike168m/Kraken?label=arm&style=flat-square)
/// Kraken is an opensource header only c library for writing multicore 
/// multithread programs using green threads on x86 (Linux, Windows & MacOSX), AVR amd ARM.
/// !!! WARNING: Implementation in progress
///     Use at your own risk.
/// ## Installation
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Bash
/// wget -O [include_dir]/kraken.h https://git.io/Je4U2
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// ## Example
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~C
/// #define  KRAKEN_SCHEDULER    0x01
/// #define  KRAKEN_MAX_THREADS  0x04
/// #include "kraken.h"
/// 
/// // 1.a. Define a function that represents a thread.
/// KRAKEN_THREAD_FUNCTION( first_thread,
/// {
///     static int var_1 = 0;
///     while ( TRUE ) 
///     {
///         // do some work
///         var_1 += 1;
///         printf( "In thread 1. var_1 = %d.\n", var_1 );
///         kraken_yield( runtime );
///     }
/// })
///
/// // 1.b. Define a second function for a second thread to make things more interesting.
/// KRAKEN_THREAD_FUNCTION( second_thread,
/// {
///     static int var_2 = 0;
///     while ( TRUE )
///     {
///       // do some other work.
///       var_2 += 2;
///       printf( "In thread 2. var_2 = %d.\n", var_2 );
///       kraken_yield( runtime );
///     } 
/// })
///
/// int main ( void )
/// {
///     // 2. Create a new single core kraken runtime.
///     // A runtime is responsible creating and managing threads on a cpu core.
///     struct kraken_runtime* my_runtime = kraken_initialize_runtime();
///     // 3. Use these predefined macros to schedule your threads to be run.
///     KRAKEN_SCHEDULE_THREAD( my_runtime, first_thread );
///     KRAKEN_SCHEDULE_THREAD( my_runtime, second_thread );
///     // 4. Next we start the runtime and specify a return value for main.
///     kraken_run( runtime, 0 );
/// }
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// ## Background
///
///
///
//==============================================================================
//
//                                  MACROS
//
//==============================================================================
#ifndef KRAKEN_H
#define KRAKEN_H


#ifndef KRAKEN_VERSION 
#define KRAKEN_VERSION                  "0.9.1"
#endif


// Scheduler implementation codes for preprocessor
#define KRAKEN_SCHEDULER_ROUND_ROBIN    0x01
#define KRAKEN_SCHEDULER_FIFO           0x02
#define KRAKEN_SCHEDULER_FAIR           0x04


// Architecture codes
#define KRAKEN_ARCH_AVR                 0x11
#define KRAKEN_ARCH_X86_64              0x12
#define KRAKEN_ARCH_X86                 0x13
#define KRAKEN_ARCH_ARM                 0x14


// Maximum number of threads
#if !defined(KRAKEN_MAX_THREADS)
#define KRAKEN_MAX_THREADS              0x04
#endif


// Maxium stack size
#if !defined( KRAKEN_STACK_SIZE )
#if KRAKEN_ARCH == KRAKEN_ARCH_X86
// use 2mb stacks for 64bit architectures
#define KRAKEN_STACK_SIZE               1024 * 1024 * 2
#else
// use 2mb stacks for 64bit architectures
#define KRAKEN_STACK_SIZE               1024 * 1024 * 2
#endif
// use 2mb stacks for 64bit architectures
#define KRAKEN_STACK_SIZE               1024 * 1024 * 2 
#endif


// architecture selection
#ifndef KRAKEN_ARCH
#if (defined(__x86_64) || defined(__x86_64__)) && (__x86_64 == 1 || __x86_64__ == 1)
#define KRAKEN_ARCH KRAKEN_ARCH_X86_64
#elif (defined(__i386) || defined(__i386__)) && (__i386 == 1 || __i386__ == 1)
#define KRAKEN_ARCH KRAKEN_ARCH_X86
#else
#define KRAKEN_ARCH                     0x0
#endif
#endif


#define KRAKEN_SCHEDULE_THREAD( runtime, function_name )\
{\
    int success = kraken_start_thread( runtime, function_name );\
    assert( -1 < success );\
}\


#define KRAKEN_THREAD_FUNCTION(name, code)\
/*__attribute__( ( regparm( 1 ), noinline ) )*/\
void name\
(\
struct kraken_runtime* runtime\
)\
{\
    __asm__\
    (\
    "movq   %rax, -8(%rbp)  \n\t"\
    );\
    code\
}\


// debug settings
#ifdef KRAKEN_DEBUG
#include <stdio.h>
#endif


#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <strings.h>


//========================================================
//
//               LOCAL STRUCTURES & ENUMS
//
//========================================================
struct kraken_context
{
#if KRAKEN_ARCH == KRAKEN_ARCH_X86_64
    uint64_t    rsp;
    uint64_t    r15;
    uint64_t    r14;
    uint64_t    r13;
    uint64_t    r12;
    uint64_t    rbx;
    uint64_t    rbp;
#elif KRAKEN_ARCH == KRAKEN_ARCH_X86
    uint32_t    esp;
    uint32_t    ebx;
    uint32_t    ebp;
#elif KRAKEN_ARCH == KRAKEN_ARCH_AVR
    uint8_t     r20;
    uint8_t     r24;
    uint8_t     sp;
#else
    #error      "Architecture not defined or implemented for Kraken library!"
#endif
};


enum kraken_status
{
    STOPPED,
    RUNNING,
    READY
};


struct kraken_thread
{
    struct kraken_context context;
    enum kraken_status    status;
    char*                 stack;
    uint16_t              id;
};


struct kraken_runtime
{
    struct kraken_thread threads[KRAKEN_MAX_THREADS];
    struct kraken_thread *current_thread;
};


typedef void (*function_type)( struct kraken_runtime* );


//==============================================================================
//
//                           FUNCTION PROTOTYPES
//
//==============================================================================
struct kraken_runtime* kraken_initialize_runtime( void );

void kraken_run (
    struct kraken_runtime*,
    int
);


int kraken_start_thread (
    struct kraken_runtime*,
    function_type
);


static void kraken_guard (
    struct kraken_runtime*
);


bool kraken_yield (
    struct kraken_runtime*
);


void kraken_print_state (
    struct kraken_runtime*,
    bool
);


static void kraken_switch (
    struct kraken_context*,
    struct kraken_context*,
    struct kraken_runtime*
);


//==============================================================================
//
//                           FUNCTION IMPLEMENTATIONS
//
//==============================================================================
static void kraken_print_thread_state
(
    struct kraken_thread*  current_thread
)
{
    printf( "Thread %d address: %p.\n\
            context addr %p\n\
            \trsp: %p\n\
            \tr15: %d\n\
            \tr14: %d\n\
            \tr13: %d\n\
            \tr12: %d\n\
            \trbx: %d\n\
            \trbp: %p\n\
            status addr %p\n\
            status addr %p\n\
            \tstatus %d\n\
            stack addr %p\n\n",
            current_thread->id,
            current_thread,
            &current_thread->context,
            current_thread->context.rsp,
            current_thread->context.r15,
            current_thread->context.r14,
            current_thread->context.r13,
            current_thread->context.r12,
            current_thread->context.rbx,
            current_thread->context.rbp,
            &current_thread->status,
            current_thread->status,
            current_thread->stack );
} // kraken_print_thread_state


void kraken_print_state
(
    struct kraken_runtime*  runtime,
    bool                    onlyCurrent
)
{
#ifdef KRAKEN_DEBUG
    assert( runtime->current_thread != NULL );

    kraken_print_thread_state( runtime->current_thread );

    if ( onlyCurrent != true )
    {
        for ( uint thread_idx = 0; thread_idx < KRAKEN_MAX_THREADS; thread_idx++ )
        {
            kraken_print_thread_state( &runtime->threads[ thread_idx ] );
        }
    }
#endif
} // kraken_print_state


void __attribute__( ( noreturn ) ) kraken_run
(
    struct kraken_runtime*  runtime,
    int                     return_code
)
{
    if ( runtime->current_thread != &runtime->threads[ 0 ] )
    {
        runtime->current_thread->status = STOPPED;
        kraken_yield( runtime );
    }

    while ( kraken_yield( runtime ) );

    // Free thread stack memory when done
    for ( uint16_t thread_idx = 0; thread_idx < KRAKEN_MAX_THREADS; thread_idx++ )
    {
        if ( NULL != runtime->threads[ thread_idx ].stack )
        {
            free( runtime->threads[ thread_idx ].stack );
        }
    }

    exit( return_code );
} // kraken_run


struct kraken_runtime* kraken_initialize_runtime
(
    void
)
{
    struct kraken_runtime* runtime = ( struct kraken_runtime* )
        malloc( sizeof( struct kraken_runtime ) );

    bzero( ( void* )runtime, sizeof( struct kraken_runtime ) );

    runtime->current_thread = &runtime->threads[ 0 ];
    runtime->current_thread->status = RUNNING;
    runtime->current_thread->stack = ( char* )malloc( sizeof( char ) * KRAKEN_STACK_SIZE );

    const char* const thread_stack = runtime->current_thread->stack;
    assert( ( thread_stack == NULL, "KRAKEN: Can't allocate memory for thread stack." ) );

    for ( uint16_t thread_idx = 0; thread_idx < KRAKEN_MAX_THREADS; thread_idx++ )
    {
        runtime->threads[ thread_idx ].id = thread_idx;
    }

    return runtime;
} // kraken_initialize_runtime


__asm__
(
    ".globl _kraken_switch, kraken_switch\n\t"
    "_kraken_switch:                     \n\t"
    "kraken_switch:                      \n\t"
#if KRAKEN_ARCH==KRAKEN_ARCH_X86_64
    // Swap contexts
    //"call check_current_thread_ptr      \n\t"
#ifdef KRAKEN_DEBUG // interrupt gdb if build type is debug.
    //"int    $3                           \n\t"
#endif
    "movq   %rsp, 0x00(%rdi)             \n\t"
    "movq   %r15, 0x08(%rdi)             \n\t"
    "movq   %r14, 0x10(%rdi)             \n\t"
    "movq   %r13, 0x18(%rdi)             \n\t"
    "movq   %r12, 0x20(%rdi)             \n\t"
    "movq   %rbx, 0x28(%rdi)             \n\t"
    "movq   %rbp, 0x30(%rdi)             \n\t"
    "movq   0x00(%rsi), %rsp             \n\t"
    "movq   0x08(%rsi), %r15             \n\t"
    "movq   0x10(%rsi), %r14             \n\t"
    "movq   0x18(%rsi), %r13             \n\t"
    "movq   0x20(%rsi), %r12             \n\t"
    "movq   0x28(%rsi), %rbx             \n\t"
    "movq   0x30(%rsi), %rbp             \n\t"
    "movq   %rdx,       %rax             \n\t"
    // jump to thread's function
    "ret                                 \n\t"
#elif KRAKEN_ARCH==KRAKEN_ARCH_X86
    "movl   %esp, 0x00(%edi)             \n\t"
    "movl   %ebx, 0x28(%edi)             \n\t"
    "movl   %ebp, 0x30(%edi)             \n\t"
    "movl   0x00(%esi), %esp             \n\t"
    "movl   0x28(%esi), %ebx             \n\t"
    "movl   0x30(%esi), %ebp             \n\t"
#elif KRAKEN_ARCH==KRAKEN_ARCH_AVR

#endif
);


static void kraken_guard
(
    struct kraken_runtime*  runtime
)
{
    // Mov runtime pointer into rax register just to ensure we have a backup copy.
    // Helpful for debugging.
    __asm__
    (
    "movq   %rdi, %rax                   \n\t"
    "movq   %rax, -0x8(%rbp)             \n\t"
    );
    
    assert( NULL != runtime );

    if ( runtime->current_thread != &runtime->threads[ 0 ] )
    {
        runtime->current_thread->status = STOPPED;
        kraken_yield( runtime );
    }

    while ( kraken_yield( runtime ) ) ;
} // kraken_guard


bool kraken_yield
(
    struct kraken_runtime*  runtime
)
{
    struct kraken_context *old_ctx             =       NULL;
    struct kraken_context *new_ctx             =       NULL;
    struct kraken_thread  *previous_thread     =       NULL;

#if KRAKEN_SCHEDULER==KRAKEN_SCHEDULER_ROUND_ROBIN
    previous_thread = runtime->current_thread;

    while ( previous_thread->status != READY )
    {
        // if the current thread is the last thread
        struct kraken_thread *next_thread = previous_thread++;

        struct kraken_thread *invalid_thread = &runtime->threads[ KRAKEN_MAX_THREADS ];

        if ( next_thread == invalid_thread )
        {
            // set the current thread to the first thread
            previous_thread = &( runtime->threads[ 0 ] );
        }

        if ( previous_thread == runtime->current_thread )
        {
            return false;
        }
    }

    if ( runtime->current_thread->status != STOPPED )
    {
        runtime->current_thread->status = READY;
    }
    
    previous_thread->status = RUNNING;
#endif

    old_ctx = &runtime->current_thread->context;
    new_ctx = &previous_thread->context;

    runtime->current_thread = previous_thread;

    // switch from old context to new context
    assert( runtime->current_thread != NULL );

    kraken_switch( old_ctx, new_ctx, runtime );

    return true;
} // kraken_yield


int kraken_start_thread
(
    struct kraken_runtime*  runtime,
    function_type           thread_func
)
{
    struct kraken_thread* new_thread;

    // look for a home for the new thread;
    for ( new_thread = &runtime->threads[ 0 ]; true ;new_thread++ )
    {
        if ( new_thread == &runtime->threads[ KRAKEN_MAX_THREADS ] )
        {
            return -1;
        }
        else if ( new_thread->status == STOPPED )
        {
            break;
        }
    }

    new_thread->stack = ( char* )malloc( KRAKEN_STACK_SIZE );

    const char* const thread_stack = new_thread->stack;
    assert( ( NULL == thread_stack, "KRAKEN: Can't allocate memory for thread stack." ) );

    if ( NULL == new_thread->stack )
    {
        return -1;
    }

#if KRAKEN_ARCH == KRAKEN_ARCH_X86_64
    *( uint64_t* )&( new_thread->stack[ KRAKEN_STACK_SIZE -  8 ] ) = ( uint64_t )kraken_guard;

    *( uint64_t* )&( new_thread->stack[ KRAKEN_STACK_SIZE - 16 ] ) = ( uint64_t )thread_func;

    new_thread->context.rsp = ( uint64_t )&( new_thread->stack[ KRAKEN_STACK_SIZE - 16 ] );

#elif KRAKEN_ARCH == KRAKEN_ARCH_X86
    *( uint32_t* )&( new_thread->stack[ KRAKEN_STACK_SIZE -  4 ] ) = ( uint32_t )kraken_guard;

    *( uint32_t* )&( new_thread->stack[ KRAKEN_STACK_SIZE -  8 ] ) = ( uint32_t )thread_func;

    new_thread->context.esp = ( uint32_t )&( new_thread->stack[ KRAKEN_STACK_SIZE - 8 ] );

#endif

    new_thread->status = READY;

    return 0;
} // kraken_start_thread

#endif // KRAKEN_H
