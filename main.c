#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

#define MAX_PLANES 5

// Structure to represent a plane
typedef struct
{
    int pid;                 // Process ID of the plane
    int fuel;                // Remaining fuel of the plane
    time_t last_update_time; // Time of the last fuel update for the plane
} Plane;

Plane planes[MAX_PLANES]; // Array to store plane information
int num_planes = 0;       // Number of currently flying planes
int plane_id;             // Variable to store user-input plane ID

// Function declarations
void launch_plane();          // Function to launch a new plane
void bomb_plane();            // Function to send a bomb signal to a plane
void refuel_plane();          // Function to send a refuel signal to a plane
void sig_handler(int signum); // Signal handler function

int main()
{
    // Set up signal handlers for user-defined signals
    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler);

    // Main program loop
    while (1)
    {
        char command[10];

        // Prompt user for command
        printf("Enter command (launch, bomb, refuel, quit): ");
        scanf("%s", command);

        // Execute the appropriate command based on user input
        if ((strcmp(command, "launch") == 0) || (strcmp(command, "l")==0))
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
        else if ((strcmp(command, "bomb") == 0) ||(strcmp(command, "b")==0))
        {
            bomb_plane();
        }
        else if ((strcmp(command, "refuel") == 0) || (strcmp(command, "r")==0))
        {
            refuel_plane();
        }
        else if ((strcmp(command, "quit") == 0) || (strcmp(command, "q")))
        {
            // Terminate all planes and exit the program
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

// Function to launch a new plane
void launch_plane()
{
    int pid = fork();

    if (pid == 0)
    {
        // Child (plane) process
        Plane plane = {.pid = getpid(), .fuel = 100, .last_update_time = time(NULL)};
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
        }
    }
    else if (pid > 0)
    {
        // Parent (base) process
        planes[num_planes++] = (Plane){.pid = pid, .fuel = 100, .last_update_time = time(NULL)};
        printf("Plane %d launched!\n", pid);
    }
    else
    {
        perror("Failed to fork plane process");
    }
}

// Function to send a bomb signal to a plane
void bomb_plane()
{
    if (num_planes > 0)
    {
        printf("Enter plane ID to bomb: ");
        scanf("%d", &plane_id);

        // Search for the specified plane ID and send a bomb signal
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

// Function to send a refuel signal to a plane
void refuel_plane()
{
    if (num_planes > 0)
    {
        printf("Enter plane ID to refuel: ");
        scanf("%d", &plane_id);

        // Search for the specified plane ID and send a refuel signal
        for (int i = 0; i < num_planes; i++)
        {
            if (planes[i].pid == plane_id)
            {
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

// Signal handler function
void sig_handler(int signum)
{
    // Handle SIGUSR1 signal (bomb signal)
    if (signum == SIGUSR1)
    {
        printf("Bomber %d to base, bombs away!\n", getpid());
    }
    // Handle SIGUSR2 signal (refuel signal)
    else if (signum == SIGUSR2)
    {
        // Refuel the plane with the matching process ID
        for (int i = 0; i < num_planes; i++)
        {
            if (plane_id == planes[i].pid)
            {
                planes[i].fuel = 100;
                printf("Plane %d has been refueled.\n", getpid());
            }
        }
    }
}
