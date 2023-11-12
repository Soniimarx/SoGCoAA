#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

// Defining the possible commands
#define LAUNCH "L"      \
               "launch" \
               "l"
#define BOMB "B"    \
             "bomb" \
             "b"
#define REFUEL "R"      \
               "refuel" \
               "r"
#define QUIT "Q"    \
             "quit" \
             "q"

// Defining the signals that will be used to communicate between the parent and child processes.
#define SIG_LAUNCH SIGUSR1
#define SIG_BOMB SIGUSR2
#define SIG_REFUEL SIGINT

// Declaring the function prototypes
void handle_launch_signal();
void handle_bomb_signal(pid_t pid);
void handle_refuel_signal(pid_t pid);
void handle_invalid_command();
void spawn_plane_process();

// Checking if the PID is valid
bool is_pid_valid(pid_t pid)
{
  int err = kill(pid, SIG_ZERO);
  return err == 0;
}

// Handling the SIG_BOMB signal
void handle_bomb_signal(pid_t pid)
{
  if (!is_pid_valid(pid))
  {
    printf("Error: The bomb command requires a valid PID\n");
    return;
  }

  kill(pid, SIG_BOMB);
}

// Handling the SIG_REFUEL signal
void handle_refuel_signal(pid_t pid)
{
  if (!is_pid_valid(pid))
  {
    printf("Error: The refuel command requires a valid PID\n");
    return;
  }

  kill(pid, SIG_REFUEL);
}

// Handling the SIG_LAUNCH signal
void handle_launch_signal()
{
  spawn_plane_process();
}

// Handling invalid commands
void handle_invalid_command()
{
  printf("Error: Invalid command.\n");
}

// Spawning a new plane process
void spawn_plane_process()
{
  // Creating a new child process
  pid_t pid = fork();
  if (pid == 0)
  {
    // Child process code
    int fuel_level = 100;
    while (fuel_level > 0)
    {
      fuel_level -= 5;
      sleep(3);
      printf("Bomber %d to base, %d%% of fuel left\n", getpid(), fuel_level);
    }

    // Exit the child process
    exit(0);
  }
  else if (pid > 0)
  {
    // Parent process code
    // Do nothing
  }
  else
  {
    // Error creating a new process
    perror("fork");
    exit(1);
  }
}

// Main function
int main()
{
  // Registering the signal handlers
  signal(SIG_LAUNCH, handle_launch_signal);
  signal(SIG_BOMB, handle_bomb_signal);
  signal(SIG_REFUEL, handle_refuel_signal);

  // Main loop
  while (1)
  {
    // Prompting the user for a command
    printf("Enter a command: ");
    char command[10];
    pid_t pid;
    scanf("%s %d", command, &pid);

    // Checking the command
    if (strcmp(command, LAUNCH) == 0)
    {
      handle_launch_signal();
    }
    else if (strcmp(command, BOMB) == 0)
    {
      handle_bomb_signal(pid);
    }
    else if (strcmp(command, REFUEL) == 0)
    {
      handle_refuel_signal(pid);
    }
    else if (strcmp(command, QUIT) == 0)
    {
      kill(0, SIGINT);
      break;
    }
    else
    {
      handle_invalid_command();
    }
  }

  return 0;
}
