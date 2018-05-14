# Scheduling Project 

1 Overview:
In this project, you will implement a multiprocessor operating system simulator using a popular userspace threading library for linux called pthreads. The framework for the multithreaded OS simulator is nearly complete, but missing one critical component: the CPU scheduler! Your task is to implement the CPU scheduler, using three different scheduling algorithms.

Note: Make sure that multiple CPU cores are enabled in your virtual machine, otherwise you will receive incorrect results.

We have provided you with source files that constitute the framework for your simulator. You will only need to modify answers.txt and student.c. However, just because you are only modifying two files doesn’t mean that you should ignore the other ones - there is helpful information in the other files. Information about using the pthreads library is given in Problem 0.
We have provided you these files:
1. Makefile - Working one provided for you; do not modify.
2. src/os-sim.c - Code for the operating system simulator which calls your CPU scheduler. 
3. src/os-sim.h - Header file for the simulator.
4. src/process.c - Descriptions of the simulated processes.
5. src/process.h - Header file for the process data.
6. src/student.c - This file contains stub functions for your CPU scheduler.
7. src/student.h - Header file for your code to interface with the OS simulator

1.1 Simulator Algorithms:
For your simulator, you will implement the following three CPU scheduling algorithms:
1. First In, First Out (FIFO) - Runnable processes are kept in a ready queue. FIFO is non-preemptive; once a process begins running on a CPU, it will continue running until it either completes or blocks for I/O.
2. Round-Robin - Similar to FIFO, except preemptive. Each process is assigned a timeslice when it is scheduled. At the end of the timeslice, if the process is still running, the process is preempted, and moved to the tail of the ready queue.
3. Shortest Remaining Time First (SRTF) - The process with the shortest remaining time in its burst always gets the CPU. Longer processes must be pre-empted if a process that has a shorter burst becomes runnable.

1.2 Process States
In our OS simulation, there are five possible states for a process, which are listed in the process state t enum in os-sim.h:
1. NEW - The process is being created, and has not yet begun executing.
2. READY - The process is ready to execute, and is waiting to be scheduled on a CPU.
3. RUNNING - The process is currently executing on a CPU.
4. WAITING - The process has temporarily stopped executing, and is waiting on an I/O request to complete.
5. TERMINATED - The process has completed.
There is a field named state in the PCB, which must be updated with the current state of the process. The
simulator will use this field to collect statistics.

1.3 The Ready Queue
On most systems, there are a large number of processes, but only one or two CPUs on which to execute them. When there are more processes ready to execute than CPUs, processes must wait in the READY state until a CPU becomes available. To keep track of the processes waiting to execute, we keep a ready queue of the processes in the READY state
Since the ready queue is accessed by multiple processors, which may add and remove processes from the ready queue, the ready queue must be protected by some form of synchronization–for this project, you will use a mutex lock. The ready queue SHOULD use a different mutex than the current mutex.

1.4 Scheduling Processes
schedule() is the core function of the CPU scheduler. It is invoked whenever a CPU becomes available for running a process. schedule() must search the ready queue, select a runnable process, and call the context switch() function to switch the process onto the CPU.
There is a special process, the idle process, which is scheduled whenever there are no processes in the READY state.

1.5 CPU Scheduler Invocation
There are four events which will cause the simulator to invoke schedule():
1. yield() - A process completes its CPU operations and yields the processor to perform an I/O request.
2. wake up() - A process that previously yielded completes its I/O request, and is ready to perform CPU operations. wake up() is also called when a process in the NEW state becomes runnable.
3. preempt() - When using a Round-Robin or SRTF scheduling algorithm, a CPU-bound process may be preempted before it completes its CPU operations.
4. terminate() - A process exits or is killed.
The CPU scheduler also contains one other important function: idle(). Idle contains the code that gets by the idle process. In the real world, the idle process puts the processor in a low-power mode and waits. For our OS simulation, you will use a pthread condition variable to block the thread until a process enters the ready queue.

