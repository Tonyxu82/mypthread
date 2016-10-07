# User Level Thread Library

###### Lai Xu 929272440 (xulai@pdx.edu), Bo Chen 904039384 (chenbo@pdx.edu)

## Motivation

In the previous project assignments, we have already built a simple user level thread library. The problem is we just use only one kernel thread to run it. Currently, most of computer system is multiprocessor/multicore, At the same time the operating system can support them in the kernel. If we want to use these facilities, what we can do is to let our user level thread library run user-level threads on more than one kernel threads.

## Design Decision

Before designing new features in the library, there are three questions we should answer.

1. How to create more kernel threads in Linux?
2. How to change our scheduler to support multiple current_thread running at the same time?
3. When real concurrency mechanism has involved in our library whether races? If races exist, how can we eliminate it?

For the first question, current Linux has already provided "clone" system call to create kernel threads.

For the second question, in the previous assignments we always maintain a curent_thread since we running all the user-level threads on just only one kernel thread. If we have N kernel threads, means we can let N user-level threads bind to it. In the other words we need N current_thread which also include the mapping from user-level thread to kernel thread. To maintain these current_threads we can use an array, link list or linked hash table. Since we can not decide how many kernel threads should be created, we need a dynamic data structure to store them, so the array has been excluded. Since the hash table provides us better average time complexity than linear data structure, so we have chosen hash table to implement it.

For the third question, the queues for maintaining user-level threads such as read queue, and users' programs which involving shared data suffer the risk of race. So this is why we need lock to protect the critical section. Since the data we want to manipulate is very small, in the other words the locking time is short, so a Spinlock (implemented with test-and-set) is a better choice in a multiprocessor environment.

## Implementation Strategy

One step to implement whole project is daunting. So we apply iterative development in this project. We will divide it into 5 iterations. The logic of task breakdown is to add basic logic as fundament, then consider how to ensure the correctness and robustness, finally the scalability. In general, it is from functional requirements to quality attributes.


We describe each iteration (Part) in the below.


### Part 0 & 1

In this part, the requirement for us to add second kernel thread to support more current_threads. What we do in the project/step1 include:
1. Adding threadmap.c in our project.
2. Modifying scheduler_begin () to add the logic of invoking clone() to create kernel threads
2. Adding kernel_thread_begin () which yielding forever in the scheduler.h and scheduler. c

The result is we have more kernel threads in our thread library. But it does not work currently.
So in this part the test result is that the program hangs there.

### Part 2

In this part, the target is to protect the ready list not being corrupted by concurrency mechanism. The method is to implement the Spinlock. For implementing Spinlock, we need atomic operation-test-and-set, which is provided in the 3rd party library "libatomic_ops". In the project/step2, we have tested the spinlock_test.c provided by Kendall. In this program is used to test whether the Spinlock can protect shared data - shared_counter and shared_arrary not been corrupted. After all threads, finishing their work, the program will verify whether the value of shared_array in position i can equal to i. If so, it outputs a "o", else an "x". If the output just contains "o", then we can say Spinlock works here. This is the output of our program:

oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
success!


### Part 3
In this part, we will formally add the Spinlock into our project and following functions should be modified by adding the Spinlock:
-scheduler_begin
-scheduler_end
-Yield
-thread_fork
-Synchronization primitives like mutex_lock.

When we perform yield and thread_fork functions, we will meet a problem is when and where to release the Spinlock. If the lock is released before the thread_swith, the task may be corrupted by the other kernel thread. If we release the lock after thread_switch, it seems impossible for us. So in the project we implement a thread_fork_wrapper () to intead original thread's initial function. If thread_start or thread_switch call this function, it will perform release lock for protecting the ready queue first, then execute the original function. After modifying the code we use the similar function used in part 2 to test our project, it works fine. After verifying the value of shared_arrary in position I, we get an output "success!"

### Part 4

With the supporting of Spinlock, we need to modify the matrix mechanism to support "block" in the multiprocessor environment. So we modify the block, mutex_lock and mutex_unlock. For testing what we just add to the code, we still use the test case that used in the last Part, but we replace all Spinlock to mute. The test result shows it works fine.

### Part 5

In this part, we will verify the scalability of our current thread library. The scalability here means if we adding more kernel thread, can this behavior improve the total performance or degrade the performance. If degradation happens, when, where and why it happens. For testing the scalability of our system, we prepare a benchmark which performs a lot competition with the power of multithread. We can input how many kernel threads will be used in this test as parameter. Also we use "time" commond to see the execution time of the program. The common line we use is like this "time ./step5 5", and we record the result like:
$ time ./step5 3:   3.252s

The detailed analysis of this test will be described in the next section.

## The Analysis of Scalability

When we read the paper "Anderson, Thomas E. "The performance of spin lock alternatives for shared-memory multiprocessors", we know the Spinlock may cause the performance degradation and scalability problem. The problem caused many subtle reasons such as the bus contention and cache invalidation. Another reason for lack of scalability is the design and implementation of the scheduler in the user-level thread library. If the schedule does not own a good algorithm, that means it cannot use the facility of multiprocessor system very well, then we also can not get the result of expectations.

How about the scalability of our user-level thread library? As a verification method, we design a benchmark. In this benchmark, we create a large data set, according to the assigned value for how much data can be processed by each thread, we divide the data set into pieces and assign to different threads for processing. These threads will perform some computation on these data. After them finishing the task, the result will be returned. When all threads have finished their tasks, the main () function will statistic the total time spent on the benchmark. Since we can assign how many kernel threads used in this program via the parameter, we can test case by case when increasing the number of kernel threads.

In this test, we will expect when adding kernel threads the performance will increase to an inflection point appears. After this point, the performance will be stable or degraded when you add more kernel threads.

The test platform is a Dell Inspiron R17 turbo with Intel? Core? i7-3610QM CPU @ 2.30GHz 4 cores processor and 8GB memory. The operating system is Ubuntu 14.04 LTS 64bit with the kernel  3.13.0-40-generic #69-Ubuntu SMP. The test result shows like below:

$ time ./step5 1:   7.083s
$ time ./step5 3:   3.252s
$ time ./step5 5:   2.123s
$ time ./step5 7:   1.689s
$ time ./step5 9:   1.742s
$ time ./step5 20:  1.807s
$ time ./step5 30:  2.927s
$ time ./step5 50:  4.598s
$ time ./step5 100: 18.193s

From the test results above, we can clearly find a inflection point which is around 7. Before that point, the efficiency improves along with the number of kernel threads increasing. After that point, the efficiency decreases even though the number of kernel threads decreasing. This is just as what we expected.
