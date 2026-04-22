/*
 * ============================================================
 *  OS CPU Scheduling Simulator
 *  scheduler.c — Implementation of all scheduling algorithms
 * ============================================================
 */

#include "scheduler.h"

/* ═══════════════════════════════════════════════════════════
 *  HELPER UTILITIES
 * ═══════════════════════════════════════════════════════════ */

/* Deep-copy n processes from src to dst */
void copyProcesses(const Process src[], Process dst[], int n) {
    for (int i = 0; i < n; i++) dst[i] = src[i];
}

/* Reset per-run fields so the same data can be re-used */
void resetProcesses(Process procs[], int n) {
    for (int i = 0; i < n; i++) {
        procs[i].remaining  = procs[i].burst;
        procs[i].completion = 0;
        procs[i].waiting    = 0;
        procs[i].turnaround = 0;
        procs[i].started    = 0;
    }
}

/* Returns 1 when every process has finished */
int allDone(const Process procs[], int n) {
    for (int i = 0; i < n; i++)
        if (procs[i].remaining > 0) return 0;
    return 1;
}

/* Validate integer input within [min, max] */
int validateInput(int val, int min, int max, const char *field) {
    if (val < min || val > max) {
        printf("  [!] %s must be between %d and %d. Got %d.\n",
               field, min, max, val);
        return 0;
    }
    return 1;
}

/* Comparison helpers for qsort */
static int cmpArrival(const void *a, const void *b) {
    const Process *pa = (const Process *)a;
    const Process *pb = (const Process *)b;
    if (pa->arrival != pb->arrival) return pa->arrival - pb->arrival;
    return strcmp(pa->pid, pb->pid); /* tie-break by PID */
}

__attribute__((unused))
static int cmpBurst(const void *a, const void *b) {
    const Process *pa = (const Process *)a;
    const Process *pb = (const Process *)b;
    if (pa->burst != pb->burst) return pa->burst - pb->burst;
    return pa->arrival - pb->arrival;
}

/* ═══════════════════════════════════════════════════════════
 *  INPUT / OUTPUT
 * ═══════════════════════════════════════════════════════════ */

/*
 * inputProcesses()
 * Prompts the user for process details.
 * Returns 1 on success, 0 on failure.
 */
int inputProcesses(Process procs[], int *n) {
    printf("\n  Enter number of processes (1-%d): ", MAX_PROCESSES);
    if (scanf("%d", n) != 1 || !validateInput(*n, 1, MAX_PROCESSES, "process count"))
        return 0;

    printf("\n  %-6s %-12s %-12s %-10s\n", "PID", "Arrival", "Burst", "Priority");
    printf("  %s\n", "──────────────────────────────────────────");

    for (int i = 0; i < *n; i++) {
        snprintf(procs[i].pid, MAX_LABEL_LEN, "P%d", i + 1);

        printf("\n  Process %s\n", procs[i].pid);

        printf("    Arrival Time : ");
        if (scanf("%d", &procs[i].arrival) != 1 ||
            !validateInput(procs[i].arrival, 0, 9999, "arrival time"))
            return 0;

        printf("    Burst Time   : ");
        if (scanf("%d", &procs[i].burst) != 1 ||
            !validateInput(procs[i].burst, 1, 9999, "burst time"))
            return 0;

        printf("    Priority     : (1=highest) ");
        if (scanf("%d", &procs[i].priority) != 1 ||
            !validateInput(procs[i].priority, 1, 9999, "priority"))
            return 0;

        procs[i].remaining  = procs[i].burst;
        procs[i].completion = 0;
        procs[i].waiting    = 0;
        procs[i].turnaround = 0;
        procs[i].started    = 0;
    }

    /* Sort by arrival time after input */
    qsort(procs, *n, sizeof(Process), cmpArrival);

    printf("\n  ✓ %d process(es) entered and sorted by arrival time.\n", *n);
    return 1;
}

/*
 * printTable()
 * Prints a formatted process results table.
 */
