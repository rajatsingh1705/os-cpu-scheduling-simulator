/*
 * ============================================================
 *  OS CPU Scheduling Simulator
 *  main.c — Menu-driven entry point
 * ============================================================
 *
 *  Build:  gcc -Wall -Wextra -O2 -std=c11 -o scheduler main.c scheduler.c
 *  Run  :  ./scheduler
 * ============================================================
 */

#include "scheduler.h"

static void printBanner(void) {
    printf("\n");
    printf("  ╔═══════════════════════════════════════════════════════════╗\n");
    printf("  ║       OS CPU SCHEDULING ALGORITHM SIMULATOR               ║\n");
    printf("  ║  FCFS · SJF · SRTF · Round Robin · Priority (NP & P)     ║\n");
    printf("  ╚═══════════════════════════════════════════════════════════╝\n\n");
}

static void printMenu(int hasData) {
    printf("  ┌─────────────────────────────────────────────┐\n");
    printf("  │              MAIN MENU                      │\n");
    printf("  ├─────────────────────────────────────────────┤\n");
    printf("  │  1. Enter / Re-enter Process Data            │\n");
    printf("  ├─────────────────────────────────────────────┤\n");
    if (hasData) {
        printf("  │  2. FCFS (First Come First Serve)            │\n");
        printf("  │  3. SJF Non-Preemptive                       │\n");
        printf("  │  4. SJF Preemptive (SRTF)                    │\n");
        printf("  │  5. Round Robin                              │\n");
        printf("  │  6. Priority Scheduling (Non-Preemptive)     │\n");
        printf("  │  7. Priority Scheduling (Preemptive)         │\n");
        printf("  │  8. Run ALL & Compare                        │\n");
        printf("  ├─────────────────────────────────────────────┤\n");
        printf("  │  9. Show current process data               │\n");
    } else {
        printf("  │  (Enter process data first to see options)   │\n");
    }
    printf("  ├─────────────────────────────────────────────┤\n");
    printf("  │  0. Exit                                     │\n");
    printf("  └─────────────────────────────────────────────┘\n");
    printf("  Choice: ");
}

static void showProcessData(Process procs[], int n) {
    printf("\n  ╔══════════════════════════════════════════════════╗\n");
    printf("  ║  Current Process Data (%d processes)              ║\n", n);
    printf("  ╠════════╦═══════════╦═══════════╦══════════════╣\n");
    printf("  ║  PID   ║  Arrival  ║   Burst   ║   Priority   ║\n");
    printf("  ╠════════╬═══════════╬═══════════╬══════════════╣\n");
    for (int i = 0; i < n; i++) {
        printf("  ║  %-5s ║    %4d   ║    %4d   ║     %4d     ║\n",
               procs[i].pid, procs[i].arrival, procs[i].burst, procs[i].priority);
    }
    printf("  ╚════════╩═══════════╩═══════════╩══════════════╝\n");
}

static int getQuantum(void) {
    int q;
    printf("\n  Enter Time Quantum (1-100): ");
    while (scanf("%d", &q) != 1 || q < 1 || q > 100) {
        printf("  [!] Invalid quantum. Enter a value between 1 and 100: ");
        while (getchar() != '\n');
    }
    return q;
}

static void flushStdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main(void) {
    Process procs[MAX_PROCESSES];
    int n = 0, choice, hasData = 0;

    printBanner();

    while (1) {
        printf("\n");
        printMenu(hasData);

        if (scanf("%d", &choice) != 1) {
            flushStdin();
            printf("  [!] Invalid input. Please enter a number.\n");
            continue;
        }
        flushStdin();

        if (choice == 0) {
            printf("\n  Goodbye! Exiting simulator.\n\n");
            break;
        }

        if (choice == 1) {
            if (inputProcesses(procs, &n)) {
                hasData = 1;
                showProcessData(procs, n);
            } else {
                printf("  [!] Process input failed. Please try again.\n");
                flushStdin();
            }
            continue;
        }

        if (!hasData) {
            printf("  [!] No process data loaded. Please enter process data first (option 1).\n");
            continue;
        }

        switch (choice) {
            case 2:
                printf("\n  Running FCFS...\n");
                fcfs(procs, n);
                break;
            case 3:
                printf("\n  Running SJF (Non-Preemptive)...\n");
                sjf_non_preemptive(procs, n);
                break;
            case 4:
                printf("\n  Running SRTF (SJF Preemptive)...\n");
                sjf_preemptive(procs, n);
                break;
            case 5: {
                int q = getQuantum();
                printf("\n  Running Round Robin (Quantum = %d)...\n", q);
                round_robin(procs, n, q);
                break;
            }
            case 6:
                printf("\n  Running Priority Scheduling (Non-Preemptive)...\n");
                priority_non_preemptive(procs, n);
                break;
            case 7:
                printf("\n  Running Priority Scheduling (Preemptive)...\n");
                priority_preemptive(procs, n);
                break;
            case 8:
                printf("\n  Running ALL algorithms for comparison...\n");
                runAllAndCompare(procs, n);
                break;
            case 9:
                showProcessData(procs, n);
                break;
            default:
                printf("  [!] Invalid choice '%d'. Please select from the menu.\n", choice);
        }
    }

    return 0;
}