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


#define MAX_BUFFER_SIZE 100
#define PROD_SEM_READ "prod_sem_read"
#define CON_SEM_WAIT_REQ "con_sem_wait_req"
#define CON_SEM_READ "con_sem_read"
#define CON_SEM_WRITE "con_sem_write"
#define MEM_NAME "SHARED_MEMORY"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

//Init values for the semaphores
#define SEM_READ_INIT_VALUE 0
#define SEM_WAIT_REQUEST_INIT_VALUE 1
#define SEM_WRITE_INIT_VALUE 1

#define CONSUMER "./consumer"



//Struct Shared Memory where buffer size is 100 and memory will 
//be the shared memory between producer and consumer

typedef struct shared_memory{

  char buffer[MAX_BUFFER_SIZE];
  void* memory;

} memshared;




int main(int argc, char *argv[]){

  if (argc != 4) {
    perror("Wrong Input\n");
    exit(EXIT_FAILURE);
  }

    // Name of .txt
    // K = Number of children processes
    // N = Number of recyclements of each child process
    char file_name[20];
    int K, N;
    
    strcpy(file_name, argv[1]);
    K = atoi(argv[2]);
    N = atoi(argv[3]);

    int lines = count_lines(file_name);
    char lines_str[20];

    sprintf(lines_str, "%d", lines);

    if (lines < 0){
      perror("Error Opening File");
      return -1;

    }


    //Creating 4 Semaphores:
    //CON_SEM_READ -> Semphore for consumer to read from shared memory
    //CON_SEM_WRITE-> Semaphore for consumer to write to shared memory
    //CON_SEM_WAIT_REQ -> Semaphore for consumer to read the result before the next process write into the shared memory
    //PROD_SEM_READ -> Semaphore for producer to read (and write) in shared memory
    //Also checking if semaphores have successfully created and opened

    sem_t* con_sem_read = sem_open(CON_SEM_READ, O_CREAT | O_EXCL, SEM_PERMS, SEM_READ_INIT_VALUE);

    if (con_sem_read == SEM_FAILED) {
      perror("Consumer Read Semaphore Open Failed");
      exit(EXIT_FAILURE);
    }

    sem_t* con_sem_write = sem_open(CON_SEM_WRITE, O_CREAT | O_EXCL, SEM_PERMS, SEM_WRITE_INIT_VALUE);

    if (con_sem_write == SEM_FAILED) {
      perror("Consumer Write Semaphore Open Failed");
      exit(EXIT_FAILURE);
    }

    sem_t* con_sem_wait_req = sem_open(CON_SEM_WAIT_REQ, O_CREAT | O_EXCL, SEM_PERMS, SEM_WAIT_REQUEST_INIT_VALUE);

    if (con_sem_wait_req == SEM_FAILED) {
      perror("Producer Write Semaphore Open Failed");
      exit(EXIT_FAILURE);
    }

    sem_t* prod_sem_read = sem_open(PROD_SEM_READ, O_CREAT | O_EXCL, SEM_PERMS, SEM_READ_INIT_VALUE);

    if (prod_sem_read == SEM_FAILED) {
      perror("Producer Read Semaphore Open Failed");
      exit(EXIT_FAILURE);
    }



    //Create Shared Memory 
    //Shared Memory size is 100 bytes (max size of every sentence line from a text)
    memshared shared_memory;
    int shm_fd;
    shm_fd = shm_open(MEM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, 100);
    shared_memory.memory = mmap(0, 100 , PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    int i;
    int pid;

    //Make Child processes
    for (i = 0; i < K; i++) {

      pid = fork();

      if (pid == 0) {   //Produce K children to execute the consumer programm
        execl(CONSUMER ,CONSUMER , argv[3], lines_str, NULL);
        break;
      }
      else if (pid < 0){
        exit(EXIT_FAILURE);
      }

    }


  
    for(i = 0; i < N * K; i++){

      // Waiting for consumer to write the request in shared memory
      sem_wait(prod_sem_read);

      //Then we read the request and write the response back into the shared memory
      printf("The Producer --> Consumer requested the line --> %s\n",(char *)shared_memory.memory);
      fflush(stdout);
      
      //Getting the line that consumer requested
      get_n_line(file_name, atoi((char *)shared_memory.memory), shared_memory.buffer, sizeof(shared_memory.buffer));
      strcpy(shared_memory.memory, shared_memory.buffer);

      //Now consumer can read the line and the next consumer proccess 
      //can write in memory (once previous consumer has read the line that requested)
      sem_post(con_sem_read);
      sem_post(con_sem_write);

    }
    

    //Parent waits all children to finish
    for (int i = 0; i < K; i++){
      wait(NULL);
    }


    //Closing and Unlinking all semaphores (4 Semaphores)
    if (sem_close(con_sem_read) < 0) {
      perror("Consumer Read Semaphore Close Failed");
      sem_unlink(CON_SEM_READ);
      exit(EXIT_FAILURE);
    }

    if (sem_unlink(CON_SEM_READ) < 0)
      perror("Consumer Read Semaphore Unlink Failed");

    if (sem_close(con_sem_write) < 0) {
      perror("Consumer Write Semaphore Close Failed");
      sem_unlink(CON_SEM_WRITE);
      exit(EXIT_FAILURE);
    }
    if (sem_unlink(CON_SEM_WRITE) < 0)
      perror("Consumer Write Semaphore Unlink Failed");

    if (sem_close(con_sem_wait_req) < 0) {
      perror("Consumer Wait Request Semaphore Close Failed");
      sem_unlink(CON_SEM_WAIT_REQ);
      exit(EXIT_FAILURE);
    }
    if (sem_unlink(CON_SEM_WAIT_REQ) < 0)
      perror("Consumer Wait Request Semaphore Unlink Failed");

    if (sem_close(prod_sem_read) < 0) {
      perror("Producer Read Semaphore Close Failed");
      sem_unlink(PROD_SEM_READ);
      exit(EXIT_FAILURE);
    }
    if (sem_unlink(PROD_SEM_READ) < 0)
      perror("Producer Read Semaphore Unlink Failed");

    
    //Unlink Shared Memory
    shm_unlink(MEM_NAME);

    return 0;
}