#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int main() {
  typedef struct mystruct {

    int data;
    float f;

  } mystructure;

  mystructure *n = (mystructure *)malloc(sizeof(mystructure));

  (*n).data = 9;
  return 0;
}
