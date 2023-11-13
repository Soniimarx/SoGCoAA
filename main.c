#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

#define MAX_PLANES 5

typedef struct
{
    int pid;
    int fuel;
    time_t last_update_time; // New field to track the last update time
    int pipe_fd[2];          // Pipe file descriptors for communication
} Plane;

Plane planes[MAX_PLANES];
int num_planes = 0;
int plane_id;
void launch_plane();
void bomb_plane();
void refuel_plane();

void sig_handler(int signum);

int main()
{
    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler);

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
                kill(planes[i].pid, SIGTERM);
                waitpid(planes[i].pid, NULL, 0);
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
        Plane plane = {.pid = getpid(), .fuel = 100, .last_update_time = time(NULL)}; // Initialize last_update_time

        // Close the write end of the pipe in the child
        close(plane.pipe_fd[1]);

        while (1)
        {
            sleep(1); // Fuel decreases every second
            plane.fuel -= 5;

            // Check if enough time has passed since the last update or the fuel is <= 0
            if ((time(NULL) - plane.last_update_time) >= 3 || plane.fuel <= 0)
            {
                if (plane.fuel <= 0)
                {
                    printf("SoS! Plane %d is going to crash\n", plane.pid);
                    exit(0);
                }

                printf("Bomber %d to base, %d%% of fuel left\n", plane.pid, plane.fuel);

                // Update the last update time
                plane.last_update_time = time(NULL);
            }

            // Check if the child process received a refuel command
            char buffer[64];
            for(int i = 0; i < num_planes; i++){
                if (plane_id == planes[i].pid){
            
                    ssize_t bytesRead = read(planes[i].pipe_fd[1], buffer, sizeof(buffer));

                    if (bytesRead > 0)
                    {
                        buffer[bytesRead] = '\0';

                    // Handle the refuel command
                        if (strcmp(buffer, "refuel") == 0)
                        {
                            // Refuel the plane
                            plane.fuel = 100;
                            printf("Plane %d has been refueled\n", plane.pid);
                        }
                    }
                }
            }
        }
    }
    else if (pid > 0)
    {
        // Parent (base) process
        planes[num_planes++] = (Plane){.pid = pid, .fuel = 100, .last_update_time = time(NULL)}; // Initialize last_update_time

        // Create a pipe for communication
        if (pipe(planes[num_planes - 1].pipe_fd) == -1)
        {
            perror("Failed to create pipe");
            exit(EXIT_FAILURE);
        }

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
        printf("Enter plane ID to bomb: ");
        scanf("%d", &plane_id);

        for (int i = 0; i < num_planes; i++)
        {
            if (planes[i].pid == plane_id)
            {
                if (kill(planes[i].pid, SIGUSR1) == 0)
                {
                    printf("Bomb signal sent to Plane %d!\n", plane_id);
                }
                else
                {
                    perror("Failed to send bomb signal");
                }
                return;
            }
        }
        printf("Invalid plane ID. No such plane exists.\n");
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
        
        printf("Enter plane ID to refuel: ");
        scanf("%d", &plane_id);

        for (int i = 0; i < num_planes; i++)
        {
            if (planes[i].pid == plane_id)
            {
                // Send refuel command to the specific child process using a pipe
                write(planes[i].pipe_fd[1], "refuel", strlen("refuel"));
                kill(planes[i].pid, SIGUSR2);
                return;
            }
        }
        printf("Invalid plane ID. No such plane exists.\n");
    }
    else
    {
        printf("No planes are flying, can't refuel!\n");
    }
}

void sig_handler(int signum)
{
    if (signum == SIGUSR1)
    {
        printf("Bomber %d to base, bombs away!\n", getpid());
    }
    else if (signum == SIGUSR2)
    {
        //
    }
}

