/* Operating systems with C */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>

#define FILENAME "data.txt" //name of file created
#define NUM_THREADS 4   //number of threads
#define NUM_CHARS 2000  //number of chars in text file to be created

sem_t sem1, sem2;
pthread_mutex_t mutex;

void* read_file(void* arg) {
    char* filename = (char*) arg;
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {/*checking for errors*/
        perror("open");
        exit(EXIT_FAILURE);
    }

    char buffer[NUM_CHARS / NUM_THREADS];
    int num_read, total_count = 0;
    while ((num_read = read(fd, buffer, sizeof(buffer))) > 0) {
        int count[26] = {0};
        for (int i = 0; i < num_read; i++) {
            if (buffer[i] >= 'a' && buffer[i] <= 'z') {
                count[buffer[i] - 'a']++;
            }
        }

        pthread_mutex_lock(&mutex);
        for (int i = 0; i < 26; i++) {
            total_count += count[i];
        }
        pthread_mutex_unlock(&mutex);
    }

    close(fd);
    return (void*) (intptr_t) total_count;
}

void write_file() {/* creating the designated file*/
    int fd = open(FILENAME, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd == -1) {/*checking for errors*/
        perror("open");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL)); /*writing 2000 random a-z letters*/
    char buffer[NUM_CHARS];
    for (int i = 0; i < NUM_CHARS; i++) {
        buffer[i] = rand() % 26 + 'a';
    }

    if (write(fd, buffer, sizeof(buffer)) == -1) {
        perror("write");/*checking for errors*/
        exit(EXIT_FAILURE);
    }

    close(fd);
    return 0;
}

void sig_handler(int signum) { /*checks for terminating signals*/
    char answer[5];
    if (signum == SIGINT || signum == SIGTERM) {
        printf("Are you sure? (y/n): ");
        
        fgets(answer , 5, stdin);
        if (answer[0] == 'y') {
            exit(EXIT_SUCCESS);
        }
        perror("signal");
    }
}

int main() {
    signal(SIGINT, sig_handler);    /*receiving sigint signal*/
    signal(SIGTERM, sig_handler);   /*receiving sigterm signal*/
    
    int total_count = 0;
    int count[26]={0};
    int thread_ids[NUM_THREADS];
    int char_counts[26] = {0};


    //sem_t *sem1;                                                                
    //sem_init(&sem1, 0, 0);          

    //pthread_mutex_t mymutex=pthread_mutex_initializer;
   
    pid_t pid = fork();
    if (pid == -1) { /*checking for errors*/
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // child process

        write_file();
        sem_post(&sem1);
        exit(EXIT_SUCCESS);
    } else {
        // parent process

        sleep(5);
        //sem_wait(&sem1);

        pthread_t threads[NUM_THREADS];
        for (int i = 0; i < NUM_THREADS; i++) {
            if (pthread_create(&threads[i], NULL, read_file, FILENAME) != 0) {
                perror("pthread_create");
                exit(EXIT_FAILURE);
            }
        }
        
        for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, read_file, (void*) &thread_ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < 26; i++) {
        printf("%c: %d\n", 'a' + i, char_counts[i]);
    }
        int partial_count=0;
        for (int i = 0; i < NUM_THREADS; i++) {
            void* result;
            if (pthread_join(threads[i], &result) != 0) {/*checking for errors*/
                perror("pthread_join");
                exit(EXIT_FAILURE);
            }
            partial_count += (int) (intptr_t) result; /*counts number of letters accessed*/

        }
    }
    return 0;
}