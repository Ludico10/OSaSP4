#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define CHILDREN_COUNT 8
#define SIG_COUNT 101
#define FILE_NAME "PIDs"

typedef struct _child_t {
    int index;
    int *child;
    int children_count;
    struct sigaction act;
} child_t;

child_t children[CHILDREN_COUNT + 1];
struct sigaction terminateAct;
int usr1Count = 0;

int* readAllPID() 
{
        FILE* fp = fopen(FILE_NAME, "rb");
        if (fp == NULL)
        {
        	perror("Can not open PIDs file");
    	        exit(-1);
        }
        int* pids = (int*)malloc((CHILDREN_COUNT + 1) * sizeof(int));
        if (pids == NULL)
        {
        	perror("Can not allocate memory");
    	        exit(-1);
        }
    	fread(pids, sizeof(int), CHILDREN_COUNT + 1, fp);
        if (fclose(fp) == EOF)
        {
        	perror("Can not close PIDs file");
    		exit(-1);
        }
        return pids;
}

void writePID(int cur, pid_t newPID) 
{
	int *pids = readAllPID();
        pids[cur] = newPID;
    	FILE* fp = fopen(FILE_NAME, "w+b");
        if (fp == NULL)
        {
        	perror("Can not open PIDs file");
    	        exit(-1);
        }
        fwrite(pids, sizeof(int), CHILDREN_COUNT + 1, fp);
        if (fclose(fp) == EOF)
        {
        	perror("Can not close PIDs file");
    		exit(-1);
        }
    	free(pids);
}

pid_t readPID(int cur) 
{
	int *pids = readAllPID();
	pid_t pid = pids[cur];
	free(pids);
	return pid;
}

long long timer() 
{
	struct timeval tv;
	if (gettimeofday(&tv, NULL) == -1) 
	{
        	perror("Can not get time");
        	return -1;
        }
        return tv.tv_sec * 1000000 + tv.tv_usec;
}

void message(int who, int sig)
{
        printf("%d PID: %d PPID: %d received SIGUSR%s Time: %lld\n", who, getpid(), getppid(), strsignal(sig), timer());
        usr1Count++;
}

void terminateCall(int sig) 
{
	int* pids = readAllPID();
	pid_t pid = getpid();
	int i = 0;
    	while (i <= CHILDREN_COUNT && pid != pids[i])  i++;
    	free(pids);
	printf("%d PID: %d PPID: %d finished the work after %3ds %s\n", i, pid, getppid(), usr1Count, strsignal(sig));

    	for (int j = 0; j < children[i].children_count; j++) 
    	{
        	int kidsPid = readPID(children[i].child[j]);
        	kill(kidsPid, SIGTERM);
    	}
    	while (wait(0) > 0) {};
    	exit(0);
}

void child1Call(int sig) 
{
	message(1, sig);
	if (usr1Count == SIG_COUNT)
	{
        	if (kill(-readPID(5), SIGTERM))
        	{
        		perror("Can not sent a signal\n");
        		exit(-1);
        	}
        	int wpid;
        	while (wait(0) > 0) {};
        	printf("%d PID: %d PPID: %d finished the work after %3ds SIGUSR2\n",1, getpid(), getppid(), usr1Count);
        	exit(0);
	}
        if (kill(-readPID(5), SIGUSR1))
        {
        	perror("Can not sent a signal\n");
        	exit(-1);
        }
        printf("%d PID: %d PPID: %d sent SIGUSR1 Time: %lld\n", 1, getpid(), getppid(), timer());
}

void child2Call(int sig) 
{
    	message(2, sig);
    	if (kill(readPID(1), SIGUSR2))
        {
        	perror("Can not sent a signal\n");
        	exit(-1);
        }
        printf("%d PID: %d PPID: %d sent SIGUSR2 Time: %lld\n", 2, getpid(), getppid(), timer());
}

void child3Call(int sig) 
{
    	message(3, sig);
    	if (kill(readPID(2), SIGUSR2))
        {
        	perror("Can not sent a signal\n");
        	exit(-1);
        }
        printf("%d PID: %d PPID: %d sent SIGUSR2 Time: %lld\n", 3, getpid(), getppid(), timer());
}

void child4Call(int sig) {}

void child5Call(int sig) {
    	message(5, sig);
    	if (kill(readPID(3), SIGUSR1))
        {
        	perror("Can not sent a signal\n");
        	exit(-1);
        }
        printf("%d PID: %d PPID: %d sent SIGUSR1 Time: %lld\n", 5, getpid(), getppid(), timer());
}

void child6Call(int sig) {
    	message(6, sig);
    	if (kill(readPID(3), SIGUSR1))
        {
        	perror("Can not sent a signal\n");
        	exit(-1);
        }
        printf("%d PID: %d PPID: %d sent SIGUSR1 Time: %lld\n", 6, getpid(), getppid(), timer());
}

void child7Call(int sig) {
    	message(7, sig);
    	if (kill(readPID(3), SIGUSR2))
        {
        	perror("Can not sent a signal\n");
        	exit(-1);
        }
        printf("%d PID: %d PPID: %d sent SIGUSR2 Time: %lld\n", 7, getpid(), getppid(), timer());
}

void child8Call(int sig) {
    	message(8, sig);
    	if (kill(readPID(3), SIGUSR1))
        {
        	perror("Can not sent a signal\n");
        	exit(-1);
        }
        printf("%d PID: %d PPID: %d sent SIGUSR1 Time: %lld\n", 8, getpid(), getppid(), timer());
}

