/*
 * ============================================================
 *  OS CPU Scheduling Simulator
 *  scheduler.h — Structs, constants, and function prototypes
 * ============================================================
 *
 * Sample Input/Output:
 *   Processes: 4
 *   PID | Arrival | Burst | Priority
 *    P1 |    0    |   8   |   2
 *    P2 |    1    |   4   |   1
 *    P3 |    2    |   9   |   3
 *    P4 |    3    |   5   |   2
 *
 *   FCFS Result:
 *   | P1 | P2 | P3 | P4 |
 *   0    8   12   21   26
 *   Avg WT: 8.75  Avg TAT: 15.25
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── Constants ─────────────────────────────────────────── */
#define MAX_PROCESSES    20
#define MAX_GANTT        500   /* maximum gantt chart segments */
#define MAX_LABEL_LEN    8

/* ─── Process struct ────────────────────────────────────── */
typedef struct {
    char  pid[MAX_LABEL_LEN]; /* process identifier, e.g. "P1"  */
    int   arrival;            /* arrival time                    */
    int   burst;              /* original CPU burst time         */
    int   remaining;          /* remaining burst (for SRTF / RR) */
    int   priority;           /* lower value = higher priority   */
    int   completion;         /* when the process finishes       */
    int   waiting;            /* waiting time = TAT - burst      */
    int   turnaround;         /* turnaround = completion-arrival */
    int   started;            /* flag: has the process started?  */
} Process;

/* ─── Gantt chart segment ───────────────────────────────── */
typedef struct {
    char label[MAX_LABEL_LEN]; /* "P1", "IDLE", etc. */
    int  start;
    int  end;
} GanttSegment;

/* ─── Input / Output ────────────────────────────────────── */
int  inputProcesses(Process procs[], int *n);
void printTable(Process procs[], int n, const char *algoName);
void printGanttChart(GanttSegment gantt[], int gLen);
void printAverages(Process procs[], int n);

/* ─── Scheduling algorithms ─────────────────────────────── */
void fcfs(Process procs[], int n);
void sjf_non_preemptive(Process procs[], int n);
void sjf_preemptive(Process procs[], int n);
void round_robin(Process procs[], int n, int quantum);
void priority_non_preemptive(Process procs[], int n);
void priority_preemptive(Process procs[], int n);

/* ─── Comparison ────────────────────────────────────────── */
void runAllAndCompare(Process procs[], int n);

/* ─── Helpers ───────────────────────────────────────────── */
void copyProcesses(const Process src[], Process dst[], int n);
void resetProcesses(Process procs[], int n);
int  allDone(const Process procs[], int n);
int  validateInput(int val, int min, int max, const char *field);

#endif /* SCHEDULER_H */