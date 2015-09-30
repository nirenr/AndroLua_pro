#ifndef __ASM_UCONTEXT_H
#define __ASM_UCONTEXT_H

typedef struct ucontext {
 unsigned long uc_flags;
 struct ucontext *uc_link;
 stack_t uc_stack;
 struct sigcontext uc_mcontext;
 sigset_t uc_sigmask;
} ucontext_t;

#endif
