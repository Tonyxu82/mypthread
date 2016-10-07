#/*************************************************
#
#  CS533 Project: User level thread
#  File: mthread.s
#  Author:
#        Lai Xu  929272440 (xulai@pdx.edu)
#        Bo Chen 904039384 (chenbo@pdx.edu)
#  Date: Oct.08.2014
#  Desc: implmentation of thread start and switch.
#        the framework of this file is based on the
#        the source code of setjmp and longjmp in
#        Linux Kernel
#
#************************************************/
.text
.align 4
.globl thread_start
.type thread_start, @function

thread_start:
    pushq %rbx           # save current thread context to its stack
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    movq %rsp, (%rdi)    # save the rsp (stack) for the current thread to its correspoinding thread table entry

    movq (%rsi), %rsp    # set stack to current thread's stack
    pushq $thread_finish # prepare for thread exit, set "thread_finish" as the return address for the upcoming function
    movq 24(%rsi), %rdi  # set the arguments that will be used for the upcomming function
    movq 16(%rsi), %rax  # jump to the upcoming function
    jmp  *%rax

.size thread_start,.-thread_start

.text
.align 4
.globl thread_switch
.type thread_switch, @function
thread_switch:
    pushq %rbx           # save current thread context
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq %rsp, (%rdi)    # save current rsp to thread control block
    movq (%rsi), %rsp    # set stack to old thread's stack

    popq %r15            # restore old thread's context and let it run
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx
    ret

.size thread_switch,.-thread_switch
