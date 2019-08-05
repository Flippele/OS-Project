#include <stdlib.h> 
#include <pthread.h> /*atoi()*/
#include "util.h" /*pthread_t, pthread_mutex_t, pthread_mutex_[un]lock(), pthread_create(), pthread_join()*/

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
	//Χρησιμοποιείται struct γιατι θελουμε πολλες τιμες ως ορισμα στην αρχικη συναρτηση των thread
	//arg_num = argc
	//argument = argv[i] οπου i : [2,argc]
	int arg_num;
	char* argument;
} thread_params;

void* run_thread(void *arg) //Συνάρτηση έναρξης νέου thread
{
	thread_params *tp = (thread_params*) arg;
	//Typecasting της παραμετρου τυπου void* σε τυπο thread_params για χρηση του στην συναρτηση

	for (int i = 0; i < tp->arg_num; i++)
	{
		pthread_mutex_lock(&mutex);
		display(tp->argument);
		pthread_mutex_unlock(&mutex);
	}	

	pthread_exit(NULL);
}

int main(int argc, char** argv)
{	
	if (argc < 2) return 0;

	pthread_t pid[argc-2];
	thread_params tp[argc-2];

	for (int i = 2; i < argc; i++)
	{
		tp[i-2].argument = argv[i];
		tp[i-2].arg_num = atoi(argv[1]);
		pthread_create(&pid[i-2], NULL, run_thread, &tp[i-2]);
	}

	for (int i = 0; i < argc-2; i++) pthread_join(pid[i], NULL);
	pthread_mutex_destroy(&mutex);
	return 0;
}
