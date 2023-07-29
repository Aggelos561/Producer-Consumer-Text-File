#include "my_functions.h"
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>


#define MAX_BUFFER_SIZE 100
#define PROD_SEM_READ "prod_sem_read"
#define CON_SEM_WAIT_REQ "con_sem_wait_req"
#define CON_SEM_READ "con_sem_read"
#define CON_SEM_WRITE "con_sem_write"

#define MEM_NAME "SHARED_MEMORY"


typedef struct shared_memory {

  char buffer[MAX_BUFFER_SIZE];
  void *memory;

} memshared;



int main(int argc, char* argv[]){

 
  double total_time = 0;

  //Random seed for srand based on pid of child
  srand(getpid());

  int N = atoi(argv[1]);
  int lines = atoi(argv[2]);

   //Opening all 4 semaphores and checking for failures
   sem_t* con_sem_read = sem_open(CON_SEM_READ, O_RDWR);

    if (con_sem_read == SEM_FAILED) {
      perror("Consumer Read Semaphore Failed To Open");
      exit(EXIT_FAILURE);
    }

    sem_t* con_sem_write = sem_open(CON_SEM_WRITE, O_RDWR);

    if (con_sem_write == SEM_FAILED) {
      perror("Consumer Write Semaphore Failed To Open");
      exit(EXIT_FAILURE);
    }

    sem_t* prod_sem_read = sem_open(PROD_SEM_READ, O_RDWR);

    if (prod_sem_read == SEM_FAILED) {
      perror("Producer Read Semaphore Failed To Open");
      exit(EXIT_FAILURE);
    }

    sem_t* con_sem_wait_req = sem_open(CON_SEM_WAIT_REQ, O_RDWR);

    if (con_sem_wait_req == SEM_FAILED) {
      perror("Consumer Wait Request Semaphore Failed To Open");
      exit(EXIT_FAILURE);
    }

    //Opening Shared Memory
    memshared shared_memory;
    int shm_fd = shm_open(MEM_NAME, O_CREAT | O_RDWR, 0666);
    shared_memory.memory = mmap(0, 100 , PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    for (int i = 0; i < N; i++) {

      //generate a random line to request
      int random_line = rand() % (lines + 1 - 1) + 1;

      sem_wait(con_sem_write); //consumer semaphore down so we can write in shared memory
      // next consumer child proccess must wait for the previous consumer child
      // to read and print the line before overwrite the shared memory
      //So we down the semaphore until the previous proccess reads it and then we raise it.
      sem_wait(con_sem_wait_req); 

      sprintf(shared_memory.buffer, "%d", random_line);
      strcpy(shared_memory.memory, shared_memory.buffer); // write in shared memory via the buffer
      clock_t begin = clock();

      sem_post(prod_sem_read); 
      sem_wait(con_sem_read);

      //calculating total time for request-response to find the average
      clock_t end = clock();
      total_time += (double)(end - begin) / CLOCKS_PER_SEC;

      printf("The Consumer ID: %d --> Consumer got this Aswear from Producer --> %s\n",getpid(),  (char*) shared_memory.memory);
      fflush(stdout);

      // Raising the semaphore so the next child process witch downed the write
      // semaphore can write in shared memory
      sem_post(con_sem_wait_req);

    }

    //Average time of all requests for the child process
    double average_time = total_time/N;
    printf("Consumer ID --> %d   Average Execution Time --> %f seconds\n", getpid(), average_time);

    return 0;
}