void printTable(Process procs[], int n, const char *algoName) {
    printf("\n  ┌───────────────────────────────────────────────────────────────────┐\n");
    printf("  │  Algorithm : %-54s│\n", algoName);
    printf("  ├──────┬─────────┬───────┬────────────┬─────────┬────────────────┤\n");
    printf("  │ PID  │ Arrival │ Burst │ Completion │ Waiting │  Turnaround    │\n");
    printf("  ├──────┼─────────┼───────┼────────────┼─────────┼────────────────┤\n");

    for (int i = 0; i < n; i++) {
        printf("  │ %-4s │   %4d  │  %4d │    %5d   │   %4d  │      %5d      │\n",
               procs[i].pid,
               procs[i].arrival,
               procs[i].burst,
               procs[i].completion,
               procs[i].waiting,
               procs[i].turnaround);
    }

    printf("  └──────┴─────────┴───────┴────────────┴─────────┴────────────────┘\n");
    printAverages(procs, n);
}

/*
 * printAverages()
 * Calculates and prints average waiting and turnaround times.
 */
void printAverages(Process procs[], int n) {
    double sumWT = 0, sumTAT = 0;
    for (int i = 0; i < n; i++) {
        sumWT  += procs[i].waiting;
        sumTAT += procs[i].turnaround;
    }
    printf("  Average Waiting Time    : %.2f\n", sumWT  / n);
    printf("  Average Turnaround Time : %.2f\n", sumTAT / n);
}

/*
 * printGanttChart()
 * Renders a Gantt chart from an array of GanttSegments.
 *
 *  | P1 | P2 | IDLE | P3 |
 *  0    3    7      9   14
 */
void printGanttChart(GanttSegment gantt[], int gLen) {
    if (gLen == 0) return;

    printf("\n  Gantt Chart:\n  ");

    /* Top border */
    for (int i = 0; i < gLen; i++) {
        int width    = gantt[i].end - gantt[i].start;
        int boxWidth = (width < 3) ? 5 : width * 2 + 1;
        for (int j = 0; j < boxWidth; j++) printf("─");
        printf("─");
    }
    printf("\n  |");

    /* Process labels — centered in their slot */
    for (int i = 0; i < gLen; i++) {
        int width    = gantt[i].end - gantt[i].start;
        int boxWidth = (width < 3) ? 5 : width * 2 + 1;
        int labelLen = (int)strlen(gantt[i].label);
        int padding  = (boxWidth - labelLen) / 2;
        int extra    = (boxWidth - labelLen) % 2;
        for (int j = 0; j < padding + extra; j++) printf(" ");
        printf("%s", gantt[i].label);
        for (int j = 0; j < padding; j++) printf(" ");
        printf("|");
    }

    /* Bottom border */
    printf("\n  ");
    for (int i = 0; i < gLen; i++) {
        int width    = gantt[i].end - gantt[i].start;
        int boxWidth = (width < 3) ? 5 : width * 2 + 1;
        for (int j = 0; j < boxWidth; j++) printf("─");
        printf("─");
    }

    /* Timeline numbers */
    printf("\n  ");
    int cursor = 0;
    printf("%-d", gantt[0].start);
    cursor = (int)snprintf(NULL, 0, "%d", gantt[0].start);

    for (int i = 0; i < gLen; i++) {
        int width    = gantt[i].end - gantt[i].start;
        int boxWidth = (width < 3) ? 5 : width * 2 + 1;
        int numLen   = (int)snprintf(NULL, 0, "%d", gantt[i].end);
        int spaces   = boxWidth + 1 - cursor + 1 - numLen;
        if (spaces < 1) spaces = 1;
        for (int j = 0; j < spaces; j++) printf(" ");
        printf("%d", gantt[i].end);
        cursor = numLen;
    }
    printf("\n");
}

/* ═══════════════════════════════════════════════════════════
 *  ALGORITHM 1 — FCFS (First Come First Serve)
 * ═══════════════════════════════════════════════════════════
 *
 * Non-preemptive. Processes run in order of arrival.
 * If CPU is free before next arrival, simulate IDLE time.
 */
void fcfs(Process procs[], int n) {
    Process p[MAX_PROCESSES];
    copyProcesses(procs, p, n);
    resetProcesses(p, n);
    qsort(p, n, sizeof(Process), cmpArrival);

    GanttSegment gantt[MAX_GANTT];
    int gLen = 0, time = 0;

    for (int i = 0; i < n; i++) {
        /* CPU is idle until the next process arrives */
        if (time < p[i].arrival) {
            snprintf(gantt[gLen].label, MAX_LABEL_LEN, "IDLE");
            gantt[gLen].start = time;
            gantt[gLen].end   = p[i].arrival;
            gLen++;
            time = p[i].arrival;
        }

        snprintf(gantt[gLen].label, MAX_LABEL_LEN, "%s", p[i].pid);
        gantt[gLen].start = time;
        time             += p[i].burst;
        gantt[gLen].end   = time;
        gLen++;

        p[i].completion = time;
        p[i].turnaround = p[i].completion - p[i].arrival;
        p[i].waiting    = p[i].turnaround - p[i].burst;
    }

    printGanttChart(gantt, gLen);
    printTable(p, n, "FCFS (First Come First Serve)");
}

