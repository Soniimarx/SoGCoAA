#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_PLANES 5

typedef struct
{
  int pid;
  int fuel;
  time_t last_update_time; // New field to track the last update time
} Plane;

Plane planes[MAX_PLANES];
int num_planes = 0;

void launch_plane();
void bomb_plane();
void refuel_plane();
void plane_report(int signum, int plane_index);

void handle_sigusr1(int signum);
void handle_sigusr2(int signum);

int main() {
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);

    while (1) {
        char command[10];

        printf("Enter command (launch, bomb, refuel, quit): ");
        scanf("%s", command);

        if (strcmp(command, "launch") == 0) {
            if (num_planes < MAX_PLANES) {
                launch_plane();
            } else {
                printf("Cannot launch more planes. Maximum limit reached.\n");
            }
        } else if (strcmp(command, "bomb") == 0) {
            bomb_plane();
        } else if (strcmp(command, "refuel") == 0) {
            refuel_plane();
        } else if (strcmp(command, "quit") == 0) {
            // Wait for all planes to finish
            for (int i = 0; i < num_planes; i++) {
                kill(planes[i].pid, SIGTERM);
                waitpid(planes[i].pid, NULL, 0);
            }
            exit(0);
        } else {
            printf("Invalid command. Please enter a valid command.\n");
        }
    }

    return 0;
}

void launch_plane() {
    int pid = fork();

    if (pid == 0) {
        // Child (plane) process
        Plane plane = { .pid = getpid(), .fuel = 100, .last_update_time = time(NULL)};

        while (1) {
            plane_report(SIGALRM, -1);
            sleep(1); // Fuel decreases every second
        }
    } else if (pid > 0) {
        // Parent (base) process
        planes[num_planes++] = (Plane){ .pid = pid, .fuel = 100, .last_update_time = time(NULL) };
        printf("Plane %d launched!\n", pid);
    } else {
        perror("Failed to fork plane process");
    }
}

void bomb_plane() {
    if (num_planes > 0) {
        int plane_id;
        printf("Enter plane ID to bomb: ");
        scanf("%d", &plane_id);

        for (int i = 0; i < num_planes; i++) {
            if (planes[i].pid == plane_id) {
                if (kill(planes[i].pid, SIGUSR1) == 0) {
                    printf("Bomb signal sent to Plane %d!\n", plane_id);
                } else {
                    perror("Failed to send bomb signal");
                }
                return;
            }
        }
        printf("Invalid plane ID. No such plane exists.\n");
    } else {
        printf("No planes are flying, can't drop a bomb!\n");
    }
}

void refuel_plane() {
    if (num_planes > 0) {
        int plane_id;
        printf("Enter plane ID to refuel: ");
        scanf("%d", &plane_id);

        for (int i = 0; i < num_planes; i++) {
            if (planes[i].pid == plane_id) {
                planes[i].fuel = 100;
                printf("Plane %d has been refueled\n", plane_id);
                return;
            }
        }
        printf("Invalid plane ID. No such plane exists.\n");
    } else {
        printf("No planes are flying, can't refuel!\n");
    }
}

void handle_sigusr1(int signum) {
    printf("Bomber %d to base, bombs away!\n", getpid());
}

void handle_sigusr2(int signum) {
    // Refuel signal handler (no action needed here)
}

void plane_report(int signum, int plane_index)
{
  if (signum == SIGALRM)
  {
    time_t current_time = time(NULL);

    for (int i = 0; i < num_planes; i++)
    {
      if (plane_index == -1 || i == plane_index)
      {
        planes[i].fuel -= 5;

        // Check if enough time has passed since the last update
        if (current_time - planes[i].last_update_time >= 3 || planes[i].fuel <= 0)
        {
          if (planes[i].fuel <= 0)
          {
            printf("SoS! Plane %d is going to crash\n", planes[i].pid);
            exit(0);
          }

          printf("Bomber %d to base, %d%% of fuel left\n", planes[i].pid, planes[i].fuel);

          // Update the last update time
          planes[i].last_update_time = current_time;
        }
      }
    }
  }
}
