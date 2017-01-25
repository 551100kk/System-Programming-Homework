/*B04902112 張凱捷*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <wait.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

struct P{
	int x, y;
};
typedef struct P P;
int cmp(const void *aaa, const void *bbb) {
	P a = *(P*)aaa;
    P b = *(P*)bbb;
	if(a.y != b.y) return a.y < b.y;
	return a.x > b.x;
}

int req[5000][4];
int main(int argc, char **argv){
	int player;
	int judge;
	sscanf(argv[1], "%d", &judge);
	sscanf(argv[2], "%d", &player);
	int cnt = 0;
	for (int a = 1; a <= player; a++)
	for (int b = a+1; b <= player; b++)
	for (int c = b+1; c <= player; c++)
	for (int d = c+1; d <= player; d++){
		if(a==b || a==c || a==d || b==c || b==d || c==d) continue;
		req[cnt][0] = a;
		req[cnt][1] = b;
		req[cnt][2] = c;
		req[cnt][3] = d;
		cnt++;
	}
	/*for(int i=0;i<cnt;i++){
		for(int j=0;j<4;j++) printf("%d ",req[i][j]);
			puts("");
	}*/
	
	
	int pipe_out[15][2];
	int pipe_in[15][2];
	/*int fd[2];
	int rr = pipe(fd);
	int tt = write(fd[1], "123\n", 4);
	printf("%d %d\n", rr, tt);
	return 0;*/
	int pid[15];

	for (int i = 1; i <= judge; i++) {
		pipe(pipe_out[i]);
		pipe(pipe_in[i]);
		pid[i] = fork();
		if (pid[i] == 0) {
			char str[10] = {0};
			sprintf(str, "%d", i);
			dup2(pipe_out[i][0], 0);
			dup2(pipe_in[i][1], 1);
			close(pipe_out[i][1]);
			close(pipe_in[i][0]);
			for (int j = 1; j < i; j++) {
				close(pipe_out[j][0]);
				close(pipe_out[j][1]);
				close(pipe_in[j][0]);
				close(pipe_in[j][1]);
			}
			if ( execl("./judge", "judge", str, (char*)0) < 0 ) {
				write(2, "exec_err\n",9);
			}
		}
		close(pipe_out[i][0]);
		close(pipe_in[i][1]);
	}

	//int tt = write(pipe_out[1][1], "123\n", 4);
	//printf("%d\n", tt);

	int idle[15] = {0};
	int score[25] = {0};
	int avai = judge;

	for (int i = 0; i <= cnt; i++) {
		while (avai == 0 || i == cnt && avai < judge) {
			fd_set readset;
        	struct timeval timeout;
        	timeout.tv_sec = 1;
        	FD_ZERO(&readset);
        	for (int j = 1; j <=judge; j++) {
        		FD_SET(pipe_in[j][0], &readset);	
        	}
        	if (select(FD_SETSIZE, &readset, NULL, NULL, &timeout) < 0) {
        		write(2, "select_err\n", 11);
        	}
        	for (int j = 1; j <= judge; j++) {
        		if (FD_ISSET(pipe_in[j][0], &readset) == 0) continue;
        		avai ++;
        		idle[j] = 0;
        		char str[105] = {0};
        		read(pipe_in[j][0], str, 100);
        		//printf("judge %d:\n%s==\n", j, str);
        		for (int k = 0; str[k]; k++) {
        			if (k == 0 || str[k - 1] == '\n') {
        				int x, y;
        				sscanf(&str[k], "%d%d", &x, &y);
        				score[x] += 4 - y;
        			}
        		}
        	}
		}
		if (i == cnt) break;
		int id = -1;
		for (int j = 1; j <= judge; j++) {
			if(idle[j] == 1) continue;
			id = j;
			//break;
		}
		if(id == -1) {
			write(2, "id_err\n", 7);
		}
		char str[100] = {0};
		sprintf(str, "%d %d %d %d\n", req[i][0], req[i][1], req[i][2], req[i][3]);
		if( write(pipe_out[id][1], str, strlen(str)) < 0) {
			write(2, "pi_err\n", 7);
		}  
		idle[id] = 1;
		avai -- ;
	}
	for (int i = 1; i <= judge; i++) {
		char str[20] = {0};
		sprintf(str, "%d %d %d %d\n", -1, -1, -1, -1);
		//puts(str);
		//sleep(1);
		if( write(pipe_out[i][1], str, strlen(str)) < 0) {
			write(2, "pi_err\n", 7);	
		}
			
	}
	for (int i = 1; i <= judge; i++) {
		int sta;
		close(pipe_out[i][1]);
		close(pipe_in[i][0]);
		waitpid(pid[i], &sta, 0);
	}
	P arr[25];
	for (int i = 0; i < player; i++) {
		 arr[i].x = i + 1;
		 arr[i].y = score[i + 1];
	}
	qsort(arr, player, sizeof(arr[0]), cmp);
	for(int i = 0; i < player; i++) {
		printf("%d %d\n", arr[i].x, arr[i].y);
	}

}
