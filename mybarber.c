//gcc -Wall -pthread 
#define MAX 20
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

void *client(void *param);
void *barber(void *param);

sem_t chairs_mutex;
sem_t sem_client;
sem_t sem_barber;
int num_chairs;
int clientWait;
int main(int argc, char *argv[]) {
	pthread_t barberid;
	pthread_t clientids[MAX];
	
	printf("Main thread beginning\n");
   /* 1. Get command line arguments */
   int runTime,clients,i;
   if (argc != 5){
	   printf("Please enter 4 arguments: <Program run time> <Number of clients>\n");
	   printf("<Number of chairs> <Client wait time>\n");
	   exit(0);
   }
   runTime = atoi(argv[1]);
   clients = atoi(argv[2]);
   num_chairs = atoi(argv[3]);
   clientWait = atoi(argv[4]);
   /* 2. Initialize semaphores */
   sem_init(&chairs_mutex,0,1);
   sem_init(&sem_client,0,0);
   sem_init(&sem_barber,0,0);
   /* 3. Create barber thread. */
   pthread_create(&barberid, NULL, barber, NULL);
   printf("Creating barber thread with id %lu\n",barberid);
   /* 4. Create client threads.  */
   for (i = 0; i < clients; i++){
	   pthread_create(&clientids[i], NULL, client, NULL);
	   printf("Creating client thread with id %lu\n",clientids[i]);
   }
   /* 5. Sleep. */
   printf("Main thread sleeping for %d seconds\n", runTime);
   sleep(runTime);
   /* 6. Exit.  */
   printf("Main thread exiting\n");
   exit(0);
}

void *barber(void *param) {
   int worktime;
  
   while(1) {
      /* wait for a client to become available (sem_client) */
	  sem_wait(&sem_client);
      /* wait for mutex to access chair count (chair_mutex) */
	  sem_wait(&chairs_mutex);
      /* increment number of chairs available */
	  num_chairs += 1;
	  printf("Barber: Taking a client. Number of chairs available = %d\n",num_chairs);
      /* signal to client that barber is ready to cut their hair (sem_barber) */
	  sem_post(&sem_barber);
      /* give up lock on chair count */
	  sem_post(&chairs_mutex);
      /* generate random number, worktime, from 1-4 seconds for length of haircut.  */
	  worktime = (rand() % 4) + 1;
      /* cut hair for worktime seconds (really just call sleep()) */
	  printf("Barber: Cutting hair for %d seconds\n", worktime);
	  sleep(worktime);
    } 
}

void *client(void *param) {
   int waittime;

   while(1) {
      /* wait for mutex to access chair count (chair_mutex) */
	  sem_wait(&chairs_mutex);
      /* if there are no chairs */
	  if(num_chairs <= 0){
           /* free mutex lock on chair count */
		   printf("Client: Thread %u leaving with no haircut\n", (unsigned int)pthread_self());
		   sem_post(&chairs_mutex);
	  }
      /* else if there are chairs */
	  else{
           /* decrement number of chairs available */
		   num_chairs -= 1;
		   printf("Client: Thread %u Sitting to wait. Number of chairs left = %d\n",(unsigned int)pthread_self(),num_chairs);
           /* signal that a customer is ready (sem_client) */
		   sem_post(&sem_client);
           /* free mutex lock on chair count */
		   sem_post(&chairs_mutex);
           /* wait for barber (sem_barber) */
		   sem_wait(&sem_barber);
           /* get haircut */
		   printf("Client: Thread %u getting a haircut\n",(unsigned int)pthread_self());
	  }
      /* generate random number, waittime, for length of wait until next haircut or next try.  Max value from command line. */
	  waittime = (rand() % clientWait) + 1;
      /* sleep for waittime seconds */
	  printf("Client: Thread %u waiting %d seconds before attempting next haircut\n",(unsigned int)pthread_self(),waittime);
	  sleep(waittime);
     }
}