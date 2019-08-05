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
	const char len = strlen(buffer);
	char* token;
	token = strtok (buffer," \t");
	int i=0;
	while (token != NULL)
	{
		A[i]=(char*) malloc(len*sizeof(char)); 
   		strcpy(A[i++], token);
    		token = strtok (NULL, " \t");
 	}
	if (i == 0) return A[0]="\0";  /* Εαν το buffer ειναι μονο κενοι χαρακτηρες τοτε η συναρτηση επιστρεφει τιμη null στην πρωτη θεση
					* του argv για να μην υπαρξει segmentation fault */	
  
  return *A;
}

int main(void)
{
	char* buffer;
	char* argv[1024];
	int condition=1;
	print_prompt();

	
	for(;;)
	{
		buffer = parser();
		get_args(argv, buffer);
		pid_t pid = fork();
		if (pid == 0)
		{
			//Child Process
			execvp(argv[0], argv);
			free(buffer);
			exit(0);
		}
		if (pid > 0)
		{
			//Parent Process
			wait(NULL);
			condition = strcmp(argv[0], EXIT);
		
			if (condition == 0) return 0;
			print_prompt();
		}
		free(buffer);
	}	

	return 0;
}
