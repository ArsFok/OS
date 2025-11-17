#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void exitHandler(){
    printf("Process PID=%d: Executing atexit() handler\n", getpid());
}
void sigintHandler(int sig){
     printf("Process PID=%d: Received SIGINT signal (%d)\n", getpid(), sig);
     exit(0);
}
void sigtermHandler(int sig, siginfo_t *info, void *context){
    printf("Process PID=%d: Received SIGTERM signal (%d)\n", getpid(), sig);
    printf("Signal sent by process PID=%d\n", info->si_pid);
    exit(0);
}
int main(int argc, char** argv){
    pid_t pid;
    int status;

    if(atexit(exitHandler) != 0){
        perror("atexit");
        exit(1);
    }
    if(signal(SIGINT, sigintHandler) == SIG_ERR){
        perror("signal");
        exit(1);
    }
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = sigtermHandler;
    sa.sa_flags = SA_SIGINFO;

    if(sigaction(SIGTERM, &sa, NULL) == -1){
        perror("sigaction");
        exit(1);
    }

    printf("=== Program started ===\n");
    printf("Parent process: PID=%d, PPID=%d\n", getpid(), getppid());

    pid = fork();

    if(pid == -1){
        perror("fork");
        exit(1);
    }
    else if(pid == 0){
        printf("\n--- Child process ---\n");
        printf("Child process: PID=%d, PPID=%d\n", getpid(), getppid());

        for(int i = 0; i < 3; i++){
            printf("Child process: working %d/3\n", i + 1);
            sleep(1);
        }
        printf("Child process finishing\n");
        exit(42);
    }else{
        printf("\n--- Parent process ---\n");
        printf("Parent process: PID=%d, PPID=%d\n", getpid(), getppid());
        printf("Created child process with PID=%d\n", pid);

        for (int i = 0; i < 2; i++) {
            printf("Parent process: working %d/2\n", i + 1);
            sleep(1);
        }

        printf("\nParent process waiting for child to finish...\n");
        pid_t terminated_pid = wait(&status);
        
        if (terminated_pid == -1) {
            perror("wait");
        }

        else {
            if (WIFEXITED(status)) {
                printf("Child process PID=%d exited normally with code: %d\n", 
                       terminated_pid, WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status)) {
                printf("Child process PID=%d terminated by signal: %d\n", 
                       terminated_pid, WTERMSIG(status));
            }
            else if (WIFSTOPPED(status)) {
                printf("Child process PID=%d stopped by signal: %d\n", 
                       terminated_pid, WSTOPSIG(status));
            }
        }

        printf("\nParent process finishing work\n");
        printf("You can press Ctrl+C to demonstrate SIGINT handling\n");
        printf("Or send SIGTERM from another terminal: kill %d\n", getpid());
        sleep(100);
    }
    return 0;
}