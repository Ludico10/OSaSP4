#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

pid_t children[2];
int msgCount = 0;

int timer()
{
        struct timeval time;
        if (gettimeofday(&time, NULL) == -1)
        {
        	perror("Cannot get time ");
            	return -1;
        };
        int msec = time.tv_sec * 1000 + time.tv_usec / 1000;
        return msec;
}

void parentCall(int sig, siginfo_t *siginfo, void *context) 
{
        printf("%3d PID: %6d PPID: %6d Time: %d PARENT gets SIGUSR%s from %d\n", msgCount, getpid(), getppid(), timer(), strsignal(sig), siginfo->si_pid);
        usleep(100 * 1000);
        if (kill(0, SIGUSR1) == -1) 
        {
        	perror("Can not send signal\n");
        	exit(-1);
    	}
    	else
        	printf("%3d PID: %6d PPID: %6d Time: %d PARENT puts User defined signal 1\n", msgCount++, getpid(), getppid(), timer());
}

void childCall(int sig, siginfo_t *siginfo, void *context) 
{
	pid_t pid = getpid();
        int num;
	if (pid == children[0]) num = 1;
	else num = 2;
	pid_t ppid = getppid();
    	printf("%3d PID: %6d PPID: %6d Time: %d CHILD%d gets SIGUSR%s\n", msgCount, pid, ppid, timer(), num, strsignal(sig));
        if (kill(ppid, SIGUSR2) == -1) 
        {
        	perror("Can not send signal\n");
        	exit(-1);
    	}
    	else
    	        printf("%3d PID: %6d PPID: %6d Time: %d CHILD%d puts User defined signal 2\n", msgCount++, pid, ppid, timer(), num);
}

int main(void)
{
	sigset_t usr1, usr2;
	if (sigemptyset(&usr1) == -1)
	{
		perror("Can not clear usr1 ");
		return -1;
	}
	if (sigemptyset(&usr2) == -1)
	{
		perror("Can not clear usr2 ");
		return -1;
	}
	if (sigaddset(&usr1, SIGUSR1) == -1)
	{
		perror("Can not add SIGUSR1 to usr1 ");
		return -1;
	}
	if (sigaddset(&usr2, SIGUSR2) == -1)
	{
		perror("Can not add SIGUSR2 to usr2 ");
		return -1;
	}
	
	struct sigaction parentAct, childAct;
    	memset(&parentAct, 0, sizeof(parentAct));
    	parentAct.sa_sigaction = parentCall;
    	parentAct.sa_flags = SA_SIGINFO;
    	memset(&childAct, 0, sizeof(childAct));
    	childAct.sa_sigaction = childCall;
    	childAct.sa_flags = SA_SIGINFO;

    	for (int i = 0; i < 2; i++) 
    	{
        	children[i] = fork();
        	switch (children[i]) 
        	{
        	case -1:
                	perror("Can not create a child proc\n");
                	break;
            	case 0:
                	children[i] = getpid();
                	if (sigprocmask(SIG_SETMASK, &usr2, 0) == -1) 
                	{
                    		perror("Can not change mask for child\n");
                    		return -1;
                	}
                	if (sigaction(SIGUSR1, &childAct, NULL) == -1) 
                	{
                    		perror("Can not change action for child\n");
                    		return -1;
                	}
                	printf("PID: %d PPID: %d Time: %d CHILD%d\n", getpid(), getppid(), timer(), i + 1);
                	while (1) {}
               }
	}
	printf("PID: %d PPID: %d Time: %d PARENT\n", getpid(), getppid(), timer());
        if (sigprocmask(SIG_SETMASK, &usr1, 0) == -1) 
        {
        	perror("Can not change mask for parent\n");
        	return -1;
    	}
    	if (sigaction(SIGUSR2, &parentAct, NULL) == -1) 
    	{
        	perror("Can not change action for parent\n");
        	return -1;
        }

        sleep(1);
        if (kill(0, SIGUSR1) == -1) 
        {
        	perror("Can not send signal\n");
        	return -1;
    	}
    	else
        	printf("%3d PID: %6d PPID: %6d Time: %d PARENT puts User defined signal 1\n", msgCount, getpid(), getppid(), timer());
        while (1) {}
	return 0;
}
