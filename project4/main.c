//
//  main.c
//  project4
//
//  Created by Anshuman Komawar on 11/10/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <dirent.h>
#include <sys/stat.h>

int fd[2];
void pipes(int i);
int search(DIR*d, char *filename);
int isDirectory(const char *path);

void printdir(char *dir, int depth)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    if((dp = opendir(dir)) == NULL) {
        fprintf(stderr,"cannot open directory: %s\n", dir);
        return;
    }
    chdir(dir);
    while((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name,&statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            /* Found a directory, but ignore . and .. */
            if(strcmp(".",entry->d_name) == 0 ||
                strcmp("..",entry->d_name) == 0)
                continue;
            printf("%*s%s/\n",depth,"",entry->d_name);
            /* Recurse at a new indent level */
            printdir(entry->d_name,depth+4);
        }
        else printf("%*s%s\n",depth,"",entry->d_name);
    }
    chdir("..");
    closedir(dp);
}

int main(int argc, const char * argv[]) {
    char *buffer = malloc(1073741824);
    int parent = getpid();
    int *kpid = mmap(NULL, (10*sizeof(int)), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    char *inputs = mmap(NULL, 1000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    int *numchild = mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    int *kiddone = mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    *kiddone = 0;
    
    signal(SIGUSR1, pipes);
    int saveST = dup(STDIN_FILENO);
    
    pipe(fd);
    printf("\033[32m");
    printf("\nEnter \"list\" to see all child processes, their PID, and their task");
    printf("\nEnter \"kill <PID>\" to kill an individual child process and stop its find attempt");
    printf("\nEnter \"quit\" or \"q\" to end the program");
    printf("\nEnter \"find \"word\"\" to find a word in a file");
    printf("\nEnter \"find filename\" to search for a word");
    printf("\nUse the flags \"-s\" to search subdirectories or \"-f:filetype\" to search by filetype");
    fflush(0);
    
    while(1) {
        printf("\033[01;34m");
        printf("\nFindStuff: ");
        printf("\033[0m");
        printf("$ ");
        fflush(0);
        
        dup2(saveST, STDIN_FILENO);
        memset(&buffer[0], 0, sizeof(buffer));
        
        read(STDIN_FILENO, buffer, 1073741824);
        fflush(0);
        
        if(strchr(buffer, 'q') != NULL || !strncmp(buffer, "quit", 4)) {
            int idx = 0;
            while(idx++ < 10) {
                if (kpid[idx] != 0)kill(kpid[idx], SIGKILL);
            }
            printf("\nProgram Ended.\n\n");
            munmap(&kpid, 10*sizeof(int));
            munmap(&inputs, 1000);
            munmap(&numchild, 4);
            munmap(&kiddone, 4);
            free(buffer);
            wait(0);
            return 0;
        } else if(!strncmp(buffer, "list", 4)) {
            kpid[4] = 1;
            int pid_idx = 0;
            while(pid_idx++ < 10) {
                if(kpid[pid_idx] != 0) printf("\nProcess %d [PID: %d Task: %s]\n\n", pid_idx, kpid[pid_idx], inputs + pid_idx*100);
            }
        } else if(!strncmp(buffer, "kill", 4)) {
            char pid[10000];
            memset(&pid[0], 0, sizeof(pid));
            char *pid_idx = strchr(buffer, ' ');
            strncpy(pid, buffer + (pid_idx - buffer) + 1, 8);
            pid[9] = '\0';
            
            int idx = 0;
            while(idx++ < 10) {
                if (atoi(pid) == kpid[idx]) {
                    *numchild -= 1;
                    kpid[idx] = 0;
                    kill(kpid[idx], SIGKILL);
                    waitpid(atoi(pid), 0, 0);
                }
            }
        } else {
            if (*numchild < 9) {
                (*numchild) += 1;
                if(fork() == 0) {
                    if (!strstr(buffer, " -f")) {
                        DIR *cwd = opendir(getcwd(NULL, 5000));
                        if(!strstr(buffer, " -s")) {
                            printdir(".", 0);
//                            printf("%d\n", search(cwd, "hello.txt"));
                        };
                    }
                }
            } else {
                write(STDOUT_FILENO, "Limit of 10 processes reached.", 31);
                fsync(fd[1]);
            }
        }
    }
    return 0;
}

int search(DIR*cwd, char *filename) {
    printf("%s", cwd);
    struct dirent *dir;
    printf("\n");
    while ((dir = readdir(cwd)) != NULL) {
        if(!isDirectory(dir->d_name) && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            return search(opendir(dir->d_name), filename);
        }
        if(!strcmp(dir->d_name, filename)) return 1;
    }
    return 0;
}

int isDirectory(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

void pipes(int i) {
    dup2(fd[0], STDIN_FILENO);
}
