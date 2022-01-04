#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

//maximum number of characters that our program will handle at any given time.
const int BUFFER_SIZE=80;

//will force quit the program when ctrl+c is hit.
void sigint_handler(int sig) {
	write(1,"Terminating through signal handler\n",35);
	exit(0);
}

//prints the input to stdout. Can be used to check the | functionality required in the assignment.
void echo(char* input) {
	printf("%s\n",input);
}

//prints the current working directory.
void pwd() {
	char cwd[1000];
	getcwd(cwd,sizeof(cwd));
	printf("%s\n", cwd);
	return;
}

//exits the shell and provides a message.
void exitshell() {
	printf("Exiting program with code 0.\n");
	exit(0);
}

//returns true if "str" starts with "pre".
bool startsWith(const char* pre, const char* str) {
	return strncmp(pre,str,strlen(pre)) == 0;
}

void printHelp() {
	printf("=============================HELP==============================\n");
	printf("CTRL + C  : Force quit the terminal.\n");
	printf("pwd       : Print the current working directory.\n");
	printf("exit      : Exits the shell.\n");
	printf("cd [dir]  : Changes the current directory to [dir].\n");
	printf("x|y       : Uses the output from program x as the input to program y.\n");
	printf("echo input: Prints whatever is after 'echo ' (note the space) to stdout. \n");
	printf("game      : play a guessing game against the computer.\n");
	printf("To demonstrate that the | functionality works, as the assignment requried,\n");
	printf("try something like 'pwd|echo' to have the results of pwd be the first\n");
	printf("argument for echo.\n");
	printf("=============================HELP==============================\n");

}

//analyzes input, returns a pointer to a string of whatever is after the pipe character.
//Must free the pointer after use.
char* checkForPipe(char* input) {
	char* ret = NULL;
	const char needle[1] = "|";
	ret = strstr(input,needle);
	char* new = (char*)malloc(sizeof(char)*BUFFER_SIZE);
	if(ret!=NULL) {
		strcpy(new,ret+1);
	}
	return new;
}


//Guessing game against the computer. User enters a max number, and then a random number
//between 1 and max is generated. They have to guess the random number, while the computer
//gives them hints on whether their guess it too low or too high. 
void game() {
	printf("Welcome to the guessing game, where you get to guess a random number!\n");
	printf("How big do you want the range of numbers to be?\n");
	int range = 0;
	char* input;
	printf("Enter any number below 100:");
	fgets(input,3,stdin);
	input[strlen(input)] = '\0';
	range = atoi(input);
	srand(time(NULL));
	int number = ((rand() % range ) +  1);
	char* guess;
	int guessInt;
	while(1) {
		printf("Guess a number between 1 and %d ('0' to exit): ",range);
		fflush(stdin);
		fgets(guess,3,stdin);
		guessInt = atoi(input);
		if(guessInt == 0) {
			break;
		}
		printf("You guessed: %d\n",guessInt);
		
		if(guessInt < number) {
			printf("Too low!\n");
			continue;
		}

		if(guessInt > number) {
			printf("Too high!\n");
			continue;
		}
		if(guessInt == number) {
			printf("You got it! The number was %d.\n",number);
			break;
		}
		
	}



}

int parse(char* argv[]) {
	if(strcmp("cd",argv[0])==0) {
		printf("argv[1]:%s\n",argv[1]);
		chdir(argv[1]);
		return 1;
	}

	if(strcmp("help",argv[0])==0) {
		printHelp();
		return 1;
	}

	if(strcmp("game",argv[0])==0) {
		game();
		return 1;
	}
	return 0;
}

