/*
 * fork.c
 *
 * simple fork program to help you test fork() and copy-on-write 
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <err.h>

static volatile int dummy = 0;

static
int
spin(int n)
{
        int i;
        
        for (i = 0; i < n; ++i) {
	        dummy += i;
	}
	
	return 0;
}

static
int
dofork(void)
{
	int pid;
	
	pid=fork();
	printf("[%d] FORKING pid addr %p \n", getpid(), &pid);

	if (pid < 0) {
		warn("fork");
	}
	return pid;
}

int
main(int argc, const char * argv[])
{
        int i;
	int n = 1;
	
	if (argc == 2) {
	        n = atoi(argv[1]);
	}
	int cpid=0;
	for (i = 0; i < n; ++i) {
		printf("[%d] START FORKING\n", getpid());
		if ((cpid=dofork()) == 0){
			printf("[%d] DONE FORKING CHILD\n", getpid());
		        
		        spin(500);
		        _exit(69);
		}
	}
	printf("[%d] DONE FORKING PARENT\n", getpid());

	int exitcode;
	cpid=waitpid(cpid, &exitcode,0);
	printf("[%d] DONE WAIT PARENT %d %d\n", getpid(),cpid,exitcode);
	/* spin for a while and exit */
	return spin(n*1000);
}
