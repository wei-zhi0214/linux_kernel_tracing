// memhog.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define MAXSIZE 35
int main(int argc, char **argv) {
	int i,j,fd,size,len;
	char *buf="Hello!I`m writing to this file!";
	char buf_r[MAXSIZE];
	len=strlen(buf);
	//open
	if((fd=open("/tmp/hello.c",O_CREAT | O_TRUNC | O_RDWR,0666))<0) {
		perror("open:");
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	}
	else
		printf("Open file:hello.c %d\n",fd);
	//write
	if((size=write(fd,buf,len))<0){
		perror("write:");
		exit(1);
	}
	else
		printf("Write:%s\n\n\n",buf);
	//test-read
	printf("Now test starts...\n\n");
	for(i=0;i<20;i++){
		lseek(fd,0,SEEK_SET);
		for(j=0;j<MAXSIZE;j++)
			buf_r[j]=0;
		if((size=read(fd,buf_r,MAXSIZE-i))<0){
			perror("read:");
			exit(1);
		}
		else {
			buf_r[MAXSIZE-i]='\0';
			printf("string-len=%d,count=%d,size=%d\n",len,MAXSIZE-i,size);
			printf("read from file:%s \n",buf_r);
		}
	}
	printf("\n\nNow test stops...\n");
	//close
	if(close(fd)<0){
		perror("close:");
		exit(1);
	}
	else
		printf("close hello.c\n");
	exit(0);
}

