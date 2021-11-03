#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <libgen.h>

void overridesig(int n)
{
    printf("\n no \n");
}

int main()
{
    printf("PID before the fork: %d", getpid());
    fflush(0);
    signal(SIGINT, overridesig);
   // signal(SIGTSTP, overridesig);
    signal(SIGKILL, overridesig);
    signal(SIGTERM, overridesig);
   /// signal(SIGSTOP, overridesig);
    
    time_t T = time(NULL);
    struct tm tm = *localtime(&T);
    while(1)
    {
        if (fork() == 0)
        {
            while(1)
            {
                printf("\n.......\n");
                printf("\nKid PID: %d\n", getpid());
                printf("\nCurrent Time: %d:%d\n", tm.tm_hour, tm.tm_min);
                DIR *d = opendir(".");
                struct dirent *dp;
                printf("\n");
                while ((dp = readdir(d)) != NULL)
                {
                    //write(STDOUT_FILENO, readdir(d), 1000);
                    printf("%s ", dp->d_name);
                    if (dp->d_type == DT_DIR)
                    {
                        printf(" - is a Directory");
                    }
                    printf("\n");
                }
                printf("\n");
                printf("\n.......\n");
                closedir(d);
                sleep(10);
            }
            return 0;
        }
        else
        {
            printf("\nParent PID: %d\n", getpid());
            wait(NULL);
        }

    }
    return 0;
}
