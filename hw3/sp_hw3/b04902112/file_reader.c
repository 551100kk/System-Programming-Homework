/* b04902112 張凱捷 */
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
#include <sys/mman.h>
char in[1024];
char name[1024];
char str[1024];
void die(){
	char *p_map;
	int mfd = open("die_time", O_RDWR | O_TRUNC | O_CREAT, 0777);
	lseek(mfd, 100, SEEK_SET);
    write(mfd, "", 1);
    p_map = (char*) mmap(0, 100, PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0);
    close(mfd);

    char content[100] = {};
    time_t current_time = time(NULL);
    strcat(content, "Last Exit CGI: ");
    strcat(content, ctime(&current_time));
    content[strlen(content) - 1] = 0;
    strcat(content, ", Filename: ");
    strcat(content, name);
    strcat(content, "\n");

    memcpy(p_map, &content , sizeof(content));
}
int main(){
	read(0, in, sizeof(in));
	sscanf(in, "%s", name);
	int fd;
	fd = open(name, O_RDONLY);
	if(fd < 0) {
		perror("You boosted bonobo!");
		die();
		return 1;
	}
	int len;
	while((len = read(fd, str, sizeof(str))) > 0) {
		write(1, str, len);
	}
	die();
	return 0;
}
