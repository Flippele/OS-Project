#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> /*wait()*/
#include <semaphore.h> /*sem_t, sem_open(), sem_wait(), sem_post(), sem_unlink()*/
#include <fcntl.h> /*O_CREAT, O_EXCL*/
#include "util.h"

int main(int argc, char** argv)
{
if (argc < 2) return 0; //Το πρόγραμμα χρειάζεται τουλάχιστον δυο παραμέτρους για να λειτουργήσει σωστά

int rep = atoi(argv[1]); //Αριθμός κλήσεων της display() ανα διεργασία-παιδί

sem_t *mutex = sem_open("/mutex", O_CREAT, 0644, 1); //semaphore συγχρονισμού των διεργασιών-παιδιών.
for (int i = 2; i < argc; i++)
{
	if (fork() == 0)
	{
		sem_t *mutex = sem_open("/mutex", O_EXCL);

		for (int x = 0; x < rep; ++x)
		{
			sem_wait(mutex);
			display(argv[i]);
			sem_post(mutex);
		}
		exit(0);
	}
}
for (int i = 2; i < argc; i++) wait(NULL);

sem_unlink("/mutex");
return 0;
}
