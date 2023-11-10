#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

//Defining the possible commands
#define LAUNCH "L" "launch" "l"
#define BOMB "B" "bomb" "b"
#define REFUEL "R" "refuel" "r"
#define QUIT "Q" "quit" "q"

//defining the signals that will be used to communicate between the parent and child processes.
#define SIG_LAUNCH SIGUSR1
#define SIG_BOMB SIGUSR2
#define SIG_REFUEL SIGINT

//Declaring the function prototypes
void handle_launch_signal();
void handle_bomb_signal(pid_t);
void handle_refuel_signal(pid_t);
void handle_invalid_command();
void spawn_plane_process();

int main() {
  //Refister the signal handlers
  signal(SIG_LAUNCH, handle_launch_signal);
  signal(SIG_BOMB, handle_bomb_signal);
  signal(SIG_REFUEL,handle_refuel_signal);

  while(1){
    //Prompy the user for a command
    printf("Enter a command: ");
    char command[10];
    pid_t pid;
    scanf("%s %d", command, pid);

    //Checking the command in order to take the appropriate action
    if (strcmp(command, LAUNCH) == 0){
      handle_launch_signal();

    } else if (strcmp(command,BOMB) == 0){
      handle_bomb_signal(pid);

    }else if(strcmp(command,REFUEL) == 0){
      handle_refuel_signal(pid);

    } else if(strcmp(command, QUIT)==0){
      kill(0,SIGINT);
      break;

    }else{
      handle_invalid_command;
    }
  }
  return 0;
}

//The function to handle the SIG_BOMB signal
void handle_bomb_signal(pid_t pid){
  if (pid == NULL){
    printf("Error: The bomb commnad requires an argument\n");
    return;
  }
  kill(0, SIGKILL, pid);
}