/* ═══════════════════════════════════════════════════════════
 *  ALGORITHM 2 — SJF Non-Preemptive
 * ═══════════════════════════════════════════════════════════
 *
 * At each selection point, pick the arrived process with the
 * smallest burst time. Once running, it cannot be preempted.
 */
void sjf_non_preemptive(Process procs[], int n) {
    Process p[MAX_PROCESSES];
    copyProcesses(procs, p, n);
    resetProcesses(p, n);

    GanttSegment gantt[MAX_GANTT];
    int gLen = 0, time = 0, done = 0;
    int finished[MAX_PROCESSES] = {0};

    while (done < n) {
        /* Find the shortest arrived, unfinished process */
        int sel = -1;
        for (int i = 0; i < n; i++) {
            if (!finished[i] && p[i].arrival <= time) {
                if (sel == -1 || p[i].burst < p[sel].burst)
                    sel = i;
            }
        }

        if (sel == -1) {
            /* Advance clock to nearest future arrival */
            int nearest = -1;
            for (int i = 0; i < n; i++)
                if (!finished[i] && (nearest == -1 || p[i].arrival < p[nearest].arrival))
                    nearest = i;

            snprintf(gantt[gLen].label, MAX_LABEL_LEN, "IDLE");
            gantt[gLen].start = time;
            gantt[gLen].end   = p[nearest].arrival;
            gLen++;
            time = p[nearest].arrival;
            continue;
        }

        snprintf(gantt[gLen].label, MAX_LABEL_LEN, "%s", p[sel].pid);
        gantt[gLen].start = time;
        time             += p[sel].burst;
        gantt[gLen].end   = time;
        gLen++;

        p[sel].completion = time;
        p[sel].turnaround = p[sel].completion - p[sel].arrival;
        p[sel].waiting    = p[sel].turnaround - p[sel].burst;
        finished[sel]     = 1;
        done++;
    }

    printGanttChart(gantt, gLen);
    printTable(p, n, "SJF Non-Preemptive");
}

/* ═══════════════════════════════════════════════════════════
 *  ALGORITHM 3 — SJF Preemptive (SRTF)
 * ═══════════════════════════════════════════════════════════
 *
 * At every clock tick, check if a newly arrived process has
 * a shorter remaining time than the current one.
 * If so, preempt and switch.
 */
void sjf_preemptive(Process procs[], int n) {
    Process p[MAX_PROCESSES];
    copyProcesses(procs, p, n);
    resetProcesses(p, n);

    GanttSegment gantt[MAX_GANTT];
    int gLen = 0, time = 0, done = 0;
    int prevPID = -1;

    while (done < n) {
        /* Pick process with shortest remaining time that has arrived */
        int sel = -1;
        for (int i = 0; i < n; i++) {
            if (p[i].remaining > 0 && p[i].arrival <= time) {
                if (sel == -1 || p[i].remaining < p[sel].remaining)
                    sel = i;
            }
        }

        if (sel == -1) {
            /* CPU idle — jump to next arrival */
            int nearest = -1;
            for (int i = 0; i < n; i++)
                if (p[i].remaining > 0 && (nearest == -1 || p[i].arrival < p[nearest].arrival))
                    nearest = i;

            if (gLen > 0 && strcmp(gantt[gLen-1].label, "IDLE") == 0) {
                gantt[gLen-1].end = p[nearest].arrival;
            } else {
                snprintf(gantt[gLen].label, MAX_LABEL_LEN, "IDLE");
                gantt[gLen].start = time;
                gantt[gLen].end   = p[nearest].arrival;
                gLen++;
            }
            time = p[nearest].arrival;
            prevPID = -1;
            continue;
        }

        /* Extend last Gantt segment or start a new one */
        if (prevPID == sel && gLen > 0) {
            gantt[gLen-1].end = time + 1;
        } else {
            snprintf(gantt[gLen].label, MAX_LABEL_LEN, "%s", p[sel].pid);
            gantt[gLen].start = time;
            gantt[gLen].end   = time + 1;
            gLen++;
        }

        p[sel].remaining--;
        prevPID = sel;
        time++;

        if (p[sel].remaining == 0) {
            p[sel].completion = time;
            p[sel].turnaround = p[sel].completion - p[sel].arrival;
            p[sel].waiting    = p[sel].turnaround - p[sel].burst;
            done++;
        }
    }

    printGanttChart(gantt, gLen);
    printTable(p, n, "SJF Preemptive (SRTF — Shortest Remaining Time First)");
}

