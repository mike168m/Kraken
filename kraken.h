#ifndef KRAKEN_H
#define KRAKEN_H

#define KRAKEN_VERSION "0.9.1"

// Scheduler implementation codes for preprocessor
#define KRAKEN_SCHEDULER_ROUND_ROBIN    0x01
#define KRAKEN_SCHEDULER_FIFO           0x02
#define KRAKEN_SCHEDULER_FAIR           0x04

// Architecture codes for preprocessor
#define KRAKEN_ARCH_AVR                 0x11
#define KRAKEN_ARCH_X86_64              0x12
#define KRAKEN_ARCH_X86                 0x13
#define KRAKEN_ARCH_ARM                 0x14

// Maximum number of threads
#if !defined(KRAKEN_MAX_THREADS)
#define KRAKEN_MAX_THREADS              4
#endif

// Maxium stack size
#if !defined( KRAKEN_STACK_SIZE )
#if KRAKEN_ARCH==KRAKEN_ARCH_X86
#define KRAKEN_STACK_SIZE               1024 * 1024 * 2  // use 2mb stacks for 64bit architectures
#endif
#endif

// architecture selection
#ifndef KRAKEN_ARCH
#if (defined(__x86_64) || defined(__x86_64__)) && (__x86_64==1 || __x86_64__==1)
#define KRAKEN_ARCH KRAKEN_ARCH_X86_64
#elif (defined(__i386) || defined(__i386__)) && (__i386==1 || __i386__==1)
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
    STOPPED, RUNNING, READY
};

struct kraken_thread {
    struct kraken_context context;
    enum kraken_status status;
    char* stack;
};

struct kraken_runtime {
    struct kraken_thread threads[KRAKEN_MAX_THREADS];
    struct kraken_thread *current_thread;
};

typedef void (*function_type)();

void        kraken_initialize();
void        kraken_run(int);
int         kraken_start_thread();
static 
void        kraken_guard();
bool        kraken_yield();
void        kraken_printt_state();
void        kraken_switch (struct kraken_context*, struct kraken_context*);


void kraken_print_state (struct kraken_runtime* runtime) {
#ifdef KRAKEN_DEBUG
    printf("Thread table address %p\n", runtime->threads);
    for (int i = 0; i < KRAKEN_MAX_THREADS; i++) {
        printf("\tThread %d stack memory at %p\n", i, (uint64_t*)&runtime->threads[i]);
        printf("\tThread %d stack starts at %p\n", i, (uint64_t*)runtime->threads[i].context.rsp);
    }
#endif
}

void __attribute__((noreturn)) kraken_run (kraken_runtime* runtime, int return_code) {
    while (kraken_yield()) ;

    // Free thread stack memory when done
    for (int i = 0; i < KRAKEN_MAX_THREADS; i++) {
        if (&runtime->threads[i] != NULL) {
            free(runtime->threads[i].stack);
        }
    }

    exit(return_code);
}

void kraken_initialize_runtime() {
    current_thread = &threads[0];
    current_thread->status = RUNNING;
    current_thread->stack = (char*)malloc(KRAKEN_STACK_SIZE);
}

__asm__ (
    ".globl _kraken_switch, kraken_switch\n\t"
    "_kraken_switch:         \n\t"
    "kraken_switch:         \n\t"
#if KRAKEN_ARCH==KRAKEN_ARCH_X86_64
    "movq     %rsp, 0x00(%rdi)\n\t"
    "movq     %r15, 0x08(%rdi)\n\t"
    "movq     %r14, 0x10(%rdi)\n\t"
    "movq     %r13, 0x18(%rdi)\n\t"
    "movq     %r12, 0x20(%rdi)\n\t"
    "movq     %rbx, 0x28(%rdi)\n\t"
    "movq     %rbp, 0x30(%rdi)\n\t"
    "movq     0x00(%rsi), %rsp\n\t"
    "movq     0x08(%rsi), %r15\n\t"
    "movq     0x10(%rsi), %r14\n\t"
    "movq     0x18(%rsi), %r13\n\t"
    "movq     0x20(%rsi), %r12\n\t"
    "movq     0x28(%rsi), %rbx\n\t"
    "movq     0x30(%rsi), %rbp\n\t"
    "ret                      \n\t"
#elif KRAKEN_ARCH==KRAKEN_ARCH_X86
    "movl     %esp, 0x00(%edi)\n\t"
    "movl     %ebx, 0x28(%edi)\n\t"
    "movl     %ebp, 0x30(%edi)\n\t"
    "movl     0x00(%esi), %esp\n\t"
    "movl     0x28(%esi), %ebx\n\t"
    "movl     0x30(%esi), %ebp\n\t"
#elif KRAKEN_ARCH==KRAKEN_ARCH_AVR

#endif
);

static void kraken_guard (struct kraken_runtime* runtime) {
    if (current_thread != &runtime->threads[0]) {
        current_thread->status = STOPPED;
        kraken_yield();
    }
}

bool kraken_yield () {
    struct kraken_thread *other_thread;
    struct kraken_context *old_ctx, *new_ctx;

#if KRAKEN_SCHEDULER==KRAKEN_SCHEDULER_ROUND_ROBIN
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

    //printf("returned from yield\n");
    return true;
}


int kraken_start_thread (function_type func) {
    struct kraken_thread* new_thread;

#if KRAKEN_SCHEDULER==KRAKEN_SCHEDULER_ROUND_ROBIN
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
#elif KRAKEN_ARCH==KRAKEN_ARCH_X86
    *(uint32_t *)&(new_thread->stack[KRAKEN_STACK_SIZE -  4]) = (uint32_t)kraken_guard;
    *(uint32_t *)&(new_thread->stack[KRAKEN_STACK_SIZE -  8]) = (uint32_t)func;
    new_thread->context.esp = (uint32_t)&(new_thread->stack[KRAKEN_STACK_SIZE - 8]);
#endif

    new_thread->status = READY;

    return 0;
}

#endif // KRAKEN_H

