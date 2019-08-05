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
int strings = argc; //Αριθμός παραμέτρων πάνω στον οποίον θα γίνονται πράξεις
int semval; //Μεταβλητή αποθήκευσης τιμής semaphore

sem_t *mutex = sem_open("/mutex", O_CREAT, 0644, 0); //semaphore συγχρονισμού των διεργασιών-παιδιών για την display, αρχικά κλειστός
sem_t *imutex = sem_open("/imutex", O_CREAT, 0644, 1); //semaphore συγχρονισμού της συνάρτησης init(), αρχικά ανοιχτός
sem_t *sem = sem_open("/sem", O_CREAT, 0644, argc-2); //semaphore μέτρησης των συναρτήσεων init() που ολοκληρώθηκαν.

for (int i = 2; i < argc; i++)
{
	if (fork() == 0)
	{
		int semval;

		sem_t *mutex = sem_open("/mutex", O_EXCL);
		sem_t *imutex = sem_open("/imutex", O_EXCL);
		sem_t *sem = sem_open("/sem", O_EXCL);

		sem_wait(imutex);

		init();
		sem_wait(sem); //Αναμονή του sem μετα την ολοκληρωση της init

		sem_getvalue(sem, &semval);
		if (semval == 0) sem_post(mutex); //Εαν η τιμή του sem είναι 0 τοτε σημαίνει οτι όλες οι init() έχουν ολοκληρωθεί αρα ανοίγει ο σηματοφόρος mutex

		sem_post(imutex);

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
sem_unlink("/imutex");
sem_unlink("/sem");
return 0;
}
