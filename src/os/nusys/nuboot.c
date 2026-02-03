#include "common.h"
#include "nu/nusys.h"
#include "functions.h"
#include "variables.h"

#if !VERSION_PAL
NOP_FIX
#endif

u64 nuMainStack[NU_SC_STACK_SIZE / sizeof(u64)];
static OSThread IdleThread; // idle thread, id 1
static OSThread MainThread; // id 3
static u64 IdleStack[NU_SC_STACK_SIZE / sizeof(u64)];

void (*nuIdleFunc)(void);
void __osInitialize_autodetect(void);

/*!
 * @brief Entry Point from MIPS Assembly
 *
 * Create an idle thread and use boot idle as the thread start point
 *
 * Then enter boot idle
 */
void nuBoot(void) {
    osInitialize();
#if VERSION_PAL
    __osInitialize_autodetect();
#endif
    osCreateThread(&IdleThread, NU_IDLE_THREAD_ID, boot_idle, NULL, &IdleStack[NU_SC_STACK_SIZE / sizeof(u64)], 10);
    osStartThread(&IdleThread);
}

/*!
 * @brief Initialize nusys and os functionality and boot into the game
 *
 * Establish communication with the southbridge, establishing comms with the reality coprocessor, sets NTSC of PAL based
 * on the compiler preprocessor definitions
 *
 * Create the scheduler with the graphics, audio, and scheduler threads started on the system
 *
 * Start the boot main thread in main.c (paper mario game application entry point)
 *
 * Idle thread then lowers its priority on the scheduler and executes a callback (which is probably a system wait or some other function)
 */
void boot_idle(void* data) {
    nuIdleFunc = NULL;

    nuPiInit();
    nuScCreateScheduler(OS_VI_NTSC_LAN1, 1);
    osViSetSpecialFeatures(OS_VI_GAMMA_OFF | OS_VI_GAMMA_DITHER_OFF | OS_VI_DIVOT_ON | OS_VI_DITHER_FILTER_ON);
    osCreateThread(&MainThread, NU_MAIN_THREAD_ID, boot_main, NULL, &nuMainStack[NU_SC_STACK_SIZE / sizeof(u64)], NU_MAIN_THREAD_PRI);
    osStartThread(&MainThread);
    osSetThreadPri(&IdleThread, NU_IDLE_THREAD_PRI);

    while (1) {
        if (nuIdleFunc != NULL) {
            nuIdleFunc();
        }
    }
}
