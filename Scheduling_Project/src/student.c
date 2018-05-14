/*
 * student.c
 * Multithreaded OS Simulation for CS 2200
 *
 * This file contains the CPU scheduler for the simulation.
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "student.h"
#include "os-sim.h"

/** Function prototypes **/
extern void idle(unsigned int cpu_id);
extern void preempt(unsigned int cpu_id);
extern void yield(unsigned int cpu_id);
extern void terminate(unsigned int cpu_id);
extern void wake_up(pcb_t *process);

/*
 * current[] is an array of pointers to the currently running processes.
 * There is one array element corresponding to each CPU in the simulation.
 *
 * current[] should be updated by schedule() each time a process is scheduled
 * on a CPU.  Since the current[] array is accessed by multiple threads, you
 * will need to use a mutex to protect it.  current_mutex has been provided
 * for your use.
 */
static pcb_t **current;
static pthread_mutex_t current_mutex;
// global var: ready queue
static pcb_t *head;
static pthread_mutex_t ready_queue_lock;
static pthread_cond_t ready_queue_cond;
int timeslice = -1;
unsigned int cpu_count;

typedef enum {
    FIFO = 0, RoundRobin = 1, STRF = 2
} sched_type;

static sched_type schedular;

static void enqueue(pcb_t *process)
{   
    if (head == NULL) {
        head = process;
        head->next = NULL;
        pthread_cond_signal(&ready_queue_cond);
    } else {
        pcb_t *curr = head;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = process;
        process->next = NULL;
    }
}

static pcb_t* dequeue(void)
{
    pcb_t *removed = NULL;
    // if STRF is chosen
    if (schedular == STRF) {
        if (head == NULL) {
            return removed;
        }
        removed = head;
        pcb_t* curr = removed->next;
        while (curr != NULL) {
            if (curr->time_remaining < removed->time_remaining) {
                removed = curr;
            } 
            curr = curr->next;
        }
        if (removed == head) {
            head = head->next;
        } else {
            pcb_t* overAgain = head;
            while (overAgain->next != removed) {
                overAgain = overAgain->next;
            }
            overAgain->next = removed->next;
        }
        removed->next = NULL;
        return removed;
    }

    // if FIFO or RR is chosen
    if (head == NULL) {
        return removed;
    } else {
        removed = head;
        head = head->next;
    }
    removed->next = NULL;
    return removed;
}


/*
 * schedule() is your CPU scheduler.  It should perform the following tasks:
 *
 *   1. Select and remove a runnable process from your ready queue which 
 *	you will have to implement with a linked list or something of the sort.
 *
 *   2. Set the process state to RUNNING
 *
 *   3. Call context_switch(), to tell the simulator which process to execute
 *      next on the CPU.  If no process is runnable, call context_switch()
 *      with a pointer to NULL to select the idle process.
 *	The current array (see above) is how you access the currently running process indexed by the cpu id. 
 *	See above for full description.
 *	context_switch() is prototyped in os-sim.h. Look there for more information 
 *	about it and its parameters.
 */
static void schedule(unsigned int cpu_id)
{
    pthread_mutex_lock(&ready_queue_lock);
    pcb_t *removed = dequeue();
    pthread_mutex_unlock(&ready_queue_lock);
    /* FIX ME */
    if (removed == NULL) {
        context_switch(cpu_id, NULL, -1);
    } else if (removed != NULL) {

        pthread_mutex_lock(&ready_queue_lock);
        removed->state = PROCESS_RUNNING;
        pthread_mutex_unlock(&ready_queue_lock);
        
    }
    pthread_mutex_lock(&current_mutex);
    current[cpu_id] = removed;
    pthread_mutex_unlock(&current_mutex);

    if (schedular == FIFO) {
        context_switch(cpu_id, removed, -1);
    } else if (schedular == RoundRobin) {
        context_switch(cpu_id, removed, timeslice);
    } else if (schedular == STRF) {
        context_switch(cpu_id, removed, -1);
    }
}

/*
 * idle() is your idle process.  It is called by the simulator when the idle
 * process is scheduled.
 *
 * This function should block until a process is added to your ready queue.
 * It should then call schedule() to select the process to run on the CPU.
 */
extern void idle(unsigned int cpu_id)
{
    /* FIX ME */
    pthread_mutex_lock(&ready_queue_lock);
    while (head == NULL) {
        pthread_cond_wait(&ready_queue_cond, &ready_queue_lock);
    }
    pthread_mutex_unlock(&ready_queue_lock);
    schedule(cpu_id);
    /*
     * REMOVE THE LINE BELOW AFTER IMPLEMENTING IDLE()
     *
     * idle() must block when the ready queue is empty, or else the CPU threads
     * will spin in a loop.  Until a ready queue is implemented, we'll put the
     * thread to sleep to keep it from consuming 100% of the CPU time.  Once
     * you implement a proper idle() function using a condition variable,
     * remove the call to mt_safe_usleep() below.
     */
}




