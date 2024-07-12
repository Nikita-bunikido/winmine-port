#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "common.h"

static int _fmode = 0;

int _adjust_fdiv;

static char prompt[256];
char *_acmdln = prompt;

static int _commode;

/* TODO: Find out usage */
unsigned int _controlfp(unsigned int new, unsigned int mask) {
    TRACE("%u %u", new, mask);
}

/* TODO: Find out usage */
void __set_app_type(int at) {
    TRACE("%i", at);
}

int *__p__fmode(void) {
    TRACE_NOARGS();
    return &_fmode;
}

/* TODO: Find out usage */
int _except_handler3( /*
   PEXCEPTION_RECORD exception_record,
   PEXCEPTION_REGISTRATION registration,
   PCONTEXT context,
   PEXCEPTION_REGISTRATION dispatcher
    */
   void *exception_record,
   void *registration,
   void *context,
   void *dispatcher
) {
    TRACE("%p %p %p %p", exception_record, registration, context, dispatcher);
}

/* TODO: Find out usage */
void __setusermatherr( /*
   _HANDLE_MATH_ERROR pf
   */
    void *pf
) {
    TRACE("%p", pf);
}

/* TODO: Find out usage */
void _initterm( /* 
   PVFV *,
   PVFV * */
   void *first, void *last
) {
    TRACE("%p %p", first, last);
}

/* TODO: Find out usage */
int __getmainargs(int *argc, char ***argv, char ***env, int do_wild_card, int *start_info) {
    TRACE("%p %p %p %i %p", argc, argv, env, do_wild_card, start_info);
}

int *__p__commode(void) {
    TRACE_NOARGS();
    return &_commode;
}

void _cexit(void) {
    TRACE_NOARGS();

    /* Never gets called, because of non-MZ header */
    __builtin_unreachable();
}

/* TODO: Find out usage */
int __attribute__((cdecl)) _XcptFilter(unsigned long xcptnum, void *pxcptinfoptrs) {
    TRACE("%lu %p", xcptnum, pxcptinfoptrs);
}

void _c_exit(void) {
    TRACE_NOARGS();
}