int main() {

	//install our signal handler
	signal(SIGINT,sigint_handler);
	printf("You can only terminate by pressing Ctrl+c\n");
	
	//buffer will store initial user input
	char* buffer;

	//argv will store the user inputs and arguments. 
	char* argv[16];
	int a = 0;
	for(a=0;a<16;a++) {
		argv[a] = (char*)malloc(sizeof(char[BUFFER_SIZE]));
	}

	char* argv2[16];
	int b = 0;
	for(b=0;b<16;b++) {
		argv2[b] = (char*)malloc(sizeof(char[BUFFER_SIZE]));
	}
	//will be used if user inputs |
	int fd[2];
	pipe(fd); // fd[0] for reading, fd[1] for writing.
	bool piped;

	

	while(1) {
		printf("mini-shell>"); //prompt
		piped = false;
				
		fflush(stdin); //clear stdin and std out to be safe
		fflush(stdout);

		fgets(buffer,BUFFER_SIZE,stdin); //get user input into a null terminated string.
		buffer[strlen(buffer)-1] = '\0';

		if(strcmp(buffer,"exit") == 0) { //exit if user typed exit.
			break;
		}
	

		char* postPipe_m = checkForPipe(buffer);
		if(strcmp(postPipe_m,"")!=0){ //if there is a pipe in the user input.
			piped = true;
		} 
		if(!piped) { //if there is no pipe, execute the program.
			free(postPipe_m);	
			char* token; //split input by spaces into an array. 
			token = strtok(buffer," ");
			int i = 0;
			while(token!=NULL) {
				argv[i] = token;
				token = strtok(NULL," ");
				i++;
			}
			argv[i]=NULL; //null terminate the array.
			int argCount = i; //number of arguments
		
			char program[100]; //this will execute the program if the user did not use |
			char* bin = "/bin/";
			strcpy(program, bin);
			strcat(program, argv[0]);

			int success = parse(argv); //will try to run any built in commands (cd, help, etc.)
			if(success==0) {
				int pid = fork();
				if(pid==0) {
					execvp(program,argv);
					fprintf(stderr, "Child process error\n");
					exit(0);
				}
				else {
					wait(NULL);
				}
			}
		}
		if(piped){ //if there is a pipe in the user input. 
			char prePipe[BUFFER_SIZE]; //split the input into 'prePipe' and 'postPipe' null terminated strings
			int difference = strlen(buffer) - strlen(postPipe_m) - 1;
			strncpy(prePipe,buffer,difference);
			prePipe[difference] = '\0';
			postPipe_m[strlen(postPipe_m)] = '\0';
			char postPipe[BUFFER_SIZE];
			strcpy(postPipe,postPipe_m);
			free(postPipe_m);
	
			pid_t p1 = fork(); 
			if(p1==0) { //this process will run the pre-pipe program, and write it's output to fd[1].
				close(STDOUT_FILENO); //close stdout
				dup2(fd[1],STDOUT_FILENO);
				
				char* token; //split prePipe by spaces into an array. 
				token = strtok(prePipe," ");
				int i = 0;
				while(token!=NULL) {
					argv[i] = token;
					token = strtok(NULL," ");
					i++;
				}
				argv[i]=NULL; //null terminate the array.
				int argCount = i; //number of arguments
				char program[100];
				char* bin = "/bin/";
				strcpy(program, bin);
				strcat(program, argv[0]);
				close(fd[1]);
				close(fd[0]);
				if(parse(argv)==0){ //run the program.
					execvp(program,argv);
					fprintf(stderr,"Child process error");
					exit(0);
				}
				exit(0); //exit.
			}
			else {
				int p1_status;
				waitpid(p1,&p1_status,0); //wait for p1
				close(fd[1]);
			}
		
			pid_t p2 = fork();
			if(p2==0) { //this process will run the post-pipe program, with the stdin set as the output from processes 1.
				close(STDIN_FILENO);
				dup2(fd[0],STDIN_FILENO);
				char* token;
				token = strtok(postPipe," ");
				int i = 0;
				while(token!=NULL) {
					argv2[i] = token;
					token = strtok(NULL," ");
					i++;
				}
				argv2[i] = NULL;
				int argCount = i;
				char program[100];
				char* bin = "/bin/";
				strcpy(program,bin);
				strcat(program, argv2[0]);
				printf("%s\n",argv2[0]);
				printf("%s\n",program);
				if(parse(argv2)==0) {
					execvp(program,argv2);
					fprintf(stderr,"Error: Command not found\n");
					exit(0);
				}
				exit(0);
			}
			else {
				wait(NULL);
				kill(p2,SIGKILL);
			}
			piped=true;
			}

		if(piped) {
			continue;
		}
	}
	int i;
	for(i=1;i<16;i++) {
		free(argv[i]);
		free(argv2[i]);
	}
	exit(0);
	return 0;

}
