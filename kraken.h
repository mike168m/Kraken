#ifndef KRAKEN_H
#define KRAKEN_H

// Scheduler implementation codes for preprocessor
#define KRAKEN_SCHEDULER_SIMPLE_ARRAY   0x01
#define KRAKEN_SCHEDULER_FIFO           0x02
#define KRAKEN_SCHEDULER_ROUND_ROBIN    KRAKEN_SCHEDULER_SIMPLE_ARRAY
#define KRAKEN_SCHEDULER_FAIR           0x04

// Architecture codes for preprocessor
#define KRAKEN_ARCH_AVR                 0x11
#define KRAKEN_ARCH_X86_64              0x12
#define KRAKEN_ARCH_X86                 0x13
#define KRAKEN_ARCH_ARM                 0x14

// Maximum number of threads
#define KRAKEN_MAX_THREADS              4

//#if KRAKEN_ARCH==KRAKEN_ARCH_X86
#define KRAKEN_STACK_SIZE               0x400000
//#else
//#define KRAKEN_STACK_SIZE               0x400000
//#endif

//#if NDEBUG
#include <stdio.h>
//#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

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
    uint32_t e15;
    uint32_t e14;
    uint32_t e13;
    uint32_t e12;
    uint32_t ebx;
    uint32_t ebp;
#else
    #error "Architecture not defined or implemented for Kraken library!"
#endif
};

enum kraken_status {
    STOPPED, RUNNING, READY
};

struct kraken_thread {
    struct kraken_context context;
    enum kraken_status status;
    char* stack;
};

struct kraken_thread threads[KRAKEN_MAX_THREADS];
struct kraken_thread *current_thread;

typedef void (*function_type)();

void        kraken_initialize();
int         kraken_run(int);
int         kraken_start_thread();
static void kraken_guard();
bool        kraken_yield();
void        kraken_printt_state();

void kraken_print_state() {
    printf("Thread table address %p\n", threads);
    for (int i = 0; i < KRAKEN_MAX_THREADS; i++) {
        printf("\tThread %d stack starts at %p\n", i, 
            (uint64_t*)threads[i].context.rsp);
    }
}

int kraken_run(int return_code) {
    while (kraken_yield()) ;

    // Free thread stack memory when done
    for (int i = 0; i < KRAKEN_MAX_THREADS; i++) {
        if (&threads[i] != NULL) {
            free(threads[i].stack);
        }
    }

    exit(return_code);
}

void kraken_initialize_runtime() {
    current_thread = &threads[0];
    current_thread->status = RUNNING;
    current_thread->stack = (char*)malloc(KRAKEN_STACK_SIZE);
}

static void kraken_switch (struct kraken_context* old_ctx, struct kraken_context *new_ctx) {
    __asm__ volatile (
        "mov     %%rsp, 0x00(%0)\n\t"
        "mov     %%r15, 0x08(%0)\n\t"
        "mov     %%r14, 0x10(%0)\n\t"
        "mov     %%r13, 0x18(%0)\n\t"
        "mov     %%r12, 0x20(%0)\n\t"
        "mov     %%rbx, 0x28(%0)\n\t"
        "mov     %%rbp, 0x30(%0)\n\t"
        "mov     0x00(%1), %%rsp\n\t"
        "mov     0x08(%1), %%r15\n\t"
        "mov     0x10(%1), %%r14\n\t"
        "mov     0x18(%1), %%r13\n\t"
        "mov     0x20(%1), %%r12\n\t"
        "mov     0x28(%1), %%rbx\n\t"
        "mov     0x30(%1), %%rbp\n\t"
        "ret                    \n\t"
        : 
        : "r" (old_ctx), "r"(new_ctx)
    );
}

static void kraken_guard () {
    if (current_thread != &threads[0]) {
        current_thread->status = STOPPED;
        kraken_yield();
        assert(!"reachable");
    }
}

bool kraken_yield () {
    struct kraken_thread *other_thread;
    struct kraken_context *old_ctx, *new_ctx;

#if KRAKEN_SCHEDULER==KRAKEN_SCHEDULER_SIMPLE_ARRAY
    other_thread = current_thread;

    while (other_thread->status != READY) {
        // if the current thread is the last thread
        struct kraken_thread *next = ++other_thread;
        struct kraken_thread *after_last = &threads[KRAKEN_MAX_THREADS];
        if (next == after_last) {
            // set the current thread to the first thread
            other_thread = &threads[0];
        }
        if (other_thread == current_thread) {
            return false;
        }
    }
#endif

    if (current_thread->status != STOPPED) {
        current_thread->status = READY;
    }

    other_thread->status = RUNNING;
    old_ctx = &current_thread->context;
    new_ctx = &other_thread->context;
    current_thread = other_thread;

    kraken_print_state();

    // switch from old context to new context
    kraken_switch(old_ctx, new_ctx);

    printf("returned from yield\n");
    return true;
}


int kraken_start_thread (function_type func) {
    struct kraken_thread* new_thread;

#if KRAKEN_SCHEDULER==KRAKEN_SCHEDULER_SIMPLE_ARRAY
    // look for a home for the new thread;
    for (new_thread = &threads[0]; true ;new_thread++) {
        if (new_thread == &threads[KRAKEN_MAX_THREADS]) {
            return -1;
        } else if (new_thread->status == STOPPED) {
            break;
        }
    }
#endif

    new_thread->stack = (char*)malloc(KRAKEN_STACK_SIZE);
    
    if (!new_thread->stack) return -1;

#if KRAKEN_ARCH==KRAKEN_ARCH_X86_64
    *(uint64_t *)&(new_thread->stack[KRAKEN_STACK_SIZE -  8]) = (uint64_t)kraken_guard;
    *(uint64_t *)&(new_thread->stack[KRAKEN_STACK_SIZE - 16]) = (uint64_t)func;
    new_thread->context.rsp = (uint64_t)&(new_thread->stack[KRAKEN_STACK_SIZE - 16]);
    new_thread->status = READY;
#endif

    return 0;
}

#endif // KRAKEN_H

