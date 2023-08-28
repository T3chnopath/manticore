;/**************************************************************************/
;/*                                                                        */
;/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
;/*                                                                        */
;/*       This software is licensed under the Microsoft Software License   */
;/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
;/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
;/*       and in the root directory of this software.                      */
;/*                                                                        */
;/**************************************************************************/
;
;
;/**************************************************************************/ 
;/**************************************************************************/ 
;/**                                                                       */ 
;/** ThreadX Component                                                     */ 
;/**                                                                       */ 
;/**   Thread                                                              */ 
;/**                                                                       */ 
;/**************************************************************************/ 
;/**************************************************************************/ 
;
;
    EXTERN     _tx_thread_execute_ptr
    EXTERN     _tx_thread_current_ptr
    EXTERN     _tx_timer_time_slice
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    EXTERN     _tx_execution_thread_enter
#endif
    
IRQ_MODE            EQU     0xD2            ; IRQ mode
USR_MODE            EQU     0x10            ; USR mode
SVC_MODE            EQU     0x13            ; SVC mode
SYS_MODE            EQU     0x1F            ; SYS mode

INT_MASK            EQU     0xC0            ; Interrupt bit mask
IRQ_MASK            EQU     0x80            ; Interrupt bit mask
#ifdef TX_ENABLE_FIQ_SUPPORT
FIQ_MASK            EQU     0x40            ; Interrupt bit mask
#endif

MODE_MASK           EQU     0x1F            ; Mode mask 
THUMB_MASK          EQU     0x20            ; Thumb bit mask

    EXTERN       _txm_system_mode_enter
    EXTERN       _txm_system_mode_exit
    EXTERN       _txm_ttbr1_page_table



;/**************************************************************************/ 
;/*                                                                        */ 
;/*  FUNCTION                                               RELEASE        */ 
;/*                                                                        */ 
;/*    _tx_thread_schedule                             Cortex-A7/MMU/IAR   */ 
;/*                                                           6.x          */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    Scott Larson, Microsoft Corporation                                 */
;/*                                                                        */
;/*  DESCRIPTION                                                           */ 
;/*                                                                        */ 
;/*    This function waits for a thread control block pointer to appear in */ 
;/*    the _tx_thread_execute_ptr variable.  Once a thread pointer appears */ 
;/*    in the variable, the corresponding thread is resumed.               */ 
;/*                                                                        */ 
;/*  INPUT                                                                 */ 
;/*                                                                        */ 
;/*    None                                                                */ 
;/*                                                                        */ 
;/*  OUTPUT                                                                */ 
;/*                                                                        */ 
;/*    None                                                                */ 
;/*                                                                        */ 
;/*  CALLS                                                                 */ 
;/*                                                                        */ 
;/*    None                                                                */ 
;/*                                                                        */ 
;/*  CALLED BY                                                             */ 
;/*                                                                        */ 
;/*    _tx_initialize_kernel_enter          ThreadX entry function         */ 
;/*    _tx_thread_system_return             Return to system from thread   */ 
;/*    _tx_thread_context_restore           Restore thread's context       */ 
;/*                                                                        */ 
;/*  RELEASE HISTORY                                                       */ 
;/*                                                                        */ 
;/*    DATE              NAME                      DESCRIPTION             */
;/*                                                                        */
;/*  09-30-2020      Scott Larson            Initial Version 6.1           */
;/*  xx-xx-xxxx      Yajun Xia               Modified comment(s),          */
;/*                                            Added thumb mode support,   */
;/*                                            resulting in version 6.x    */
;/*                                                                        */
;/**************************************************************************/
;VOID   _tx_thread_schedule(VOID)
;{
    RSEG    .text:CODE:NOROOT(2)
    PUBLIC  _tx_thread_schedule
#ifdef THUMB_MODE
    THUMB
#else
    ARM
#endif
_tx_thread_schedule

    ; Enter the scheduler.
    SVC     0

    ; We should never get here - ever!
_tx_scheduler_fault__
    B       _tx_scheduler_fault__
;}
; ****************************************************************************


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SWI_Handler
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    RSEG    .text:CODE:NOROOT(2)
    PUBLIC  SWI_Handler
#ifdef THUMB_MODE
    THUMB
#else
    ARM
