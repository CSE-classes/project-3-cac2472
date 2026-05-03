// gcc producer_consumer.c -o pc_task -lpthread
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 5

char buffer[BUFFER_SIZE];
int count = 0;    // Number of items in buffer
int head = 0;     // Next free position for Producer
int tail = 0;     // Next item for Consumer to read
int finished = 0; // Flag to indicate end of file

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;  // Buffer has space
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER; // Buffer has data

void* producer(void* arg) {
    FILE* fp = fopen("message.txt", "r");
    if (!fp) {
        printf("Error: Could not open message.txt\n");
        finished = 1;
        pthread_cond_signal(&cond_empty);
        return NULL;
    }

    char ch;
    while ((ch = fgetc(fp)) != EOF) {
        pthread_mutex_lock(&mutex);

        // Wait while buffer is full
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&cond_full, &mutex);
        }

        // Add character to circular queue
        buffer[head] = ch;
        head = (head + 1) % BUFFER_SIZE;
        count++;

        // Signal consumer that data is available
        pthread_cond_signal(&cond_empty);
        pthread_mutex_unlock(&mutex);
    }

    fclose(fp);
    
    // Mark as finished so consumer knows when to stop
    pthread_mutex_lock(&mutex);
    finished = 1;
    pthread_cond_signal(&cond_empty);
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

void* consumer(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        // Wait while buffer is empty AND producer is still working
        while (count == 0 && !finished) {
            pthread_cond_wait(&cond_empty, &mutex);
        }

        // Exit condition: No more data and producer is done
        if (count == 0 && finished) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Read character from circular queue
        char ch = buffer[tail];
        tail = (tail + 1) % BUFFER_SIZE;
        count--;

        printf("%c", ch);
        fflush(stdout); // Ensure character prints immediately

        // Signal producer that there is now space
        pthread_cond_signal(&cond_full);
        pthread_mutex_unlock(&mutex);
    }
    printf("\n");
    return NULL;
}

int main() {
    pthread_t prod_thread, cons_thread;

    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_full);
    pthread_cond_destroy(&cond_empty);

    return 0;
}
