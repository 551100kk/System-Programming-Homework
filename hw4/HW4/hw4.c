/*b04902112	張凱捷*/
#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define filename_len 	1024
#define trainnum		25150
#define testnum			25008
#define featurenum		33
#define maxtree			2048
#define maxthread		2048
#define treesize		33
#define eps				1e-8

struct  people{
	int id;
	double feature[featurenum];
	int label;
};

struct node{
	int feature, label;
	double threshold;
	struct node *left, *right;
};

struct  argument{
	int start;
	int len;
};

struct people	data_train[trainnum + 5];
struct people	data_test[testnum + 5];
struct argument thread_in[maxthread];
struct node		root[maxtree];

char	train[filename_len];
char	test[filename_len];
char	output[filename_len];
int		thread_cnt;
int		tree_cnt;

int cmp (const void *aa , const void *bb, void* idd) {
	struct  people *a = (struct  people *)aa;
	struct  people *b = (struct  people *)bb;
	int id = *(int *)idd;
	if (a->feature[id] < b->feature[id]) return -1;
	if (a->feature[id] == b->feature[id]) return 0;
	if (a->feature[id] > b->feature[id]) return 1;
}

void build (struct node *pt, int size, struct people data[]) {
	// check end
	int flag0 = 0;
	int flag1 = 0;
	for (int i = 0; i < size; i++) {
		if (data[i].label == 0) flag0 = 1;
		if (data[i].label == 1) flag1 = 1;
	}
	if (flag0 + flag1 <= 1) {
		if (flag0) pt->label = 0;
		else pt->label = 1;
		return ;
	}
	// cut tree
	int cut;
	double min = 1e100;
	
	for (int i = 0; i < featurenum; i++) {
		qsort_r(data, size, sizeof(struct people), cmp, &i);
		
		int total = 0;

		for (int j = 0; j < size; j++) {
			if (data[j].label) total++;
		}

		int cnt = 0;

		for (int j = 0; j < size; j++) {
			if (data[j].label) cnt++;
			double gin1 = (double) cnt / (j + 1);
			double gin2 = (double) (total - cnt) / (size - j - 1);
			double val = gin1 * (1.0 - gin1) + gin2 * (1.0 - gin2);
			if (val < min) {
				cut = j;
				min = val;
				pt->feature = i;
				pt->threshold = data[j].feature[i];
			}
		}
	}
	//printf("%d, %d..  %f\n", cut, size, min);

	qsort_r(data, size, sizeof(struct people), cmp, &pt->feature);
	pt->label = -1;
	pt->left = (struct node *) malloc(sizeof(struct node));
	pt->right = (struct node *) malloc(sizeof(struct node));
	build(pt->left, cut + 1, data);
	build(pt->right, size - cut - 1, data + cut + 1);
}

void *thread (void *arg) {	
	struct argument *tmp = (struct argument*) arg;
	int l = tmp->start;
	int r = l + tmp->len;
	int seed = pthread_self();

	for (int i = l; i < r; i++) {
		// random get data
		struct people data[treesize + 5];
		for (int j = 0; j < treesize; j++) {
			int id = rand_r(&seed) % trainnum;
			data[j] = data_train[id];
		}
		// build tree
		build(&root[i], treesize, data);
	}

	pthread_exit(NULL);
}

int solve (struct node* pt, struct people input) {
	if (pt->label != -1) return pt->label;
	int feature = pt->feature;
	double threshold = pt->threshold;
	//printf("%d, %f, %d\n", feature, threshold, pt->label);
	if (input.feature[feature] < threshold + eps) {
		return solve(pt->left, input);
	} 
	else {
		return solve(pt->right, input);
	}
}

int main (int argc,char **argv) {
	// read arg
	sprintf(train, "%s/training_data", argv[2]);
	sprintf(test, "%s/testing_data", argv[2]);
	sprintf(output, "%s", argv[4]);
	sscanf(argv[6], "%d", &tree_cnt);
	sscanf(argv[8], "%d", &thread_cnt);

	// read training data
	FILE *fp;
	fp = fopen(train, "r");
	for(int i = 0; i < trainnum; i++) {
		fscanf(fp, "%d", &data_train[i].id);
		for(int j = 0; j < featurenum; j++) {
			fscanf(fp, "%lf", &data_train[i].feature[j]);
		}
		fscanf(fp, "%d", &data_train[i].label);
	}
	fclose(fp);

	// read testing data
	fp = fopen(test, "r");
	for(int i = 0; i < testnum; i++) {
		fscanf(fp, "%d", &data_test[i].id);
		for(int j = 0; j < featurenum; j++) {
			fscanf(fp, "%lf", &data_test[i].feature[j]);
		}
	}
	fclose(fp);

	// build random forest
	int 		avg = tree_cnt / thread_cnt;
	int 		now_cnt = 0;
	pthread_t 	pthid[maxthread];
	
	for(int i = 0; i < thread_cnt; i++) {
		int total = avg;
		if(i < tree_cnt - thread_cnt * avg) avg += 1;
		thread_in[i].start	= now_cnt;
		thread_in[i].len 	= total;
		now_cnt += total;
		pthread_create(&pthid[i], NULL, thread, &thread_in[i]);
	}

	for (int i = 0; i < thread_cnt; i++) {
		pthread_join(pthid[i], NULL);	
	}

	// test
	fp = fopen(output, "w");
	fprintf(fp, "id,label\n");
	for (int i = 0; i < testnum; i++) {
		int cnt0 = 0;
		int cnt1 = 0;
		for (int j = 0; j < tree_cnt; j++) {
			int tmp = solve(&root[j], data_test[i]); 
			if (tmp == 1) cnt1 ++;
			else if (tmp == 0) cnt0 ++;
			else puts("??");
		}
		//if(cnt0) puts("*");
		if(cnt0 < cnt1) fprintf(fp, "%d,%d\n", data_test[i].id, 1);
		else fprintf(fp, "%d,%d\n", data_test[i].id, 0);
	}
	fclose(fp);
	
	return 0;
}