#endif
SWI_Handler

    PUSH    {r0-r3, r12, lr}                ; Store the registers

    MRS     r0, spsr                        ; Get spsr
    TST     r0, #THUMB_MASK                 ; Occurred in Thumb state?
    ITTEE   NE
    LDRHNE  r1, [lr,#-2]                    ; Yes: Load halfword and...
    BICNE   r1, r1, #0xFF00                 ; ...extract comment field
    LDREQ   r1, [lr,#-4]                    ; No: Load word and...
    BICEQ   r1, r1, #0xFF000000             ; ...extract comment field

    ; r1 now contains SVC number

    ; The service call is handled here

    CMP     r1, #0                          ; Is it a schedule request?
    BEQ     _tx_handler_svc_schedule        ; Yes, go there

    CMP     r1, #1                          ; Is it a system mode enter request?
    BEQ     _tx_handler_svc_super_enter     ; Yes, go there

    CMP     r1, #2                          ; Is it a system mode exit request?
    BEQ     _tx_handler_svc_super_exit      ; Yes, go there

    LDR     r2, =0x123456
    CMP     r1, r2                          ; Is it an ARM request?
    BEQ     _tx_handler_svc_arm             ; Yes, go there

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Unknown SVC argument
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Unrecognized service call
    PUBWEAK _tx_handler_svc_unrecognized
_tx_handler_svc_unrecognized
    BKPT    0x0000
_tx_handler_svc_unrecognized_loop           ; We should never get here
    B       _tx_handler_svc_unrecognized_loop
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SVC 1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; At this point we have an SVC 1, which means we are entering 
    ; supervisor mode to service a kernel call.
_tx_handler_svc_super_enter
    ; Make sure that we have been called from the system mode enter location (security)
    LDR     r2, =_txm_system_mode_enter     ; Load the address of the known call point
    BIC     r2, #1                          ; Clear bit 0 of the symbol address (it is 1 in Thumb mode, 0 in ARM mode)
    MOV     r1, lr                          ; Get return address
    TST     r0, #THUMB_MASK                 ; Check if we come from Thumb mode or ARM mode
    ITE     NE
    SUBNE   r1, #2                          ; Calculate the address of the actual call (thumb mode)
    SUBEQ   r1, #4                          ; Calculate the address of the actual call (ARM mode)
    CMP     r1, r2                          ; Did we come from txm_module_manager_user_mode_entry?
    BNE     _tx_handler_svc_unrecognized    ; Return to where we came

    ; Clear the user mode flag in the thread structure
    LDR     r1, =_tx_thread_current_ptr     ; Load the current thread pointer address
    LDR     r2, [r1]                        ; Load current thread location from the pointer (pointer indirection)
    MOV     r1, #0                          ; Load the new user mode flag value (user mode flag clear -> not user mode -> system)
    STR     r1, [r2, #0x9C]                 ; Clear tx_thread_module_current_user_mode for thread

    ; Now we enter the system mode and return
    BIC     r0, r0, #MODE_MASK              ; clear mode field
    ORR     r0, r0, #SYS_MODE               ; system mode code
    MSR     SPSR_cxsf, r0                   ; Restore the spsr

    LDR     r1, [r2, #0xA8]                 ; Load the module kernel stack pointer
    CPS     #SYS_MODE                       ; Switch to SYS mode
    MOV     r3, sp                          ; Grab thread stack pointer
    MOV     sp, r1                          ; Set SP to kernel stack pointer
    CPS     #SVC_MODE                       ; Switch back to SVC mode
    STR     r3, [r2, #0xB0]                 ; Save thread stack pointer
#ifdef TXM_MODULE_KERNEL_STACK_MAINTENANCE_DISABLE
    ; do nothing
#else
    LDR     r3, [r2, #0xAC]                 ; Load the module kernel stack size
    STR     r3, [r2, #20]                   ; Set stack size
    LDRD    r0, r1, [r2, #0xA4]             ; Load the module kernel stack start and end
    STRD    r0, r1, [r2, #0x0C]             ; Set stack start and end
#endif

    ; Restore the registers and return
#if defined(THUMB_MODE)
    POP     {r0-r3, r12, lr}
#if defined(IAR_SIMULATOR)

;   /* The reason for adding this segment is that IAR's simulator
;      may not handle PC-relative instructions correctly in thumb mode.*/
    STR      lr, [sp, #-8]
    MRS      lr, SPSR    
    STR      lr, [sp, #-4]
    SUB      lr, sp, #8
    RFE      lr
#else
    SUBS    pc, lr, #0
#endif
#else
    LDMFD   sp!, {r0-r3, r12, pc}^
#endif


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SVC 2
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; At this point we have an SVC 2, which means we are exiting 
    ; supervisor mode after servicing a kernel call.
_tx_handler_svc_super_exit:
    ; Make sure that we have been called from the system mode exit location (security)
    LDR     r2, =_txm_system_mode_exit      ; Load the address of the known call point
    BIC     r2, #1                          ; Clear bit 0 of the symbol address (it is 1 in Thumb mode, 0 in ARM mode)
    MOV     r1, lr                          ; Get return address
    TST     r0, #THUMB_MASK                 ; Check if we come from Thumb mode or ARM mode
    ITE     NE
    SUBNE   r1, #2                          ; Calculate the address of the actual call (thumb mode)
    SUBEQ   r1, #4                          ; Calculate the address of the actual call (ARM mode)
    CMP     r1, r2                          ; Did we come from txm_module_manager_user_mode_entry?
    BNE     _tx_handler_svc_unrecognized    ; Return to where we came

    ; Set the user mode flag into the thread structure
    LDR     r1, =_tx_thread_current_ptr     ; Load the current thread pointer address
    LDR     r2, [r1]                        ; Load the current thread location from the pointer (pointer indirection)
    MOV     r1, #1                          ; Load the new user mode flag value (user mode enabled -> not system anymore)
    STR     r1, [r2, #0x9C]                 ; Set tx_thread_module_current_user_mode for thread

    ; Now we enter user mode (exit the system mode) and return
    BIC     r0, r0, #MODE_MASK              ; clear mode field
    ORR     r0, r0, #USR_MODE               ; user mode code
    MSR     SPSR_cxsf, r0                   ; Restore the spsr

    LDR     r1, [r2, #0xB0]                 ; Load the module thread stack pointer
    CPS     #SYS_MODE                       ; Switch to SYS mode
    MOV     r3, sp                          ; Grab kernel stack pointer
    MOV     sp, r1                          ; Set SP back to thread stack pointer
    CPS     #SVC_MODE                       ; Switch back to SVC mode
#ifdef TXM_MODULE_KERNEL_STACK_MAINTENANCE_DISABLE
    ; do nothing
#else
    LDR     r3, [r2, #0xBC]                 ; Load the module thread stack size
    STR     r3, [r2, #20]                   ; Set stack size
    LDRD    r0, r1, [r2, #0xB4]             ; Load the module thread stack start and end
    STRD    r0, r1, [r2, #0x0C]             ; Set stack start and end
#endif
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ARM Semihosting
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
_tx_handler_svc_arm
    
    ; *** TODO: handle semihosting requests or ARM angel requests ***
    
    ; Restore the registers and return
#if defined(THUMB_MODE)
    POP     {r0-r3, r12, lr}
#if defined(IAR_SIMULATOR)

;   /* The reason for adding this segment is that IAR's simulator
;      may not handle PC-relative instructions correctly in thumb mode.*/
    STR      lr, [sp, #-8]
    MRS      lr, SPSR    
    STR      lr, [sp, #-4]
    SUB      lr, sp, #8
    RFE      lr
#else
    SUBS    pc, lr, #0
#endif

#else
    LDMFD   sp!, {r0-r3, r12, pc}^
#endif

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SVC 0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; At this point we have an SVC 0: enter the scheduler.
_tx_handler_svc_schedule

    POP   {r0-r3, r12, lr}                  ; Restore the registers


    ; This code waits for a thread control block pointer to appear in 
    ; the _tx_thread_execute_ptr variable.  Once a thread pointer appears 
    ; in the variable, the corresponding thread is resumed.
;
;    /* Enable interrupts.  */
;
#ifdef TX_ENABLE_FIQ_SUPPORT
    CPSIE   if                              ; Enable IRQ and FIQ interrupts
#else
    CPSIE   i                               ; Enable IRQ interrupts
#endif
;
;    /* Wait for a thread to execute.  */
;    do
;    {
    LDR     r1, =_tx_thread_execute_ptr     ; Address of thread execute ptr
;
__tx_thread_schedule_loop
    LDR     r0, [r1]                        ; Pickup next thread to execute
    CMP     r0, #0                          ; Is it NULL?
    BEQ     __tx_thread_schedule_loop       ; If so, keep looking for a thread
    ; }
    ; while(_tx_thread_execute_ptr == TX_NULL);

   ; Yes! We have a thread to execute. Lockout interrupts and transfer control to it.
#ifdef TX_ENABLE_FIQ_SUPPORT
    CPSID   if                              ; Disable IRQ and FIQ interrupts
#else
    CPSID   i                               ; Disable IRQ interrupts
#endif

;   /* Setup the current thread pointer.  */
    ; _tx_thread_current_ptr =  _tx_thread_execute_ptr;

    LDR     r1, =_tx_thread_current_ptr     ; Pickup address of current thread
    STR     r0, [r1]                        ; Setup current thread pointer

;   /* Increment the run count for this thread.  */
    ; _tx_thread_current_ptr -> tx_thread_run_count++;

    LDR     r2, [r0, #4]                    ; Pickup run counter
    LDR     r3, [r0, #24]                   ; Pickup time-slice for this thread
    ADD     r2, r2, #1                      ; Increment thread run-counter
    STR     r2, [r0, #4]                    ; Store the new run counter

;   /* Setup time-slice, if present.  */
    ; _tx_timer_time_slice =  _tx_thread_current_ptr -> tx_thread_time_slice;

    LDR     r2, =_tx_timer_time_slice       ; Pickup address of time-slice variable
    STR     r3, [r2, #0]                    ; Setup time-slice

#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))

;   /* Call the thread entry function to indicate the thread is executing.  */

    MOV     r5, r0                          ; Save r0
    BL      _tx_execution_thread_enter      ; Call the thread execution enter function
    MOV     r0, r5                          ; Restore r0
#endif

    ; Determine if an interrupt frame or a synchronous task suspension frame is present.
    CPS     #SYS_MODE                       ; Enter SYS mode
    LDR     sp, [r0, #8]                    ; Switch to thread stack pointer
    POP     {r4, r5}                        ; Pickup the stack type and saved CPSR
    CPS     #SVC_MODE                       ; Enter SVC mode

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;/
    ; Set up MMU for module.
    LDR     r2, [r0, #0x94]                 ; Pickup the module pointer
    CMP     r2, #0                          ; Valid module pointer?
    IT      NE
    LDRNE   r2, [r2, #0x64]                 ; Load ASID
    ; Otherwise, ASID 0 & master table will be loaded.
    ; Is ASID already loaded?
    MRC     p15, 0, r1, c13, c0, 1          ; Read CONTEXTIDR into r1
    CMP     r1, r2
    ; If so, skip MMU setup.
    BEQ     _tx_skip_mmu_update
    ; New ASID & TTBR values to load
    DSB
    ISB
    ; Load new ASID and TTBR
    LDR     r1, =_txm_ttbr1_page_table      ; Load master TTBR
    ORR     r1, r1, #0x9                    ; OR it with #TTBR0_ATTRIBUTES
    MCR     p15, 0, r1, c2, c0, 0           ; Change TTBR to master
    ISB
    DSB
    MCR     p15, 0, r2, c13, c0, 1          ; Change ASID to new value
    ISB
    ; Change TTBR to new value
    ADD     r1, r1, r2, LSL #14             ; r1 = _txm_ttbr1_page_table + (asid << 14)
    MCR     p15, 0, r1, c2, c0, 0           ; Change TTBR to new value

    ; refresh TLB
    MOV     r2, #0
    DSB
    MCR     p15, 0, r2, c8, c7, 0           ; Invalidate entire unified TLB
    MCR     p15, 0, r2, c7, c5, 0           ; Invalidate all instruction caches to PoU
    MCR     p15, 0, r2, c7, c5, 6           ; Invalidate branch predictor
    DSB
    ISB

    ; test address translation
    ;mcr p15, 0, r0, c7, c8, 0
    
_tx_skip_mmu_update
    ; **************************************************************************
    
    CMP     r4, #0                          ; Check for synchronous context switch
    BEQ     _tx_solicited_return

    MSR     SPSR_cxsf, r5                   ; Setup SPSR for return
    LDR     r1, [r0, #8]                    ; Get thread SP
    LDR     lr, [r1, #0x40]                 ; Get thread PC
    CPS     #SYS_MODE                       ; Enter SYS mode

#ifdef TX_ENABLE_VFP_SUPPORT
    LDR     r2, [r0, #144]                  ; Pickup the VFP enabled flag
    CMP     r2, #0                          ; Is the VFP enabled?
    BEQ     _tx_skip_interrupt_vfp_restore  ; No, skip VFP interrupt restore
    VLDMIA  sp!, {D0-D15}                   ; Recover D0-D15
    VLDMIA  sp!, {D16-D31}                  ; Recover D16-D31
    LDR     r4, [sp], #4                    ; Pickup FPSCR
    VMSR    FPSCR, r4                       ; Restore FPSCR
    CPS     #SVC_MODE                       ; Enter SVC mode
    LDR     lr, [r1, #0x144]                ; Get thread PC
    CPS     #SYS_MODE                       ; Enter SYS mode
_tx_skip_interrupt_vfp_restore
#endif

    POP     {r0-r12, lr}                    ; Restore registers
    ADD     sp, #4                          ; Fix stack pointer (skip PC saved on stack)
    CPS     #SVC_MODE                       ; Enter SVC mode

#if defined(THUMB_MODE) && defined(IAR_SIMULATOR)

;   /* The reason for adding this segment is that IAR's simulator
;      may not handle PC-relative instructions correctly in thumb mode.*/
    STR      lr, [sp, #-8]
    MRS      lr, SPSR    
    STR      lr, [sp, #-4]
    SUB      lr, sp, #8
    RFE      lr
#else
    SUBS    pc, lr, #0                      ; Return to point of thread interrupt
#endif

_tx_solicited_return
    MOV     r2, r5                          ; Move CPSR to scratch register
    CPS     #SYS_MODE                       ; Enter SYS mode
    
#ifdef __ARMVFP__
    LDR     r1, [r0, #144]                  ; Pickup the VFP enabled flag
    CMP     r1, #0                          ; Is the VFP enabled?
    BEQ     _tx_skip_solicited_vfp_restore  ; No, skip VFP solicited restore
    VLDMIA  sp!, {D8-D15}                   ; Recover D8-D15
    VLDMIA  sp!, {D16-D31}                  ; Recover D16-D31
    LDR     r4, [sp], #4                    ; Pickup FPSCR
    VMSR    FPSCR, r4                       ; Restore FPSCR
_tx_skip_solicited_vfp_restore
#endif
    
    POP     {r4-r11, lr}                    ; Restore registers
    MOV     r1, lr                          ; Copy lr to r1 to preserve across mode change
    CPS     #SVC_MODE                       ; Enter SVC mode
    MSR     SPSR_cxsf, r2                   ; Recover CPSR
    MOV     lr, r1                          ; Deprecated return via r1, so copy r1 to lr and return via lr
#if defined(THUMB_MODE) && defined(IAR_SIMULATOR)

;   /* The reason for adding this segment is that IAR's simulator
;      may not handle PC-relative instructions correctly in thumb mode.*/
    STR      lr, [sp, #-8]
    MRS      lr, SPSR    
    STR      lr, [sp, #-4]
    SUB      lr, sp, #8
    RFE      lr
#else
    SUBS    pc, lr, #0                      ; Return to thread synchronously
#endif

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; End SWI_Handler
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    
#ifdef __ARMVFP__
    PUBLIC  tx_thread_vfp_enable
#ifdef THUMB_MODE
    THUMB
#else
    ARM
#endif
tx_thread_vfp_enable
    MRS     r0, CPSR                        ; Pickup current CPSR
#ifdef TX_ENABLE_FIQ_SUPPORT
    CPSID   if                              ; Disable IRQ and FIQ
#else
    CPSID   i                               ; Disable IRQ
#endif
    LDR     r2, =_tx_thread_current_ptr     ; Build current thread pointer address
    LDR     r1, [r2]                        ; Pickup current thread pointer
    CMP     r1, #0                          ; Check for NULL thread pointer
    BEQ     restore_ints                    ; If NULL, skip VFP enable
    MOV     r2, #1                          ; Build enable value
    STR     r2, [r1, #144]                  ; Set the VFP enable flag (tx_thread_vfp_enable field in TX_THREAD)
    B       restore_ints

    PUBLIC  tx_thread_vfp_disable
#ifdef THUMB_MODE
    THUMB
#else
    ARM
#endif
tx_thread_vfp_disable
    MRS     r0, CPSR                        ; Pickup current CPSR
#ifdef TX_ENABLE_FIQ_SUPPORT
    CPSID   if                              ; Disable IRQ and FIQ
#else
    CPSID   i                               ; Disable IRQ
#endif
    LDR     r2, =_tx_thread_current_ptr     ; Build current thread pointer address
    LDR     r1, [r2]                        ; Pickup current thread pointer
    CMP     r1, #0                          ; Check for NULL thread pointer
    BEQ     restore_ints                    ; If NULL, skip VFP disable
    MOV     r2, #0                          ; Build disable value
    STR     r2, [r1, #144]                  ; Clear the VFP enable flag (tx_thread_vfp_enable field in TX_THREAD)

restore_ints:
    TST     r0, #IRQ_MASK
    BNE     no_irq
    CPSIE   i
no_irq:
#ifdef TX_ENABLE_FIQ_SUPPORT
    TST     r0, #FIQ_MASK
    BNE     no_fiq
    CPSIE   f
no_fiq:
#endif
    BX      lr

    END

