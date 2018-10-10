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



int main(int argc, char* argv[]) {
    pid_t pid;
    char c;
    char p[BUFSIZ+1];
    int pp[2]; // parent writes to child(1) and parent reads from child (0)
    int cp[2]; // child writes to parent (1) and child reads from parent(0)
    char disable[23] = "display2d:false$expand(";  
    char end[2] = ");";
    char inp[100], sent[100];
    char tmp[BUFSIZ+1];


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
		char q[4] = "quit"; // quit instance
		int t = 0; // temp variable
		// check if input == quit, yes this is a weird way to check
		for(int i =0; i < 4;i++){
			if(inp[i] == q[i])
				t++; //increment
			else
				continue;
		}
				
                if(t==4) { // if quit, send close command
                    write(pp[1], "quit();" , strlen("quit();")); // send quit condition
                    close(pp[1]); //close pipes
                    close(cp[0]);
                }
		strcat(disable, inp);		
		strcat(disable, end);


                write(pp[1], disable, strlen(disable)); // send full string
		write(pp[1], "\n", 1); // send new line as enter/EOF
                do {
                    read(cp[0], &c, 1); // read output from maxima
                    write(STDOUT_FILENO, &c, 1); // write output to terminal
                } while(c != '\n'); // do while there isn't a newline
                close(pp[1]); // close parent write
                waitpid(pid, NULL, 0); // wait on child before next input
                close(cp[0]); // close child read
            } 

        }
        if(argc > 1) {
            close(pp[0]);
            close(cp[1]);
            for(int i = 1; i < argc; i++) {
                strcat(tmp, "display2d:false$expand(");
                strcat(tmp, argv[i]);
                strcat(tmp, ");");
                write(pp[1], tmp, strlen(tmp));
                write(pp[1], "\n", 1);
                do {
                    read(cp[0], &c, 1);
                    write(STDOUT_FILENO, &c, 1);
                    //fflush(stdout);
                } while(c != '\n');
            }
            write(pp[1], "quit()$\n", strlen("quit()$\n"));
            close(pp[1]);
            waitpid(pid, NULL, 0);
            close(cp[0]);
        }
    }
    return 0;
}
