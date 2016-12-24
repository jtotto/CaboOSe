#include "offsets.h"

.global engine
.global software_except

#define SYSTEM_MODE 0b11111
#define IRQ_MODE    0b10010
#define SVC_MODE    0b10011

/* Do a bunch of work to save the active task stack pointer. */
/* Modifies @scratch, stores @to_save */
.macro save_sp scratch to_save
    ldr \scratch, =(caboose + caboose_tasks_offset + tasks_active_offset)
    ldr \scratch, [\scratch]
    str \to_save, [\scratch, #task_sp_offset]
.endm

software_except:
    /********* SUPERVISOR ***********/
    cps #SYSTEM_MODE                    /* Jank immediately into system mode. */
    /********* SYSTEM ***********/
    stmfd sp!, { r4-r11, lr }           /* Save the callee-saved registers. */
    sub r4, sp, #4                      /* Make space for r0 and place the
                                         * resulting sp value in r4, where we
                                         * can touch it from SVC_MODE. */

    cps #SVC_MODE                       /* Jank back for good. */
    /********* SUPERVISOR ***********/
    mrs r5, spsr
    stmfd r4!, { r5 }                   /* Stack spsr. */
    save_sp r5 r4                       /* Using r5 as scratch, save the final
                                         * sp value (in r4) into the active
                                         * task. */

    ldr r5, [lr, #-4]
    bic r5, r5, #0xff000000 /* Remove the opcode from the svc instruction
                             * so we can get the index in syscall table. */

    ldr r6, =syscalls        /* Put the base of the syscall table in r6. */
    ldr r6, [r6, r5, lsl #2] /* r6 now points to the syscall. */
    blx r6                   /* Call the computed syscall handler. */
    str r0, [r4, #svcframe_sf_r0_offset] /* Save the return value to the frame
                                          * on the caller's stack. */

activate:
    /********* SUPERVISOR ***********/
    bl schedule             /* Find the next task to activate. */
#if 0
    tst r0, #0x80000000     /* Did we enter through an IRQ, or an SWI? */
    bne irq_return          /* => IRQ */
#endif

    /* If we're here, we're returning from SVC. */
    ldmia r0!, { r5 }       /* Load the spsr from the SVC frame into r5. */
    msr spsr, r5            /* Restore spsr. */
    ldmia r0!, {r3-r11, lr} /* Restore the user registers. */
    cps #SYSTEM_MODE        /* Jank into system mode for sp. */
    /********* SYSTEM ***********/
    mov sp, r0
    cps #SVC_MODE           /* Jank back to return. */
    /********* SUPERVISOR ***********/
    mov r0, r3                          /* r3 contains the saved return value */
    movs pc, lr                         /* This also restores cpsr from spsr. */

engine:
    push { fp, lr }
    b activate
