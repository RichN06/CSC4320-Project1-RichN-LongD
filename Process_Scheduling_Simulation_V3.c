#include <stdio.h>

#define MAX 100
#define MEMORY_SIZE 100
#define MAX_BLOCKS  20

// All Processes information
struct Process {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;

    int start_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;

    int mem_size;
};

// Memory Implementation for process int size and int free (if block is free to be used)
struct Block {
    int size;
    int free;
};

struct Block blocks[MAX_BLOCKS];
int blockCount = 0;

// Initializing process blocks memory info
void init_memory() {
    blockCount = 1;
    blocks[0].size = MEMORY_SIZE;
    blocks[0].free = 1;
}

// For each time a process is scanned and allocated in a memory, print the block number, size, and whether that block is USED or FREE.
void print_memory() {
    printf("\nMemory Blocks:\n");
    for (int i = 0; i < blockCount; i++) {
        printf("Block %d: Size=%d, %s\n",
               i,
               blocks[i].size,
               blocks[i].free ? "FREE" : "USED");
    }
}

// Function for splitting blocks for each process memory allocation if there are more than one process and requesting the block size of each process needed.
// if you request a size lesser than the maximum size for a block,
// it will take out whatever n amount from that max amount and split into 2 blocks which will be a USED and a FREE block (FREE to be used by a next process if there are more than one process)
void split_block(int index, int reqSize) {
    // Case where a requested size equals to the block size, whether that is the first block or so on blocks, no further splitting is required.
    if (blocks[index].size == reqSize) {
        blocks[index].free = 0;
        return;
    }

    // Case where a requested size is lesser than the maximum block size, splitting occurs
    // The for loop is only used when there is a FREE block somewhere in between blocks and we would need to shift the USED block to the right in order to split
    for (int i = blockCount; i > index + 1; i--) {
        blocks[i] = blocks[i - 1];
    }
    blockCount++;

    blocks[index + 1].size = blocks[index].size - reqSize;  // i=0 : blocks[1].size = blocks[0].size (100) - reqSize (n)
    blocks[index + 1].free = 1;                             // i=0 : blocks[1] is FREE

    blocks[index].size = reqSize;                           // i=0 : blocks[0] = reqSize (n)
    blocks[index].free = 0;                                 // i=0 : blocks[0] is USED
}

// ------> FIRST-FIT MEMORY ALLOCATION <------
int first_fit(int reqSize) {
    for (int i = 0; i < blockCount; i++) {
        if (blocks[i].free && blocks[i].size >= reqSize) {
            split_block(i, reqSize);
            return i;
        }
    }
    return -1;
}

int readProcesses(struct Process p[]);
void fcfs(struct Process p[], int n);
void sjf(struct Process p[], int n);

// ------> MAIN FUNCTION <------
int main() {
    struct Process processes[MAX];   // Array to store all processes
    int n;                           // Number of processes read from processes.txt file

    int choice = 0;                  // User's input choice

    n = readProcesses(processes);    // Read the processes.txt file and return the # of processes there are

    // if there are no processes
    if (n == 0) {
        printf("No processes found.\n");
        return 1;
    }

    // User's input for memory size required for First-Fit allocation
    printf("\nEnter memory size for each process (for First-Fit allocation):\n");
    for (int i = 0; i < n; i++) {
        printf("Process P%d memory size: ", processes[i].pid);
        scanf("%d", &processes[i].mem_size);
    }

    init_memory();

    // User's input for printing options
    while (choice != 5) {
        printf("\n--------------------> MENU <--------------------\n");
        printf("1) Allocate memory for all processes (First-Fit)\n");
        printf("2) Print current memory blocks\n");
        printf("3) Run FCFS scheduling\n");
        printf("4) Run SJF scheduling\n");
        printf("5) Exit program\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            printf("\n--- First-Fit Memory Allocation ---\n");

            for (int i = 0; i < n; i++) {
                int blockIndex = first_fit(processes[i].mem_size);

                if (blockIndex == -1) {
                    printf("P%d allocation FAILED (not enough free space)\n",
                           processes[i].pid);
                }
                else {
                    printf("P%d allocated in Block %d\n",
                           processes[i].pid, blockIndex);
                }
            }
            print_memory();
        }
        else if (choice == 2) {
            print_memory();
        }
        else if (choice == 3) {
            fcfs(processes, n);
        }
        else if (choice == 4) {
            sjf(processes, n);
        }
        else if (choice == 5) {
            printf("Exiting program...\n");
        }
        else {
            printf("Invalid choice. Please enter 1-5.\n");
        }
    }
    return 0;
}

