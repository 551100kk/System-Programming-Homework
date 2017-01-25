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

int main(int agrc,char **argv){
  	int judge;
	sscanf(argv[1], "%d", &judge);
	//printf("%d\n", judge);
	int		player_id[4];
	while (1) {
		fd_set readset;
    	struct timeval timeout;
    	timeout.tv_sec = 5;
    	timeout.tv_usec = 0;
    	FD_ZERO(&readset);
    	FD_SET(0, &readset);        	
    	if (select(FD_SETSIZE, &readset, NULL, NULL, &timeout) < 0) {
    		write(2, "select_err\n", 11);
    		break;
    	}
    	//printf("%d\n",  clock());
    	if (FD_ISSET(0, &readset) == 0) continue;
		char strr[105] = {0};
		read(0, strr, 100);
		sscanf(strr, "%d%d%d%d", &player_id[0], &player_id[1], &player_id[2], &player_id[3]);

		if(player_id[0] == -1) break;
		int 	fd[5];
		char filename[5][100];
		sprintf(filename[0], "judge%d.FIFO", judge);
		mkfifo(filename[0], 0600);
		sprintf(filename[1], "judge%d_A.FIFO", judge);
		mkfifo(filename[1], 0600);
		sprintf(filename[2], "judge%d_B.FIFO", judge);
		mkfifo(filename[2], 0600);
		sprintf(filename[3], "judge%d_C.FIFO", judge);
		mkfifo(filename[3], 0600);
		sprintf(filename[4], "judge%d_D.FIFO", judge);
		mkfifo(filename[4], 0600);

		
		char	player_ch[4][2]={0};
		int		hash_val[4];
		srand(7122);
		
		
		for(int i = 0; i < 4; i++) {
			int flag = 1;
			while(flag) {
				flag = 0;
				hash_val[i] = rand() % 65536;
				for(int j = 0; j < i; j ++) {
					if(hash_val[i] == hash_val[j]) flag = 1;
				}
			}
			player_ch[i][0] = i + 'A';
		}

		/*for(int i = 0; i < 4; i++) {
			printf("%d %c %d\n", player_id[i], player_ch[i][0], hash_val[i]);
		}*/

		int		pid[4];	 
		
		for(int i = 0; i < 4; i++) {
			pid[i] = fork();
			if(pid[i] == 0){
				char	hash[10];
				sprintf(hash, "%d", hash_val[i]);

				if( execl("./player", "player", argv[1], player_ch[i], hash, (char*)0) < 0 ) {
					write(2, "exec_err\n", 9);	
				}
				exit(0);
			}
		}

		
		fd[0] = open(filename[0], O_RDONLY);
		for(int i = 0; i < 4; i++) {
			fd[i + 1] = open(filename[i + 1], O_WRONLY);
		}

		int 	score[4] = {0};
		int 	die[4] = {0};

		for(int T = 0; T < 20; T++) {
			int st_time = clock();
			int cnt = 0;
			for (int i = 0; i < 4; i++) {
				if (die[i] == 0) {
					cnt++;
				}
			}
			int 	is[4] = {0};
			int 	cnt_val[10] = {0};
			int		cli_val[4] = {0};
			while(clock() - st_time < 3000000 && cnt){
				fd_set readset;
	        	struct timeval timeout;
	        	timeout.tv_sec = 0;
	        	timeout.tv_usec = 0;
	        	FD_ZERO(&readset);
	        	FD_SET(fd[0], &readset);        	
	        	if (select(FD_SETSIZE, &readset, NULL, NULL, &timeout) < 0) {
	        		write(2, "select_err\n", 11);
	        	}
	        	//printf("%d\n",  clock());
	        	if (FD_ISSET(fd[0], &readset) == 0) continue;
	        	char str[105]={0};
	        	read(fd[0], str, 100);
	        	for (int i = 0; str[i]; i++) {
	        		if (i == 0 || str[i - 1] == '\n') {
	        			char ch[3];
	        			int hash;
	        			int val;
	        			sscanf(&str[i], "%s%d%d", ch, &hash, &val);
	        			int id = ch[0] - 'A';
	        			if (hash_val[id] != hash) continue;
	        			if (die[id] == 0 && is[id] == 0) {
	        				is[id] = 1;
	        				cnt--;
	        				cnt_val[val] ++;
	        				cli_val[id] = val;
	        			}
	        		}
	        	}
			}
			for (int i = 0; i < 4; i++) {
				if (is[i] == 0) {
					die[i] = 1;
				}
				else {
					//score[i]+=4-i;
					if(cnt_val[cli_val[i]] == 1) {
						score[i] += cli_val[i];
					}
				}
			}
			char str[20];
			sprintf(str, "%d %d %d %d\n", cli_val[0], cli_val[1], cli_val[2], cli_val[3]);
			for (int i = 0; i < 4; i++) {
				if(T < 19) write(fd[i + 1], str, strlen(str));
			}
		}
		//score
		P arr[5];
		for (int i = 0; i < 4; i++) {
			 arr[i].x = player_id[i];
			 arr[i].y = score[i];
		}
		qsort(arr, 4, sizeof(arr[0]), cmp);
		int pre = -1, rank;
		int real[25];
		for(int i = 3; i >= 0; i--) {
			int y = arr[i].y;
			if(arr[i].y == pre) arr[i].y = rank;
			else arr[i].y = i + 1, rank = i + 1;
			pre = y;
		}
		for (int i = 0; i < 4; i++) real[arr[i].x] = arr[i].y;
		for (int i = 0; i < 4; i++) arr[i].x = player_id[i], arr[i].y = real[player_id[i]];
		char str[100] = {0};
		sprintf(str, "%d %d\n%d %d\n%d %d\n%d %d\n", arr[0].x, arr[0].y, arr[1].x, arr[1].y, arr[2].x, arr[2].y, arr[3].x, arr[3].y);
		write(1, str, strlen(str));

		close(fd[0]);
		for(int i = 0; i < 4; i++) {
			close(fd[i + 1]);
		}
		//close(0);
		for(int i = 0; i < 4; i++) {
			kill(pid[i],SIGKILL);
			int sta;
			waitpid(pid[i], &sta, 0);
		}	
	}
	
}
