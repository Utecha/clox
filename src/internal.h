#ifndef lox_internal_h
#define lox_internal_h

/*
 * The CLI, for now at least, is very simple to unwind.
 * These definitions exist as a way to provide a proper error
 * exitcode without the use of things like setjmp().
 *
 * This allows memory cleanup to occur even during an error as
 * neither the CLI nor the VM will be forced out through the
 * use of exit().
*/

int getExit(void);
void setExit(int code);

#endif // lox_internal_h