/*
 * preempt() is the handler called by the simulator when a process is
 * preempted due to its timeslice expiring.
 *
 * This function should place the currently running process back in the
 * ready queue, and call schedule() to select a new runnable process.
 */
extern void preempt(unsigned int cpu_id)
{
    /* FIX ME */
    pthread_mutex_lock(&current_mutex);

    pcb_t *proc = current[cpu_id];
    current[cpu_id]->state = PROCESS_READY;

    pthread_mutex_unlock(&current_mutex);

    pthread_mutex_lock(&ready_queue_lock);
    enqueue(proc);
    pthread_mutex_unlock(&ready_queue_lock);
    schedule(cpu_id);
}


/*
 * yield() is the handler called by the simulator when a process yields the
 * CPU to perform an I/O request.
 *
 * It should mark the process as WAITING, then call schedule() to select
 * a new process for the CPU.
 */
extern void yield(unsigned int cpu_id)
{
    /* FIX ME */
    pthread_mutex_lock(&current_mutex);
    pcb_t *hello = current[cpu_id];
    hello->state = PROCESS_WAITING;
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}


/*
 * terminate() is the handler called by the simulator when a process completes.
 * It should mark the process as terminated, then call schedule() to select
 * a new process for the CPU.
 */
extern void terminate(unsigned int cpu_id)
{
    /* FIX ME */
    pthread_mutex_lock(&current_mutex);
    pcb_t *hello = current[cpu_id];
    hello->state = PROCESS_TERMINATED;
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}


/*
 * wake_up() is the handler called by the simulator when a process's I/O
 * request completes.  It should perform the following tasks:
 *
 *   1. Mark the process as READY, and insert it into the ready queue.
 *
 *   2. If the scheduling algorithm is SRTF, wake_up() may need
 *      to preempt the CPU with the highest remaining time left to allow it to
 *      execute the process which just woke up.  However, if any CPU is
 *      currently running idle, or all of the CPUs are running processes
 *      with a lower remaining time left than the one which just woke up, wake_up()
 *      should not preempt any CPUs.
 *	To preempt a process, use force_preempt(). Look in os-sim.h for 
 * 	its prototype and the parameters it takes in.
 */
extern void wake_up(pcb_t *process)
{
    /* FIX ME */
    process->state = PROCESS_READY;
    pthread_mutex_lock(&ready_queue_lock);
    enqueue(process);
    pthread_mutex_unlock(&ready_queue_lock);

    if (schedular == STRF) {
        pcb_t *highest_rem = process;
        unsigned int num = 0;
        pthread_mutex_lock(&current_mutex);
        for (unsigned int i = 0; i < cpu_count; i++) {
            if (current[i] == NULL) {
                highest_rem = current[i];
                break;
            } else if (current[i]->time_remaining > highest_rem->time_remaining) {
                highest_rem = current[i];
                num = i;
            }
        }
        pthread_mutex_unlock(&current_mutex);
        if (highest_rem != process && highest_rem != NULL) {
            force_preempt(num);
        }
    }
}


/*
 * main() simply parses command line arguments, then calls start_simulator().
 * You will need to modify it to support the -r and -s command-line parameters.
 */
int main(int argc, char *argv[])
{
    /* Parse command-line arguments */
    if (argc == 2) {
        schedular = FIFO;
    } else if (strcmp(argv[2], "-r") == 0) {
        schedular = RoundRobin;
        timeslice = atoi(argv[3]);
    } else if (strcmp(argv[2], "-s") == 0) {
        schedular = STRF;
    } else {
        fprintf(stderr, "CS 2200 OS Sim -- Multithreaded OS Simulator\n"
            "Usage: ./os-sim <# CPUs> [ -r <time slice> | -s ]\n"
            "    Default : FIFO Scheduler\n"
            "         -r : Round-Robin Scheduler\n"
            "         -s : Shortest Remaining Time First Scheduler\n\n");
        return -1;
    }
    cpu_count = strtoul(argv[1], NULL, 0);

    /* FIX ME - Add support for -r and -s parameters*/

    /* Allocate the current[] array and its mutex */
    current = malloc(sizeof(pcb_t*) * cpu_count);
    assert(current != NULL);
    pthread_mutex_init(&current_mutex, NULL);
    pthread_mutex_init(&ready_queue_lock, NULL);

    /* Start the simulator in the library */
    start_simulator(cpu_count);

    return 0;
}


