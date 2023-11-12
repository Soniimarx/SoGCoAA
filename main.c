#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_PLANES 5

int planes[MAX_PLANES];
int num_planes = 0;

void launch_plane();
void bomb_plane();
void refuel_plane();
void plane_report(int signum);

void handle_sigusr1(int signum);
void handle_sigusr2(int signum);

int main()
{
  signal(SIGUSR1, handle_sigusr1);
  signal(SIGUSR2, handle_sigusr2);

  while (1)
  {
    char command[10];

    printf("Enter command (launch, bomb, refuel, quit): ");
    scanf("%s", command);

    if (strcmp(command, "launch") == 0)
    {
      if (num_planes < MAX_PLANES)
      {
        launch_plane();
      }
      else
      {
        printf("Cannot launch more planes. Maximum limit reached.\n");
      }
    }
    else if (strcmp(command, "bomb") == 0)
    {
      bomb_plane();
    }
    else if (strcmp(command, "refuel") == 0)
    {
      refuel_plane();
    }
    else if (strcmp(command, "quit") == 0)
    {
      // Wait for all planes to finish
      for (int i = 0; i < num_planes; i++)
      {
        kill(planes[i], SIGTERM);
        waitpid(planes[i], NULL, 0);
      }
      exit(0);
    }
    else
    {
      printf("Invalid command. Please enter a valid command.\n");
    }
  }

  return 0;
}

void launch_plane()
{
  int pid = fork();

  if (pid == 0)
  {
    // Child (plane) process
    while (1)
    {
      sleep(3); // Fuel decreases every second
      plane_report(SIGALRM);
    }
  }
  else if (pid > 0)
  {
    // Parent (base) process
    planes[num_planes++] = pid;
    printf("Plane %d launched!\n", pid);
  }
  else
  {
    perror("Failed to fork plane process");
  }
}

void bomb_plane()
{
  if (num_planes > 0)
  {
    int plane_id;
    printf("Enter plane ID to bomb: ");
    scanf("%d", &plane_id);

    if (kill(plane_id, SIGUSR1) == 0)
    {
      printf("Bomb signal sent to Plane %d!\n", plane_id);
    }
    else
    {
      perror("Failed to send bomb signal");
    }
  }
  else
  {
    printf("No planes are flying, can't drop a bomb!\n");
  }
}

void refuel_plane()
{
  if (num_planes > 0)
  {
    int plane_id;
    printf("Enter plane ID to refuel: ");
    scanf("%d", &plane_id);

    if (kill(plane_id, SIGUSR2) == 0)
    {
      printf("Refuel signal sent to Plane %d!\n", plane_id);
    }
    else
    {
      perror("Failed to send refuel signal");
    }
  }
  else
  {
    printf("No planes are flying, can't refuel!\n");
  }
}

void handle_sigusr1(int signum)
{
  printf("Bomber %d to base, bombs away!\n", getpid());
}

void handle_sigusr2(int signum)
{
  printf("Plane %d has been refueled\n", getpid());
}

void plane_report(int signum)
{
  static int fuel = 100;

  if (signum == SIGALRM)
  {
    fuel -= 5;
    if (fuel <= 0)
    {
      printf("SoS! Plane %d is going to crash\n", getpid());
      exit(0);
    }

    if (fuel % 15 == 0)
    {
      printf("Bomber %d to base, %d%% of fuel left\n", getpid(), fuel);
    }
  }
}
