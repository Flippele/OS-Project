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
		if (c == '|')
		{
			size += CHUNK;
			string = realloc(string, size*sizeof(char));
			string[i++]=' ';
			string[i++]=c;
			string[i++]=' ';

		}
		else
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

void get_args(char** args, char* buffer)
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
		args[i]=(char*) malloc(sizeof(char*)); 
   		strcpy(args[i++], token);
    		token = strtok (NULL, " \t");
 	}
	if (i == 0) args[0]="\0";  /* Εαν το buffer ειναι μονο κενοι χαρακτηρες τοτε η συναρτηση επιστρεφει τιμη null στην πρωτη θεση
					* του args για να μην υπαρξει segmentation fault */	
}

int get_argc(char** args)
{
	/*Μετρητης των arguments στην εισοδο*/
	int i = 0;
	while (args[i] != NULL)
	{
		i++;
	}
	return i;
}

int get_pipec(char** args)
{
	/*Μετρητης των pipes, για κανενα pipe ο μετρητης ειναι 1, για ενα ο μετρητης ειναι 2 κ.ο.κ*/
	int i = 0;
	int pipes = 1;
	while (args[i] != NULL)
	{
		if (!strcmp(args[i++], "|")) pipes++;
	}
	return pipes;
}

void set_argv(int pipec, int argc, char* argv[pipec][argc], char** args)
{
	/*Κατανομη των arguments σε δισδιαστατο πινακα string, οι στηλες περιεχουν την εντολη στην πρωτη θεση
	 * και τις παραμετρους στις επομενες, και η καθε γραμμη αντιστοιχει σε ενα μερος του pipe
	 * π.χ για εισοδο "ls -l /tmp | wc -l" ο πινακας argv θα ειναι:
	 * +_____+_____+______+______+______+______+
	 * | ls  | -l  | /tmp |(null)|(null)|(null)|
	 * | wc  | -l  |(null)|(null)|(null)|(null)|
	 * +-----+-----+------+------+------+------+
	 * */
	int index = 0;
	
	for (int x = 0; x < pipec; x++)
	{
		int y = 0;
		while (( y < argc ) && ( index < argc))
		{
			if (!strcmp(args[index], "|")){ index++; break; }
			if (index >= argc) break;
			argv[x][y++] = args[index++];
		}
	}
}

void exec_pipes(int pipec, int argc, char* argv[pipec][argc], int fd[2])
{
	pipe(fd);

	char ** exec_args = (char**) calloc (argc+1, sizeof(char*));

	if ( fork() == 0 )
	{
		for ( int i = 0; i < argc; i++ )
		{
			exec_args[i] = argv[0][i];
		}
		exec_args[argc] = NULL;

		dup2(fd[1], 1);
		close(fd[0]);
		close(fd[1]);
		execvp(exec_args[0], exec_args);
	}
	if ( fork() == 0 )
	{
					
		for ( int i = 0; i < argc; i++ )
		{
			exec_args[i] = argv[1][i];
		}
		exec_args[argc] = NULL;

		dup2(fd[0], 0);
		close(fd[1]);
		close(fd[0]);
		execvp(exec_args[0], exec_args);
		exit(1);
	}
	close(fd[0]);
	close(fd[1]);
	wait(0);
	wait(0);
}

int sh_chdir(char** args, int argc)
{
	/*Εδω υλοποιειται η αλλαγη directory του shell*/

	if (argc < 2) return 1;  //Εαν ο χρηστης εχει γραψει απλως 'cd' τοτε δεν καλειται η συναρτηση

	int dir_status; //Μεταβλητη ελεγχου υπαρξης του στοχευμενου directory
	const char* home = getenv("HOME"); //Το home directory του χρηστη
	char* fixdir; // Η τελικη μορφη του directory που θα περασει στην συναρτηση chdir
	const char first = args[1][0]; // Ο πρωτος χαρακτηρας του επικαλουμενου directory

	fixdir = (char*) malloc(CHUNK*sizeof(char));

	switch (first)
	{	
		case '~' :
		       	//Αντικατασταση του χαρακτηρα ~ με το home directory του χρηστη
	
			fixdir = (char*) realloc(fixdir, (strlen(args[1]) + strlen(home))*sizeof(char));
			strcpy(fixdir, home);
			args[1]++;
			strcat(fixdir, args[1]);
			dir_status = chdir(fixdir);
			break;

		default :
			if (first != '/' && first != '.') // Εαν το επικαλουμενο directory δεν ειναι path ή relative path
							 // Θεωρειται οτι βρισκεται στο working dir
			{
				fixdir = (char*) realloc(fixdir, (strlen(args[1]) + 2)*sizeof(char));
				strcpy(fixdir, "./");
				strcat(fixdir, args[1]);
				dir_status = chdir(fixdir);
			}
			else dir_status = chdir(args[1]);
	}
	free(fixdir);
	if (dir_status == -1) { printf("%s : Not a directory!\n", args[1]); return 1;}
	return 0;
}

int main(void)
{
	//Μεταβλητές
	char* buffer; // Το string εισοδου του χρηστη
	char* args[1024]; // Η καθε εντολη και οι παραμετροι τους
	int condition=1; // Συνθηκη εξοδου, εξοδος για condition = 0
	int argc; // Αριθμος εντολων
	int pipec; // Αριθμος pipes
	int status;


	print_prompt();
	
	//SHELL LOOP
	for(;;)
	{
		buffer = parser(); 
		get_args(args, buffer); 
			free(buffer);
		argc = get_argc(args); 
		pipec = get_pipec(args);
		
		char *argv[pipec][argc];
		memset(argv, 0, sizeof argv);

		set_argv(pipec, argc, argv, args);

		pid_t pid = fork();
		if (pid == 0)
		{
			//Child Process	
			if (pipec == 1)
			{
				char ** exec_args = (char**) calloc (argc+1, sizeof(char*));
				for ( int i = 0; i < argc; i++ )
				{
					exec_args[i] = argv[0][i];
				}
				exec_args[argc] = NULL;

				execvp(exec_args[0], exec_args);
			}
			else
			{
				int fd[2];
				exec_pipes(pipec, argc, argv, fd);

			}
			exit(0);
		}
		if (pid > 0)
		{
			//Parent Process
			wait(NULL);

			if (strcmp(args[0], "cd") == 0) sh_chdir(args, argc); //Επικληση της cd σε περιπτωση που η εισοδος ειναι cd

			condition = strcmp(args[0], EXIT);
			if (condition == 0) return 0;//exit for args[0] = "exit"

			memset(args, 0, 1024);//Αρχικοποιηση του πινακα εντολων

			print_prompt();
		}
	}	//END OF SHELL LOOP	

	return 0;
}
