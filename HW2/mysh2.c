#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define PROMPT "$"
#define CHUNK 8 // ελαχιστο μεγεθος χαρακτηρων που μπορει να ειναι ενα string. ολα τα string ειναι πολλαπλασια του chunk σε μεγεθος
#define EXIT "exit"  //exit condition


void print_prompt(void)
{
	printf ("%s ", PROMPT);
}

void newline(void)
{
	printf ("\n");
}

char* parser(void)
{
	/*Η συναρτηση επιστρεφει την εισοδο (stdin) σε μορφη string array
	 * με δυναμικα κατανεμημενη μνημη.*/
	char c, *string;
	unsigned int i=0;
        size_t size=CHUNK;
	string = malloc(size*sizeof(char));//αρχικο μεγεθος του string (8 char)
	while((c=fgetc(stdin)) != '\n' && c != EOF) 
	{
		/*Η while ελεγχει καθε χαρακτηρα ξεχωριστα μεχρι να βρει
		 * newline ή EOF. Εαν το μηκος του string ειναι ισο με το
		 * μεγεθος του σε μνημη τοτε γινεται ανακατανομη ενος πλεον
		 * CHUNK στην μνημη που χρησιμοποιει το string*/
		string[i++]=c;
		if (i==size)
		{
			size += CHUNK;
			string = realloc(string, size*sizeof(char));
		}
	}
	string[i++]='\0'; //τοποθετηση του null χαρακτηρα στο τελος του string
	return realloc(string, i*sizeof(char));
}

char* get_args(char** A, char* buffer)
{
	/*Τομη του buffer σε διαφορετικα string με την χρηση της strtok
	 * για την προσπελαση των εντολων στην execvp*/
	const char len = strlen(buffer);
	char* token;
	token = strtok (buffer," \t");//To προγραμμα εχει ως delimeter τον κενο χαρακτηρα και τον χαρακτηρα tab
	int i=0;
	while (token != NULL)
	{
		// Καταγραφη καθε token σε πινακα A[i] μεχρι το string buffer να φτασει στο τελος του
		A[i]=(char*) malloc(sizeof(char*)); 
   		strcpy(A[i++], token);
    		token = strtok (NULL, " \t");
 	}
	if (i == 0) return A[0]="\0";  /* Εαν το buffer ειναι μονο κενοι χαρακτηρες τοτε η συναρτηση επιστρεφει τιμη null στην πρωτη θεση
					* του argv για να μην υπαρξει segmentation fault */	
  
  return *A;
}

int get_argc(char** A)
{
	int i = 0;
	while (A[i] != NULL)
	{
		i++;
	}
	return i;
}
int sh_chdir(char** argv, int argc)
{
	/*Εδω υλοποιειται η αλλαγη directory του shell*/

	if (argc < 2) return 1;  //Εαν ο χρηστης εχει γραψει απλως 'cd' τοτε δεν καλειται η συναρτηση

	int dir_status; //Μεταβλητη ελεγχου υπαρξης του στοχευμενου directory
	const char* home = getenv("HOME"); //Το home directory του χρηστη
	char* fixdir; // Η τελικη μορφη του directory που θα περασει στην συναρτηση chdir
	const char first = argv[1][0]; // Ο πρωτος χαρακτηρας του επικαλουμενου directory

	fixdir = (char*) malloc(CHUNK*sizeof(char));

	switch (first)
	{	
		case '~' :
		       	//Αντικατασταση του χαρακτηρα ~ με το home directory του χρηστη
	
			fixdir = (char*) realloc(fixdir, (strlen(argv[1]) + strlen(home))*sizeof(char));
			strcpy(fixdir, home);
			argv[1]++;
			strcat(fixdir, argv[1]);
			dir_status = chdir(fixdir);
			break;

		default :
			if (first != '/' && first != '.') // Εαν το επικαλουμενο directory δεν ειναι path ή relative path
							 // Θεωρειται οτι βρισκεται στο working dir
			{
				fixdir = (char*) realloc(fixdir, (strlen(argv[1]) + 2)*sizeof(char));
				strcpy(fixdir, "./");
				strcat(fixdir, argv[1]);
				dir_status = chdir(fixdir);
			}
			else dir_status = chdir(argv[1]);
	}
	free(fixdir);
	if (dir_status == -1) { printf("%s : Not a directory!\n", argv[1]); return 1;}
	return 0;
}

int main(void)
{
	//Μεταβλητές
	char* buffer;
	char* argv[1024];
	int condition=1;
	int argc;


	print_prompt();
	
	//SHELL LOOP
	for(;;)
	{
		buffer = parser(); //Το string εισοδου του χρηστη
		get_args(argv, buffer); //Ο πινακας εντολων
		argc = get_argc(argv); //Ο αριθμος εντολων

		pid_t pid = fork();
		if (pid == 0)
		{
			//Child Process
			free(buffer);
			execvp(argv[0], argv);
			exit(0);
		}
		if (pid > 0)
		{
			//Parent Process
			wait(NULL);

			if (strcmp(argv[0], "cd") == 0) sh_chdir(argv, argc); //Επικληση της cd σε περιπτωση που η εισοδος ειναι cd

			condition = strcmp(argv[0], EXIT);
			if (condition == 0) return 0;//exit for argv[0] = "exit"

			memset(argv, 0, 1024);//Αρχικοποιηση του πινακα εντολων

			print_prompt();
		}
	}	//END OF SHELL LOOP	

	return 0;
}
