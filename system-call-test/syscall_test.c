#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int main() {
  long ret = syscall(436);
  printf("jongwoocall returns: %ld\n", ret);
  return 0;
}