/* ═══════════════════════════════════════════════════════════
 *  ALGORITHM 4 — Round Robin
 * ═══════════════════════════════════════════════════════════
 *
 * Circular queue. Each process gets at most `quantum` units.
 * If a process uses its full quantum and still has remaining
 * time, it re-enters the back of the queue.
 */
void round_robin(Process procs[], int n, int quantum) {
    Process p[MAX_PROCESSES];
    copyProcesses(procs, p, n);
    resetProcesses(p, n);

    int queue[MAX_PROCESSES * MAX_GANTT];
    int front = 0, rear = 0;

    GanttSegment gantt[MAX_GANTT];
    int gLen = 0, time = 0, done = 0;
    int inQueue[MAX_PROCESSES] = {0};

    /* Enqueue all processes that arrive at time 0 */
    for (int i = 0; i < n; i++) {
        if (p[i].arrival == 0) {
            queue[rear++] = i;
            inQueue[i] = 1;
        }
    }

    while (done < n) {
        if (front == rear) {
            /* Queue empty — advance to next arrival */
            int nearest = -1;
            for (int i = 0; i < n; i++)
                if (p[i].remaining > 0 && (nearest == -1 || p[i].arrival < p[nearest].arrival))
                    nearest = i;

            snprintf(gantt[gLen].label, MAX_LABEL_LEN, "IDLE");
            gantt[gLen].start = time;
            gantt[gLen].end   = p[nearest].arrival;
            gLen++;
            time = p[nearest].arrival;

            for (int i = 0; i < n; i++)
                if (!inQueue[i] && p[i].remaining > 0 && p[i].arrival <= time) {
                    queue[rear++] = i;
                    inQueue[i] = 1;
                }
            continue;
        }

        int cur = queue[front++];
        inQueue[cur] = 0;

        int execTime = (p[cur].remaining < quantum) ? p[cur].remaining : quantum;

        snprintf(gantt[gLen].label, MAX_LABEL_LEN, "%s", p[cur].pid);
        gantt[gLen].start = time;
        gantt[gLen].end   = time + execTime;
        gLen++;

        p[cur].remaining -= execTime;
        time             += execTime;

        /* Enqueue newly-arrived processes BEFORE re-enqueueing current */
        for (int i = 0; i < n; i++)
            if (!inQueue[i] && p[i].remaining > 0 && p[i].arrival <= time && i != cur) {
                queue[rear++] = i;
                inQueue[i] = 1;
            }

        if (p[cur].remaining == 0) {
            p[cur].completion = time;
            p[cur].turnaround = p[cur].completion - p[cur].arrival;
            p[cur].waiting    = p[cur].turnaround - p[cur].burst;
            done++;
        } else {
            queue[rear++] = cur;
            inQueue[cur]  = 1;
        }
    }

    char algoLabel[64];
    snprintf(algoLabel, sizeof(algoLabel), "Round Robin (Quantum = %d)", quantum);
    printGanttChart(gantt, gLen);
    printTable(p, n, algoLabel);
}

/* ═══════════════════════════════════════════════════════════
 *  ALGORITHM 5 — Priority Scheduling (Non-Preemptive)
 * ═══════════════════════════════════════════════════════════
 *
 * At each selection point, pick the arrived process with the
 * lowest priority number (= highest priority). Non-preemptive.
 */
