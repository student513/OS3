#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "lib/user/syscall.h"
void syscall_init (void);

/*1101형준 */
void halt(void);//pj1
void exit(int status);//pj1
pid_t exec (const char *cmd_line);//pj1
int wait (pid_t pid);//pj1
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned size);//pj1
int write (int fd, const void *buffer, unsigned size);//pj1
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);

/**/

/*191102 inseok : functions included*/
int fibonacci(int n);//Return N th value of Fibonacci sequence
int sum(int a, int b, int c, int d);//Return the sum of a, b, c and d
/**/

#endif /* userprog/syscall.h */