void ProcTree(child_t *node) 
{
	for (int i = 0; i <= CHILDREN_COUNT; i++) 
    	{
        	memset(&node[i], 0, sizeof(node[i]));
        	node[i].index = i;
    	}

    	node[0].children_count = 1;
    	node[0].child = (int*)malloc(sizeof(int) * node[0].children_count);
    	if (node[0].child == NULL)
    	{
    		perror("Can not allocate memory");
    		exit(-1);
    	}
    	node[0].child[0] = 1;
    
    	node[1].children_count = 1;
    	node[1].child = (int*)malloc(sizeof(int) * node[0].children_count);
    	if (node[1].child == NULL)
    	{
    		perror("Can not allocate memory");
    		exit(-1);
    	}
    	node[1].child[0] = 2;
    	node[1].act.sa_handler = child1Call;
    
    	node[2].children_count = 1;
    	node[2].child = (int*)malloc(sizeof(int) * node[0].children_count);
    	if (node[2].child == NULL)
    	{
    		perror("Can not allocate memory");
    		exit(-1);
    	}
    	node[2].child[0] = 3;
    	node[2].act.sa_handler = child2Call;

    	node[3].children_count = 3;
    	node[3].child = (int*)malloc(sizeof(int) * node[1].children_count);
    	if (node[3].child == NULL)
    	{
    		perror("Can not allocate memory");
    		exit(-1);
    	}
    	node[3].child[0] = 4;
    	node[3].child[1] = 5;
	node[3].child[2] = 6;
    	node[3].act.sa_handler = child3Call;
    	
    	node[4].children_count = 1;
    	node[4].child = (int*)malloc(sizeof(int) * node[0].children_count);
    	if (node[4].child == NULL)
    	{
    		perror("Can not allocate memory");
    		exit(-1);
    	}
    	node[4].child[0] = 8;
    	node[4].act.sa_handler = child4Call;

    	node[5].children_count = 0;
    	node[5].act.sa_handler = child5Call;
    	
    	node[6].children_count = 1;
    	node[6].child = (int*)malloc(sizeof(int) * node[0].children_count);
    	if (node[6].child == NULL)
    	{
    		perror("Can not allocate memory");
    		exit(-1);
    	}
    	node[6].child[0] = 7;
    	node[6].act.sa_handler = child6Call;

    	node[7].children_count = 0;
    	node[7].act.sa_handler = child7Call;
    	
    	node[8].children_count = 0;
    	node[8].act.sa_handler = child8Call;
}

void createSignals(child_t node) 
{
	if (node.index == 5)  setpgid(0, getpid());
    	if (node.index > 5)  setpgid(0, readPID(2));
    	printf("PID: %d PPID: %d GPID: %d Time: %lld CHILD%d\n", getpid(), getppid(), getpgrp(), timer(), node.index);

    	if (node.index == 3 || node.index > 4)
        	if (sigaction(SIGUSR1, &node.act, 0) == -1)
        	{
        		perror("Can not change the action for child\n");
        		exit(-1);
        	}
        if (node.index < 4)
        	if (sigaction(SIGUSR2, &node.act, 0) == -1)
        	{
        		perror("Can not change the action for child\n");
        		exit(-1);
        	}

    	if (node.index > 1)
        	if (sigaction(SIGTERM, &terminateAct, 0) == -1)
        	{
        		perror("Can not change the action for child\n");
        		exit(-1);
        	}
    	writePID(node.index, getpid());

    	for (int i = 0; i < node.children_count; i++) 
    	{
        	while (readPID(node.child[i] - 1) == 0) {}
        	pid_t child = fork();
        	switch(child) 
        	{
            	case 0:
                	createSignals(children[node.child[i]]);
                	while (1) {}
                	break;
            	case -1:
        		perror("Can not create a child\n");
        		exit(-1);
                	break;

            	default:
                	if (node.index == 1 && i + 1 == node.children_count) 
                	{
                    		for (int i = 0; i <= CHILDREN_COUNT; i++) 
                    		{
                       	 	while (readPID(i) == 0) {}
                    		}

                    		if (kill(-readPID(5), SIGUSR1) == -1)
                    		{
                            		perror("Can not send signal\n");
        				exit(-1);
        	     		}
                    		while (1) {}
                	}
                	break;
        	}
    }
    return;
}

int main(void) 
{
	FILE* fp = fopen(FILE_NAME, "w+b");
        if (fp == NULL)
        {
        	perror("Can not open PIDs file");
    	        return -1;
        }
	int initial = 0;
	for (int i = 0; i <= CHILDREN_COUNT; i++)
        	fwrite(&initial, sizeof(int), 1, fp);
        if (fclose(fp) == EOF)
        {
        	perror("Can not close PIDs file");
    		return -1;
        }
        
    	ProcTree(children);
    	memset(&terminateAct, 0, sizeof(terminateAct));
    	terminateAct.sa_handler = terminateCall;

    	createSignals(children[0]);
    	while (readPID(1) == 0) {}
    	if (waitpid(readPID(1), NULL, 0) == -1)
    	{
    		perror("Waiting error");
    		return -1;
    	}
    	printf("%d PID: %d PPID: %d finished the work after %3ds SIGUSR1\n",0, getpid(), getppid(), usr1Count);
    	return 0;
}
