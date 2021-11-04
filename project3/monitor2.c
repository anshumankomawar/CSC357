#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <string.h>

void overridesig()
{
    printf("\nProcess cannot be killed. Enter 'q' to end the program.\n");
}

int main()
{
    signal(SIGINT, overridesig);
    signal(SIGTSTP, overridesig);
    signal(SIGKILL, overridesig);
    signal(SIGTERM, overridesig);
    signal(SIGSTOP, overridesig);

    int *kpid = mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    int *running = mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    char *buffer = mmap(NULL, 2000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    buffer = getcwd(NULL, 100);
    char uIn[50];
    
    struct stat st;
    *running = 1;
    fflush(NULL);
    while(1)
    {

    if (fork() == 0)
    {
        while(1)
        {
            chdir(buffer);
            DIR *d = opendir(buffer);
            *kpid = getpid();
            printf("\nKid Pid: %d", *kpid);
            printf("\nEnter 'list' to see the current directory,");
            printf("\nOr enter 'q' to end the program,");
            printf("\nOr enter a file name to get it's status. \n");
            
            printf("\033[01;34m");
            printf("\nStat Prog: %s", buffer);
            printf("\033[0m");
            printf("$");

            scanf("%s", uIn);
            *running += 1;
            if (strcmp(uIn, "..") == 0)
            {
                chdir("..");
                strcpy(buffer, getcwd(NULL, 100));
                fflush(0);
            }
            else if (uIn[0] == '/')
            {
                char * dname = uIn + 1;
                if (stat(dname, &st) != 0)
                {
                    printf("\nFolder does not exist.\n");
                    break;
                } 
                strcat(buffer, uIn);
                *buffer = *getcwd(NULL, 100);
                printf("\nEntering Directory: %s\n", uIn);
            }
            else if (strcmp(uIn, "q") == 0)
            {
                *running = 0;
                printf("\nEnding Program...\n");
            }
            else if (strcmp(uIn, "list") == 0)
            {
                struct dirent *dp;
                printf("\n");
                while ((dp = readdir(d)) != NULL)
                {
                    printf("%s ", (dp->d_name));
                    if (dp->d_type == DT_DIR)
                    {
                        printf(" - is a Directory");
                    }
                    printf("\n");
                }
                printf("\n");
                closedir(d);
            }
            else if ((stat(uIn, &st) != 0))
            {
                printf("\nFile does not exist or incorrect command\n");
            }
            else
            {
                stat(uIn, &st);
                printf("\nID of device containing file: %d", st.st_dev);
                printf("\nInode number: %llu", st.st_ino);
                printf("\nFile type and mode: %d", st.st_mode);
                printf("\nNumber of hard links: %d", st.st_nlink);
                printf("\nUser ID of owner: %d", st.st_uid);
                printf("\nGroup ID of owner: %d", st.st_gid);
                printf("\nDevice ID (if special file): %d", st.st_rdev);
                printf("\nTotal size, in bytes: %lld", st.st_size);
                printf("\nBlock size for filesystem I/O: %d", st.st_blksize);
                printf("\nNumber of 512B blocks allocated: %lld\n", st.st_blocks);
            }
        }
        return 0;
    }
    else
    {
        int g;
        wait(&g);
        struct timespec reqtime;
        reqtime.tv_sec = 10;
        reqtime.tv_nsec = 0;
        struct timespec remaining;
        remaining.tv_sec = 0;
        remaining.tv_nsec = 0;
        if (nanosleep(&reqtime, &remaining) < 0)
        {
            while (remaining.tv_sec > 0 && nanosleep(&reqtime, &remaining) < 0)
            {
                reqtime.tv_sec = remaining.tv_sec;
            }   
        }
        if (*running == 0)
        {
            printf("\nProgram terminated.\n\n");
            kill(*kpid, SIGKILL);
            if (waitpid(*kpid, &g, WNOHANG | WUNTRACED) != -1)
            {
                kill(*kpid, SIGKILL);
            }  
            munmap(&kpid, 4);
            munmap(&buffer, 2000);
            munmap(&running, 4);
            wait(0);
            kill(getpid(), SIGKILL);
            return 0;
        }
        chdir(buffer);
        }
    }      
    return 0;
}
