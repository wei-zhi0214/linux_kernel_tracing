#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
	int i;
	for(i = 0; i<2; i++){
		fork();
		printf("_\n");
	}
	wait(NULL);
	wait(NULL);
	return 0;
}
