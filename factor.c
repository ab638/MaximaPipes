#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define SWOLL    BUFSIZ
#define TOOSWOLL BUFSIZ+1

int main(int argc, char* argv[]) {
    pid_t pid;
    char c[TOOSWOLL]; // how swoll is the buffer? +1
    int pp[2]; // parent writes to child(1) and parent reads from child (0)
    int cp[2]; // child writes to parent (1) and child reads from parent(0)
    char disable[24] = "display2d:false$factor(";
    char end[3] = ");";
    char inp[100];
    char tmp[TOOSWOLL];
    char buffer[TOOSWOLL]; 
    char t[10];
    if(pipe(pp) < 0|| pipe(cp) <0 ) {
        fprintf(stderr,"Pipe Failure");
        exit(1);
    }
    if((pid = fork()) < 0) {
        fprintf(stderr,"Fork Failure");
        exit(2);
    }
    if(pid == 0) { // child process
 
        close(pp[1]); // unused pipes get closed
        close(cp[0]);
        dup2(pp[0],STDIN_FILENO); // copy stdin
        close(pp[0]);
        dup2(cp[1],STDOUT_FILENO); // copy stdout
        close(cp[1]); // finish closing so exec doesn't see them
        execlp("maxima","maxima", "-q", (char*)0);
  
        exit(EXIT_FAILURE);
    }
    else {
        if(argc == 1) { // parent process
            
            close(pp[0]); // close unused ends of pipes
            close(cp[1]);
            
            while(1) {
                printf("> "); // prompt user
                
                scanf("%s", inp); // get input
                char q[5] = "quit"; // quit instance
                
                if(!strcmp(q, inp)) { // if quit, send close command
                    write(cp[1], "quit();" , strlen("quit();")); // send quit condition
                    close(pp[1]); //close pipes
                    close(cp[0]);
                    break;
                }
                strcat(disable, inp);

                strcat(disable, end);
                 
                strcpy(buffer, disable);
              //  printf("Command sent to Child input: %s\n", buffer);     
                write(pp[1], buffer, SWOLL); // send full string
             //   printf("Contents of Child Pipe from Maxima\n");
		
               
		while(strcmp(c, "\n(%%i#)") ){ // do while there isn't a newline
                    read(cp[0], c, SWOLL); // read output from maxima
                    write(STDOUT_FILENO, c, SWOLL); // write output to terminal
		    fflush(stdout);
		    
                }
               
                close(pp[1]); // close parent write
                waitpid(pid, NULL, 0); // wait on child before next input
                close(cp[0]); // close child read
            }

        }
        if(argc > 1) {
            close(pp[0]);
            close(cp[1]);
            for(int i = 1; i < argc; i++) {
                strcat(tmp, "display2d:false$factor(");
                strcat(tmp, argv[i]);
                strcat(tmp, ");");
                write(pp[1], tmp, strlen(tmp));
                write(pp[1], "\n", 1);
                do {
                    read(cp[0], c, SWOLL);
	
                    write(STDOUT_FILENO, c, SWOLL);
		    
                } while(c != "(%i");
            }
            write(pp[1], "quit()$", strlen("quit()$"));
            close(pp[1]);
            waitpid(pid, NULL, 0);
            close(cp[0]);
	    kill(SIGTERM, pid);
        }
    }
    return 0;
}