void priority_non_preemptive(Process procs[], int n) {
    Process p[MAX_PROCESSES];
    copyProcesses(procs, p, n);
    resetProcesses(p, n);

    GanttSegment gantt[MAX_GANTT];
    int gLen = 0, time = 0, done = 0;
    int finished[MAX_PROCESSES] = {0};

    while (done < n) {
        int sel = -1;
        for (int i = 0; i < n; i++) {
            if (!finished[i] && p[i].arrival <= time) {
                if (sel == -1 || p[i].priority < p[sel].priority ||
                   (p[i].priority == p[sel].priority && p[i].arrival < p[sel].arrival))
                    sel = i;
            }
        }

        if (sel == -1) {
            int nearest = -1;
            for (int i = 0; i < n; i++)
                if (!finished[i] && (nearest == -1 || p[i].arrival < p[nearest].arrival))
                    nearest = i;

            snprintf(gantt[gLen].label, MAX_LABEL_LEN, "IDLE");
            gantt[gLen].start = time;
            gantt[gLen].end   = p[nearest].arrival;
            gLen++;
            time = p[nearest].arrival;
            continue;
        }

        snprintf(gantt[gLen].label, MAX_LABEL_LEN, "%s", p[sel].pid);
        gantt[gLen].start = time;
        time             += p[sel].burst;
        gantt[gLen].end   = time;
        gLen++;

        p[sel].completion = time;
        p[sel].turnaround = p[sel].completion - p[sel].arrival;
        p[sel].waiting    = p[sel].turnaround - p[sel].burst;
        finished[sel]     = 1;
        done++;
    }

    printGanttChart(gantt, gLen);
    printTable(p, n, "Priority Scheduling Non-Preemptive");
}

/* ═══════════════════════════════════════════════════════════
 *  ALGORITHM 6 — Priority Scheduling (Preemptive)
 * ═══════════════════════════════════════════════════════════
 *
 * At every clock tick, if a newly arrived process has a
 * higher priority (lower number) than the running one,
 * preempt immediately.
 */
void priority_preemptive(Process procs[], int n) {
    Process p[MAX_PROCESSES];
    copyProcesses(procs, p, n);
    resetProcesses(p, n);

    GanttSegment gantt[MAX_GANTT];
    int gLen = 0, time = 0, done = 0;
    int prevPID = -1;

    while (done < n) {
        int sel = -1;
        for (int i = 0; i < n; i++) {
            if (p[i].remaining > 0 && p[i].arrival <= time) {
                if (sel == -1 || p[i].priority < p[sel].priority ||
                   (p[i].priority == p[sel].priority && p[i].arrival < p[sel].arrival))
                    sel = i;
            }
        }

        if (sel == -1) {
            int nearest = -1;
            for (int i = 0; i < n; i++)
                if (p[i].remaining > 0 && (nearest == -1 || p[i].arrival < p[nearest].arrival))
                    nearest = i;

            if (gLen > 0 && strcmp(gantt[gLen-1].label, "IDLE") == 0) {
                gantt[gLen-1].end = p[nearest].arrival;
            } else {
                snprintf(gantt[gLen].label, MAX_LABEL_LEN, "IDLE");
                gantt[gLen].start = time;
                gantt[gLen].end   = p[nearest].arrival;
                gLen++;
            }
            time = p[nearest].arrival;
            prevPID = -1;
            continue;
        }

        if (prevPID == sel && gLen > 0) {
            gantt[gLen-1].end = time + 1;
        } else {
            snprintf(gantt[gLen].label, MAX_LABEL_LEN, "%s", p[sel].pid);
            gantt[gLen].start = time;
            gantt[gLen].end   = time + 1;
            gLen++;
        }

        p[sel].remaining--;
        prevPID = sel;
        time++;

        if (p[sel].remaining == 0) {
            p[sel].completion = time;
            p[sel].turnaround = p[sel].completion - p[sel].arrival;
            p[sel].waiting    = p[sel].turnaround - p[sel].burst;
            done++;
        }
    }

    printGanttChart(gantt, gLen);
    printTable(p, n, "Priority Scheduling Preemptive");
}

/* ═══════════════════════════════════════════════════════════
 *  COMPARISON MODE
 * ═══════════════════════════════════════════════════════════ */

typedef struct {
    const char *name;
    double avgWT;
    double avgTAT;
} AlgoResult;

static double calcAvgWT(Process p[], int n) {
    double s = 0;
    for (int i = 0; i < n; i++) s += p[i].waiting;
    return s / n;
}

static double calcAvgTAT(Process p[], int n) {
    double s = 0;
    for (int i = 0; i < n; i++) s += p[i].turnaround;
    return s / n;
}

/*
 * runAllAndCompare()
 * Runs every algorithm on the same process set (using copies)
 * and prints a side-by-side comparison table, highlighting
 * the algorithm with the lowest average waiting time.
 */
