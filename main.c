#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

//Defining the possible commands
#define COMMAND_LAUNCH "L"
#define COMMAND_BOMB "B"
#define COMMAND_REFUEL "R"
#define COMMAND_QUIT "Q"

//defining the signals that will be used to communicate between the parent and child processes.
#define SIGNAL_LAUNCH SIGUSR1
#define SIGNAL_BOMB SIGUSR2
#define SIGNAL_REFUEL SIGINT