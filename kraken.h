#ifndef KRAKEN_H
#define KRAKEN_H

#define KRAKEN_VERSION                  "0.9.1"

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
#if KRAKEN_ARCH==KRAKEN_ARCH_X86
// use 2mb stacks for 64bit architectures
#define KRAKEN_STACK_SIZE               1024 * 1024 * 2
#else
// use 2mb stacks for 64bit architectures
#define KRAKEN_STACK_SIZE               1024 * 1024 * 2
#endif
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

// debug settings
#ifdef KRAKEN_DEBUG
#include <stdio.h>
#endif


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <strings.h>


struct kraken_context {
#if KRAKEN_ARCH==KRAKEN_ARCH_X86_64
    uint64_t rsp;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rbx;
    uint64_t rbp;
#elif KRAKEN_ARCH==KRAKEN_ARCH_X86
    uint32_t esp;
    uint32_t ebx;
    uint32_t ebp;
#elif KRAKEN_ARCH==KRAKEN_ARCH_AVR
    uint8_t r20;
    uint8_t r24;
    uint8_t sp;
#else
    #error "Architecture not defined or implemented for Kraken library!"
#endif
};


enum kraken_status {
    STOPPED,
    RUNNING,
    READY
};


struct kraken_thread {
    struct kraken_context context;
    enum kraken_status status;
    char* stack;
    uint16_t id;
};


struct kraken_runtime {
    struct kraken_thread threads[KRAKEN_MAX_THREADS];
    struct kraken_thread *current_thread;
};


typedef void (*function_type)(struct kraken_runtime*);


struct kraken_runtime* kraken_initialize_runtime();


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


static void kraken_print_thread_state
(
    struct kraken_thread*      current_thread
)
{
    printf("Thread %d address: %p.\n\
            context addr %p\n\
            \trsp: %p\n\
            \tr15: %d\n\
            \tr14: %d\n\
            \tr13: %d\n\
            \tr12: %d\n\
            \trbx: %d\n\
            \trbp: %p\n\
            status addr %p\n\
            \tstatus %d\n\
            stack addr %p\n\
            \n",
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
            current_thread->stack
    );
}


void kraken_print_state
(
    struct kraken_runtime*      runtime,
    bool                        onlyCurrent
)
{
#ifdef KRAKEN_DEBUG
    assert(runtime->current_thread != NULL);

    kraken_print_thread_state(runtime->current_thread);

    if (onlyCurrent != true) {
        for (uint thread_idx = 0; thread_idx < KRAKEN_MAX_THREADS; thread_idx++) {
            kraken_print_thread_state(&runtime->threads[thread_idx]);
        }
    }
#endif
}


void __attribute__((noreturn)) kraken_run
(
    struct kraken_runtime*      runtime,
    int                         return_code
)
{
    if (runtime->current_thread != &runtime->threads[0]) {
        runtime->current_thread->status = STOPPED;
        kraken_yield(runtime);
    }

    while (kraken_yield(runtime)) ;

    // Free thread stack memory when done
    for (uint16_t thread_idx = 0; thread_idx < KRAKEN_MAX_THREADS; thread_idx++) {
        if (NULL != runtime->threads[thread_idx].stack) {
            free(runtime->threads[thread_idx].stack);
        }
    }

    exit(return_code);
}


struct kraken_runtime* kraken_initialize_runtime() 
{
    struct kraken_runtime* runtime = (struct kraken_runtime*) malloc(sizeof(struct kraken_runtime));

    bzero((void*)runtime, sizeof(struct kraken_runtime));

    runtime->current_thread = &runtime->threads[0];
    runtime->current_thread->status = RUNNING;
    runtime->current_thread->stack = (char*)malloc(KRAKEN_STACK_SIZE);

    for (uint16_t thread_idx = 0; thread_idx < KRAKEN_MAX_THREADS; thread_idx++) {
        runtime->threads[thread_idx].id = thread_idx;
    }

    return runtime;
}


void check_current_thread_ptr (struct kraken_runtime* runtime) {
    //kraken_print_state(runtime, true);
    assert(runtime->current_thread != NULL);
}


