#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  if (fork() == 0) {
    /* Child A code */
    printf("Anshuman ");
    return 0;
  } else {
    wait(0);
    if (fork() == 0) {
      /* Child B code */
      printf("Nitin ");
      return 0;
    } else {
      /* Parent Code */
      wait(0);
      printf("Komawar\n");
      return 0;
    }
  }
  return 0;
}
