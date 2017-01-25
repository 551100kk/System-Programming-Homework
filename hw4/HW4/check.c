#include <stdio.h>
int main() {
	FILE *f1,*f2;
	f1=fopen("submission.csv","r");
	f2=fopen("ans.csv","r");
	char s[1000];
	fscanf(f1,"%s",s);
	fscanf(f2,"%s",s);
	int cnt=0;
	for(int i=0;i<25008;i++){
		int z,x,y;
		fscanf(f1,"%d,%d",&z,&x);
		fscanf(f2,"%d,%d",&z,&y);
		if(x==y) cnt++;
	}
	printf("%f\n", 1.0*cnt/25008);
}