// ------> READING THE PROCESS FILE <------
int readProcesses(struct Process p[]) {
    FILE *fp;
    int count = 0;

    fp = fopen("processes.txt", "r");

    // if file doesn't exist or not in folder
    if (fp == NULL) {
        printf("Error opening file.\n");
        return 0;
    }

    // Ignore header (PID	Arrival_Time	Burst_Time	Priority)
    fscanf(fp, "%*[^\n]\n");

    while (fscanf(fp, "%d %d %d %d",
                  &p[count].pid,
                  &p[count].arrival_time,
                  &p[count].burst_time,
                  &p[count].priority) != EOF) {
        count++;
    }
    fclose(fp);
    return count;
}

// ------> FIRST-COME, FIRST-SERVE SCHEDULING ALGORITHM (FCFS) <------
// ------> The first process that arrives runs first <------
void fcfs(struct Process p[], int n) {
    // temporary copy of Struct processes
    struct Process temp[MAX];

    int time = 0;
    float total_wt = 0, total_tat = 0;

    // Area to copy each info of corresponding processes
    for (int i = 0; i < n; i++) {
        temp[i] = p[i];
    }

    // Sort by arrival time
    // ( Simple Selection Sort Algorithm : finds smaller element then immediately swap rather than finding smallest then swap )
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (p[i].arrival_time > p[j].arrival_time) {
                struct Process temp = p[i];
                p[i] = p[j];
                p[j] = temp;
            }
        }
    }

    printf("\n------> FCFS Scheduling <------\n");

    // ------> GANTT CHART <------
    printf("Gantt Chart:\n|");
    // The for loop is printing | P1 | P2 | P3 | while also calculating the time completion for later calculations
    for (int i = 0; i < n; i++) {
        if (time < temp[i].arrival_time)    // i=0 : time = 0 < temp[0] = AT 0 --> i=1 : time = 0 < temp[1] = AT 2
            time = temp[i].arrival_time;    // i=0 : time = (temp[i] = AT 0)   --> i=1 : time = (temp[1] = AT 2)

        temp[i].start_time = time;      // i=0 : start_time = time 0    --> i=1 : start_time = time 2
        time += temp[i].burst_time;     // i=0 : time 0 += BT 5         --> i=1 : time 5 += BT 3
        temp[i].completion_time = time; // i=0 : complete_time = time 5 --> i=1 : complete_time = time 8

        printf("  P%d   |", temp[i].pid);
    }

    // Timeline of FCFS burst times --> 0   5   8   10 <--
    printf("\n");
    time = 0;   // Setting time back to 0

    // Now we're just printing the arrival time in order with each burst time of each process added together over time
    for (int i = 0; i < n; i++) {
        if (time < temp[i].arrival_time)
            time = temp[i].arrival_time;
        printf("%d\t", time);
        time += temp[i].burst_time;
    }
    printf("%d\n", time);   // Print the final time which is 10

    printf("\n---------> PROCESS TABLE <---------");
    printf("\nPID\tAT\tBT\tWT\tTAT\n");

    for (int i = 0; i < n; i++) {
        temp[i].turnaround_time =
            temp[i].completion_time - temp[i].arrival_time;
        temp[i].waiting_time =
            temp[i].turnaround_time - temp[i].burst_time;

        total_wt += temp[i].waiting_time;
        total_tat += temp[i].turnaround_time;

        printf("%d\t%d\t%d\t%d\t%d\n",
               temp[i].pid,
               temp[i].arrival_time,
               temp[i].burst_time,
               temp[i].waiting_time,
               temp[i].turnaround_time);
    }
    printf("\nAverage Waiting Time = %.2f",
           total_wt / n);
    printf("\nAverage Turnaround Time = %.2f\n",
           total_tat / n);
}

