/*B04902112 張凱捷*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <dirent.h>
#include <time.h>

int main(int agrc,char **argv){
	//printf("judge %s, player %s, hash %s\n", argv[1], argv[2],argv[3]);
	char fifo_out[100];
	sprintf(fifo_out, "judge%s.FIFO", argv[1]);
	char fifo_in[100];
	sprintf(fifo_in, "judge%s_%s.FIFO", argv[1], argv[2]);

	//printf("%s %s\n", fifo_in, fifo_out);

	int fd1 = open(fifo_out, O_WRONLY);
	int fd2 = open(fifo_in, O_RDONLY);

	int can[3] = {1,3,5};
	int val = 0;
	/*if (argv[2][0] == 'A') val = 1,sleep(10);
	if (argv[2][0] == 'B') val = 3;
	if (argv[2][0] == 'C') val = 5;
	if (argv[2][0] == 'D') val = 3;*/
	srand(time(0)*getpid()*(int)argv[2][0]);
	for(int i = 0; i < 4; i++) {
		val = rand() % 3;
	}
	val = can[val];
	
	char str[100] = {0};
	sprintf(str, "%s %s %d\n", argv[2], argv[3], val);
	write(fd1, str, strlen(str));

	for (int T = 1; T < 20; T++) {
		memset(str, 0, sizeof(str));
		read(fd2, str, 20);
		int a, b, c, d;
		sscanf(str, "%d%d%d%d", &a, &b, &c, &d);		
		//printf("%s --- %d %d %d %d\n",argv[2], a, b, c, d);
		for(int i = 0; i < 4; i++) {
			val = rand() % 3;
		}
		val = can[val];
		sprintf(str, "%s %s %d\n", argv[2], argv[3], val);
		write(fd1, str, strlen(str));

		
	}

	close(fd1);
	close(fd2);
}