void runAllAndCompare(Process procs[], int n) {
    Process tmp[MAX_PROCESSES];
    AlgoResult results[6];
    int rCount = 0;

    int quantum = 2;
    printf("\n  Using Time Quantum = %d for Round Robin in comparison.\n", quantum);

    /* ── FCFS ── */
    copyProcesses(procs, tmp, n);
    resetProcesses(tmp, n);
    qsort(tmp, n, sizeof(Process), cmpArrival);
    {
        GanttSegment g[MAX_GANTT]; int gl=0, time=0;
        for (int i = 0; i < n; i++) {
            if (time < tmp[i].arrival) time = tmp[i].arrival;
            snprintf(g[gl].label, MAX_LABEL_LEN, "%s", tmp[i].pid);
            g[gl].start=time; time+=tmp[i].burst; g[gl++].end=time;
            tmp[i].completion=time; tmp[i].turnaround=time-tmp[i].arrival;
            tmp[i].waiting=tmp[i].turnaround-tmp[i].burst;
        }
    }
    results[rCount++] = (AlgoResult){"FCFS", calcAvgWT(tmp,n), calcAvgTAT(tmp,n)};

    /* ── SJF Non-Preemptive ── */
    copyProcesses(procs, tmp, n); resetProcesses(tmp, n);
    {
        int time=0, done=0, finished[MAX_PROCESSES]={0};
        while(done<n){
            int sel=-1;
            for(int i=0;i<n;i++)
                if(!finished[i]&&tmp[i].arrival<=time&&(sel==-1||tmp[i].burst<tmp[sel].burst)) sel=i;
            if(sel==-1){
                int nr=-1;
                for(int i=0;i<n;i++) if(!finished[i]&&(nr==-1||tmp[i].arrival<tmp[nr].arrival)) nr=i;
                time=tmp[nr].arrival; continue;
            }
            time+=tmp[sel].burst; tmp[sel].completion=time;
            tmp[sel].turnaround=time-tmp[sel].arrival;
            tmp[sel].waiting=tmp[sel].turnaround-tmp[sel].burst;
            finished[sel]=1; done++;
        }
    }
    results[rCount++] = (AlgoResult){"SJF Non-Preemptive", calcAvgWT(tmp,n), calcAvgTAT(tmp,n)};

    /* ── SRTF ── */
    copyProcesses(procs, tmp, n); resetProcesses(tmp, n);
    {
        int time=0, done=0;
        while(done<n){
            int sel=-1;
            for(int i=0;i<n;i++)
                if(tmp[i].remaining>0&&tmp[i].arrival<=time&&
                   (sel==-1||tmp[i].remaining<tmp[sel].remaining)) sel=i;
            if(sel==-1){
                int nr=-1;
                for(int i=0;i<n;i++) if(tmp[i].remaining>0&&(nr==-1||tmp[i].arrival<tmp[nr].arrival)) nr=i;
                time=tmp[nr].arrival; continue;
            }
            tmp[sel].remaining--; time++;
            if(tmp[sel].remaining==0){
                tmp[sel].completion=time;
                tmp[sel].turnaround=time-tmp[sel].arrival;
                tmp[sel].waiting=tmp[sel].turnaround-tmp[sel].burst;
                done++;
            }
        }
    }
    results[rCount++] = (AlgoResult){"SRTF (SJF Preemptive)", calcAvgWT(tmp,n), calcAvgTAT(tmp,n)};

    /* ── Round Robin ── */
    copyProcesses(procs, tmp, n); resetProcesses(tmp, n);
    {
        int queue[MAX_PROCESSES*MAX_GANTT], front=0, rear=0;
        int inQ[MAX_PROCESSES]={0}, time=0, done=0;
        for(int i=0;i<n;i++) if(tmp[i].arrival==0){ queue[rear++]=i; inQ[i]=1; }
        while(done<n){
            if(front==rear){
                int nr=-1;
                for(int i=0;i<n;i++) if(tmp[i].remaining>0&&(nr==-1||tmp[i].arrival<tmp[nr].arrival)) nr=i;
                time=tmp[nr].arrival;
                for(int i=0;i<n;i++) if(!inQ[i]&&tmp[i].remaining>0&&tmp[i].arrival<=time){ queue[rear++]=i; inQ[i]=1; }
                continue;
            }
            int cur=queue[front++]; inQ[cur]=0;
            int ex=(tmp[cur].remaining<quantum)?tmp[cur].remaining:quantum;
            tmp[cur].remaining-=ex; time+=ex;
            for(int i=0;i<n;i++)
                if(!inQ[i]&&tmp[i].remaining>0&&tmp[i].arrival<=time&&i!=cur){ queue[rear++]=i; inQ[i]=1; }
            if(tmp[cur].remaining==0){
                tmp[cur].completion=time;
                tmp[cur].turnaround=time-tmp[cur].arrival;
                tmp[cur].waiting=tmp[cur].turnaround-tmp[cur].burst;
                done++;
            } else { queue[rear++]=cur; inQ[cur]=1; }
        }
    }
    results[rCount++] = (AlgoResult){"Round Robin (Q=2)", calcAvgWT(tmp,n), calcAvgTAT(tmp,n)};

    /* ── Priority Non-Preemptive ── */
    copyProcesses(procs, tmp, n); resetProcesses(tmp, n);
    {
        int time=0, done=0, finished[MAX_PROCESSES]={0};
        while(done<n){
            int sel=-1;
            for(int i=0;i<n;i++)
                if(!finished[i]&&tmp[i].arrival<=time&&
                   (sel==-1||tmp[i].priority<tmp[sel].priority)) sel=i;
            if(sel==-1){
                int nr=-1;
                for(int i=0;i<n;i++) if(!finished[i]&&(nr==-1||tmp[i].arrival<tmp[nr].arrival)) nr=i;
                time=tmp[nr].arrival; continue;
            }
            time+=tmp[sel].burst; tmp[sel].completion=time;
            tmp[sel].turnaround=time-tmp[sel].arrival;
            tmp[sel].waiting=tmp[sel].turnaround-tmp[sel].burst;
            finished[sel]=1; done++;
        }
    }
    results[rCount++] = (AlgoResult){"Priority Non-Preemptive", calcAvgWT(tmp,n), calcAvgTAT(tmp,n)};

    /* ── Priority Preemptive ── */
    copyProcesses(procs, tmp, n); resetProcesses(tmp, n);
    {
        int time=0, done=0;
        while(done<n){
            int sel=-1;
            for(int i=0;i<n;i++)
                if(tmp[i].remaining>0&&tmp[i].arrival<=time&&
                   (sel==-1||tmp[i].priority<tmp[sel].priority)) sel=i;
            if(sel==-1){
                int nr=-1;
                for(int i=0;i<n;i++) if(tmp[i].remaining>0&&(nr==-1||tmp[i].arrival<tmp[nr].arrival)) nr=i;
                time=tmp[nr].arrival; continue;
            }
            tmp[sel].remaining--; time++;
            if(tmp[sel].remaining==0){
                tmp[sel].completion=time;
                tmp[sel].turnaround=time-tmp[sel].arrival;
                tmp[sel].waiting=tmp[sel].turnaround-tmp[sel].burst;
                done++;
            }
        }
    }
    results[rCount++] = (AlgoResult){"Priority Preemptive", calcAvgWT(tmp,n), calcAvgTAT(tmp,n)};

    /* ── Print Comparison Table ── */
    int bestIdx = 0;
    for (int i = 1; i < rCount; i++)
        if (results[i].avgWT < results[bestIdx].avgWT) bestIdx = i;

    printf("\n");
    printf("  ╔══════════════════════════════════════════════════════════════════╗\n");
    printf("  ║             ALGORITHM COMPARISON SUMMARY                       ║\n");
    printf("  ╠══════════════════════════════╦═══════════════╦═════════════════╣\n");
    printf("  ║ Algorithm                    ║   Avg WT      ║   Avg TAT       ║\n");
    printf("  ╠══════════════════════════════╬═══════════════╬═════════════════╣\n");

    for (int i = 0; i < rCount; i++) {
        const char *marker = (i == bestIdx) ? " ★ BEST" : "       ";
        printf("  ║ %-28s ║    %8.2f    ║    %10.2f   ║%s\n",
               results[i].name, results[i].avgWT, results[i].avgTAT, marker);
    }

    printf("  ╚══════════════════════════════╩═══════════════╩═════════════════╝\n");
    printf("\n  ★  Best algorithm for this dataset: %s\n", results[bestIdx].name);
    printf("     (Lowest Average Waiting Time: %.2f)\n\n", results[bestIdx].avgWT);
}