1.6 The Simulator
We will use pthreads to simulate an operating system on a multiprocessor computer. We will use one thread per CPU and one thread as a ’supervisor’ for our simulation. The CPU threads will simulate the currently- running processes on each CPU, and the supervisor thread will print output and dispatch events to the CPU threads.
Since the code you write will be called from multiple threads, the CPU scheduler you write must be thread- safe! This means that all data structures you use, including your ready queue, must be protected using mutexes.
The number of CPUs is specified as a command-line parameter to the simulator. For this project, you will be performing experiments with 1, 2, and 4 CPU simulations.
Also, for demonstration purposes, the simulator executes much slower than a real system would. In the real world, a CPU burst might range from one to a few hundred milliseconds, whereas in this simulator, they range from 0.2 to 2.0 seconds.

The simulator generates a Gantt Chart, showing the current state of the OS at every 100ms interval. The leftmost column shows the current time, in seconds. The next three columns show the number of Running, Ready, and Waiting processes, respectively. The next two columns show the process currently running on each CPU. The rightmost column shows the processes which are currently in the I/O queue, with the head of the queue on the left and the tail of the queue on the right.
As you can see, nothing is executing. This is because we have no CPU scheduler to select processes to execute! Once you complete Problem 1 and implement a basic FIFO scheduler, you will see the processes executing on the CPUs.

2 Problem 1: FIFO Scheduler
NOTE: Part B of each part requires you to put your answer down in answers.txt

2.1 Part A
Implement the CPU scheduler using the FIFO scheduling algorithm. You may do this however you like, however, we suggest the following:
• Implement a thread-safe ready queue using a linked list. A linked list will allow you to reuse this ready queue for the Round-Robin and SRTF scheduling algorithms.
• Implement the yield(), wake up(), and terminate() handlers. preempt() is not necessary for this stage of the project. See the overview and the comments in the code for the proper behavior of these events.
• Implement idle(). idle() must wait on a condition variable that is signalled whenever a process is added to the ready queue.
• Implement schedule(). schedule() should extract the first process in the ready queue, then call context switch() to select the process to execute. If there are no runnable processes, schedule() should call context switch() with a NULL pointer as the PCB to execute the idle process.

2.2 Part B
Run your OS simulation with 1, 2, and 4 CPUs. Compare the total execution time of each. Is there a linear relationship between the number of CPUs and total execution time? Why or why not? Keep in mind that the execution time refers to the simulated execution time.

3 Problem 2: Round-Robin Scheduler 
3.1 Part A
Add Round-Robin scheduling functionality to your code. You should modify main() to add a command line option, -r, which selects the Round-Robin scheduling algorithm, and accepts a parameter, the length of the timeslice. For this project, timeslices are measured in tenths of seconds. E.g.:
$ ./os−sim<#CPUs>−r 5
should run a Round-Robin scheduler with timeslices of 500 ms. While:
$ ./os−sim <# of CPUs>
should continue to run a FIFO scheduler. You should also make sure preempt is implemented in this section
of the project.
To specify a timeslice when scheduling a process, use the timeslice parameter of context switch(). The simulator will automatically preempt the process and call your preempt() handler if the process executes on the CPU for the length of the timeslice without terminating or yielding for I/O.

3.2 Part B
Run your Round-Robin scheduler with timeslices of 800ms, 600ms, 400ms, and 200ms. Use only one CPU for your tests. Compare the statistics at the end of the simulation. Show that the total waiting time decreases with shorter timeslices. However, in a real OS, the shortest timeslice possible is usually not the best choice. Why not?

4 Problem 3: Shortest Remaining Time First Scheduler 

4.1 Part A
Add SRTF scheduling to your code. Modify main() to accept the -s parameter to select the SRTF algorithm. The -r and default FIFO scheduler should continue to work.
The scheduler should use the time remaining field of the PCB to prioritize processes that have a shorter remaining time in their CPU burst.
For SRTF scheduling, you will need to make use of the current[] array and force preempt() function. The current[] array should be used to keep track of the process currently executing on each CPU. Since this array is accessed by multiple CPU threads, it must be protected by a mutex. current mutex has been provided for you.
The force preempt() function preempts a running process before its timeslice expires. Your wake up() handler should make use of this function to preempt a process when a process with lower time remaining needs a CPU.

4.2 Part B
While it is easy to simulate an SRTF algorithm in the simulator, it is essentially impossible to implement precisely in real life and is thus usually approximated. Why is this the case?
Run each of the scheduling algorithms using one CPU and compare the total waiting times. Which one had the lowest? Why?