// ------> SHORTEST JOB FIRST SCHEDULING ALGORITHM (SJF) <------
// ------> The process with the smallest burst time runs first <------
void sjf(struct Process p[], int n) {
    // temporary copy of Struct processes
    struct Process temp[MAX];

    int completed = 0, time = 0;
    int is_done[100] = {0};
    float total_wt = 0, total_tat = 0;

    // Area to copy each info of corresponding processes
    for (int i = 0; i < n; i++)
        temp[i] = p[i];

    printf("\n------> SJF Scheduling (Non-Preemptive) <------\n");

    // ------> GANTT CHART <------
    printf("Gantt Chart:\n|");

    int gantt_start[MAX];   // Array to store start time    ---> [0, 5, 7]
    int gantt_end[MAX];     // Array to store end time      ---> [5, 7, 10]
    int g = 0;              // Number of indexes for how many blocks there are

    while (completed < n) {
        int idx = -1;
        int min_bt = 99999;

        // This for loop iterates through each processes and compares current process arrival time with the current time of
        // the cpu and making sure that this process is not yet done (the {0,0,0}, if one process is done then the 0 corresponding to the process index is set to 1)
        // 2nd if statement checks for minimal burst time. If the condition is true, we set min_bt as new BT and save the process index for later usage.
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival_time <= time && !is_done[i]) {      // i=0 : AT[0]=0  <=  Time=0 && {0,0,0} | i=1 : AT[1]=2  <=  Time=5 && {1,0,0} | i=2 : AT[2]=4  <=  Time=5 && {1,0,0}
                                                                            // At time 0, CPU can only choose a process with an arrival time of 0 which is P1,
                                                                            // after P1, time becomes 5 which the loop can iterate through last 2 processes and find which has the smaller BT
                                                                            // which in this case is P3.
                if (temp[i].burst_time < min_bt) {                  // i=0 : BT[0]=5  <  min_bt=99999
                    min_bt = temp[i].burst_time;                    // i=0 : min_bt = 5
                    idx = i;                                        // i=0 : idx = 0
                }
            }
        }

        if (idx == -1) {
            time++;
        }
        else {
            gantt_start[g] = time;              // i=0 : Ganntt_Start = 0
            time += temp[idx].burst_time;       // i=0 : Time += temp[0].burst_time=5 = 5
            gantt_end[g] = time;                // i=0 : Time_end = 5
            temp[idx].completion_time = time;   // i=0 : Completion_time = 5

            temp[idx].turnaround_time =         // i=0 : TAT = 5 - temp[0].arrival_time=0 --> 5 - 0 = 5
                time - temp[idx].arrival_time;

            temp[idx].waiting_time =            // i=0 : WT = TAT (5) - BT (5) = 0
                temp[idx].turnaround_time - temp[idx].burst_time;

            total_wt += temp[idx].waiting_time;     // i=0 : Total WT = 0

            total_tat += temp[idx].turnaround_time; // i=0 : Total TAT = 5

            is_done[idx] = 1;       // i=0 : {1,0,0}
            completed++;            // i=0 : int completed = 1
            g++;                    // i=0 : int g = 1

            printf("  P%d   |", temp[idx].pid);    // | P1 | P3 | P2 | in burst time order
        }
    }

    // Timeline of SJF burst times --> 0   5   7   10 <--
    printf("\n");
    for (int i = 0; i < g; i++)
        printf("%d\t", gantt_start[i]);
    printf("%d\n", gantt_end[g - 1]);

    printf("\n---------> PROCESS TABLE <---------");
    printf("\nPID\tAT\tBT\tWT\tTAT\n");

    for (int i = 0; i < n; i++) {
        printf("%d\t%d\t%d\t%d\t%d\n",
               temp[i].pid,
               temp[i].arrival_time,
               temp[i].burst_time,
               temp[i].waiting_time,
               temp[i].turnaround_time);
    }
    printf("\nAverage Waiting Time = %.2f",
           total_wt / n);
    printf("\nAverage Turnaround Time = %.2f\n",
           total_tat / n);
}