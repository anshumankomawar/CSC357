#include <stdio.h>

int main(int argc, char* argv[]) {
  int height = -1;
  int level = 1;
  int spaces, stemlevel; 
  int _;

  while(height < 0 || height > 10) {
    printf("Enter the height of the tree: ");
    scanf("%d", &height);
  }

  spaces = ((2*height) - 2) / 2;
  
<<<<<<< HEAD
  while(level <= height) {
    int stars = 2*level - 1;
    
    for(_ = 0; _ < spaces; _++) {
      printf(" ");
    }

    for(_ = 0; _ < stars; _++) {
      printf("*");
    }

    printf("\n");
    level += 1;
    spaces -= 1;
  }

  for(stemlevel = 0; stemlevel < 3; stemlevel++) {
    for(_ = 0; _ < ((2*height) - 2) / 2; _++) {
      printf(" ");
    }
    printf("*\n");
  }
=======
  //while(level < 0) {
  //  int spaces = ;
  //  int stars = ;
  //  
  //  for(int leftspace = 0; leftspace <= ; leftspace++) {
  //    printf(" ");
  //  }

  //  while(stars > 0) {
  //  }

  //  while(spaces > 0) {
  //    printf(" ");
  //  }

  //  printf();
  //}
>>>>>>> f915dd1ecd6891a6fa817901eb573d69c9d279a8

  return 0;
}
