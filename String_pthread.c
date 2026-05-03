// gcc String_pthread.c -o substring_search -lpthread
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUM_THREADS	4
#define MAX 1024

void *sub_string(void *);
int readf(FILE *fp);
int total=0;
int nlocal,n1,n2;
char *s1,*s2;
FILE *fp;
pthread_mutex_t total_lock;

int main(int argc, char *argv[])
{
	int i,rc;
	pthread_t threads[NUM_THREADS];

	pthread_mutex_init(&total_lock,NULL);
	readf(fp);
	for(i=0;i<NUM_THREADS;i++){
		rc = pthread_create(&threads[i], NULL, sub_string, (void *)(long)i);
		if (rc){
			printf("ERROR: return error from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	for(i=0; i<NUM_THREADS; i++){
		rc = pthread_join(threads[i], NULL);
		if (rc){
			printf("ERROR: return error from pthread_join() is %d\n", rc);
			exit(-1);
		}
	}
	printf("the occurences of s2 in s1 is %d\n",total);
	
	// Clean up resources
    	pthread_mutex_destroy(&total_lock);
    	free(s1);
    	free(s2);
	
	pthread_exit(0);
}



int readf(FILE *dummy)
{
    fp = fopen("strings.txt", "r");
    if (fp == NULL) {
        printf("ERROR: can't open strings.txt!\n");
        return -1;
    }

    s1 = (char *)malloc(sizeof(char) * MAX);
    s2 = (char *)malloc(sizeof(char) * MAX);

    if (fgets(s1, MAX, fp) == NULL) return -1;
    if (fgets(s2, MAX, fp) == NULL) return -1;

    // Strip trailing newlines if present
    s1[strcspn(s1, "\r\n")] = 0;
    s2[strcspn(s2, "\r\n")] = 0;

    n1 = strlen(s1);
    n2 = strlen(s2);
    
    if (n1 < n2) return -1;

    nlocal = n1 / NUM_THREADS;
    fclose(fp);
    return 0;
}

void *sub_string(void *threadid) 
{
    long tid = (long)threadid;
    
    // Determine the starting and ending indices
    int start = tid * nlocal;
    int end = (tid == NUM_THREADS - 1) ? (n1 - n2) : (start + nlocal);

    int local_count = 0;

    // Each thread searches in its assigned range
    // Only check up to (n1 - n2) bc a substr of len n2 cannot start after that.
    for (int i = start; i < end && i <= (n1 - n2); i++) {
        // Check if s2 matches the substring of s1 starting at index i
        if (strncmp(&s1[i], s2, n2) == 0) {
            local_count++;
        }
    }

    // Update the global total using the mutex
    pthread_mutex_lock(&total_lock);
    total += local_count;
    pthread_mutex_unlock(&total_lock);

    pthread_exit(NULL);
}







