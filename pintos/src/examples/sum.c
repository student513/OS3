#include <stdio.h>
//#include <syscall.h>
#include <string.h>
#include "userprog/syscall.h"
int main(int argc, char **argv) {
  int fib, sum4;
  fib = fibonacci(atoi(argv[1]));
  sum4 = sum(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
  printf("%d %d\n",fib,sum4);
  return 0;
}