__asm__ (
    ".globl _kraken_switch, kraken_switch\n\t"
    "_kraken_switch:         \n\t"
    "kraken_switch:         \n\t"
#if KRAKEN_ARCH==KRAKEN_ARCH_X86_64
    // Swap contexts
    "movq   %rsp, 0x00(%rdi)\n\t"
    "movq   %r15, 0x08(%rdi)\n\t"
    "movq   %r14, 0x10(%rdi)\n\t"
    "movq   %r13, 0x18(%rdi)\n\t"
    "movq   %r12, 0x20(%rdi)\n\t"
    "movq   %rbx, 0x28(%rdi)\n\t"
    "movq   %rbp, 0x30(%rdi)\n\t"
    "movq   0x00(%rsi), %rsp\n\t"
    "movq   0x08(%rsi), %r15\n\t"
    "movq   0x10(%rsi), %r14\n\t"
    "movq   0x18(%rsi), %r13\n\t"
    "movq   0x20(%rsi), %r12\n\t"
    "movq   0x28(%rsi), %rbx\n\t"
    "movq   0x30(%rsi), %rbp\n\t"
    //"call check_current_thread_ptr\n\t"
    
    // load runtime pointer 
    //into thread function parameter

    // jump to thread function
    "ret                      \n\t"
#elif KRAKEN_ARCH==KRAKEN_ARCH_X86
    "movl   %esp, 0x00(%edi)\n\t"
    "movl   %ebx, 0x28(%edi)\n\t"
    "movl   %ebp, 0x30(%edi)\n\t"
    "movl   0x00(%esi), %esp\n\t"
    "movl   0x28(%esi), %ebx\n\t"
    "movl   0x30(%esi), %ebp\n\t"
#elif KRAKEN_ARCH==KRAKEN_ARCH_AVR

#endif
);


static void kraken_guard
(
    struct kraken_runtime*      runtime
)
{
    assert(runtime->current_thread != NULL);

    if (runtime->current_thread != &runtime->threads[0])
    {
        runtime->current_thread->status = STOPPED;
        kraken_yield(runtime);
    }
} // kraken_guard


bool kraken_yield
(
    struct kraken_runtime*      runtime
)
{
    struct kraken_context *old_ctx =            NULL;
    struct kraken_context *new_ctx =            NULL;
    struct kraken_thread  *other_thread =       NULL;

#if KRAKEN_SCHEDULER==KRAKEN_SCHEDULER_ROUND_ROBIN
    other_thread = runtime->current_thread;

    while (other_thread->status != READY)
    {
        // if the current thread is the last thread
        struct kraken_thread *next = other_thread++;

        struct kraken_thread *after_last = &runtime->threads[KRAKEN_MAX_THREADS];

        if (next == after_last) {
            // set the current thread to the first thread
            other_thread = &(runtime->threads[0]);
        }

        if (other_thread == runtime->current_thread) {
            return false;
        }
    }

    if (runtime->current_thread->status != STOPPED) {
        runtime->current_thread->status = READY;
    }

    other_thread->status = RUNNING;
#endif

    old_ctx = &runtime->current_thread->context;
    new_ctx = &other_thread->context;

    runtime->current_thread = other_thread;

    //printf("Called kraken_yield\n");

    //kraken_print_state(runtime, false);

    // switch from old context to new context
    assert(runtime->current_thread != NULL);

    kraken_switch(old_ctx, new_ctx, runtime);

    return true;
} // kraken_yield


int kraken_start_thread 
(
    struct kraken_runtime*      runtime,
    function_type               thread_func
)
{
    struct kraken_thread* new_thread;

    // look for a home for the new thread;
    for (new_thread = &runtime->threads[0]; true ;new_thread++) {
        if (new_thread == &runtime->threads[KRAKEN_MAX_THREADS]) {
            return -1;
        } else if (new_thread->status == STOPPED) {
            break;
        }
    }

    new_thread->stack = (char*)malloc(KRAKEN_STACK_SIZE);

    if (new_thread->stack == NULL) {
        return -1;
    }

#if KRAKEN_ARCH==KRAKEN_ARCH_X86_64
    *(uint64_t *)&(new_thread->stack[KRAKEN_STACK_SIZE -  8]) = (uint64_t)kraken_guard;

    *(uint64_t *)&(new_thread->stack[KRAKEN_STACK_SIZE - 16]) = (uint64_t)thread_func;

    new_thread->context.rsp = (uint64_t)&(new_thread->stack[KRAKEN_STACK_SIZE - 16]);

#elif KRAKEN_ARCH==KRAKEN_ARCH_X86
    *(uint32_t *)&(new_thread->stack[KRAKEN_STACK_SIZE -  4]) = (uint32_t)kraken_guard;

    *(uint32_t *)&(new_thread->stack[KRAKEN_STACK_SIZE -  8]) = (uint32_t)thread_func;

    new_thread->context.esp  = (uint32_t)&(new_thread->stack[KRAKEN_STACK_SIZE - 8]);
#endif

    new_thread->status = READY;

    return 0;
}

#endif // KRAKEN_H
