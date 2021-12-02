#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, const char * argv[]) {
  char *buffer = malloc(100000);
  int saved, restore = 0, size = 100000; 

  while(1) {
    printf("> ");
    fflush(0);
    memset(&buffer[0], 0, size);
    read(STDIN_FILENO, buffer, 100000);
    buffer[strlen(buffer) - 1] = 0;
    size = strlen(buffer);
    fflush(0);


    if(strchr(buffer, '>') != NULL) {
      char *token = strrchr(buffer, '>');
      char *filename = token + 2;
      buffer[strlen(buffer) - 1] = 0;

      int pfd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);

      token = 0;
      saved = dup(1);
      close(1);
      dup(pfd);
      close(pfd);
      restore = 1;
    }

    
    if(!strncmp(buffer, "dir /f", 6)) {
      char path[200];
      getcwd(path, 200);
      printf("current working directory: %s\n\n", path);
    } else if(!strncmp(buffer, "dir", 3)) {
      char path[200], *token;
      getcwd(path, 200);
      token = strrchr(path, '/');
      printf("current working directory: %s\n\n", token + 1);
    } else if(!strncmp(buffer, "list", 4)) {
      DIR *d;
      struct dirent *dir;
      d = opendir(".");
      if (d) {
        while ((dir = readdir(d)) != NULL) {
          printf("%s", dir->d_name);
          if(dir->d_type == DT_DIR) printf("%*s\n", (int)(50 - strlen(dir->d_name)), "[DIRECTORY]");
          else printf("%*s\n", (int)(50 - strlen(dir->d_name)), "[FILE]"); 
        }
        closedir(d);
      }
      printf("\n");
    } else if(!strncmp(buffer, "cd", 2)) {
      char *token = strrchr(buffer, ' ');
      char *dirname = token + 1;

      if(!chdir(dirname)) printf("Successfully entered directory: %s\n\n", dirname);
      else printf("Could not enter directory: %s\n\n", dirname);

    } else if(!strncmp(buffer, "exit", 4)) {
      printf("Program Terminated\n");
      return 0; 
    } else printf("Invalid Command!\n\n");

    if(restore) {
      fflush(stdout);
      dup2(saved, 1);
      close(saved);
    }
  }
  return 0;
}
