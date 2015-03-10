Kernel Architecture
===================

LibK is a lightweight kernel that implements cooperative multitasking using stackless threads. Stackless threads have been chosen because they are the most lightweight kind of threads available and they have very small overhead. There are however both advantages and also some disadvantages to using stackless protothreads. 

Advantages of stackless protothreads: 

- Allow device drivers to be written such that no CPU cycles are ever wasted waiting for I/O. The device driver can simply return control to the application and wait until the next time it has the chance to run. 
- Writing asynchronous tasks becomes a lot easier because they can be written linearly instead of being organized as a complicated state machine. 
- No thread is ever interrupted while it is in the middle of some operation. Threads always run to completion. Meaning that we can design our code without having to think about byte level synchronization (device level syncrhonization is required though). 
- No stack also means that we never run out of stack space inside a thread. 
- Context switching is very fast - current thread method saves it's resume point, returns to libk scheduler, libk scheduler loads the address of the next thread function to run, and calls it. 
- Not a problem to have tens of threads for each asynchronous action. Since threads are just normal methods minus the stack, we can have many threads without experiencing significant slowdown. 
- Synchronization is much easier because all code that you "see" is atomic until it explicitly releases control to the scheduler. We thus do not have to worry about non atomic memory access. 

Disadvantages of this approach: 

- Stackless means that no variable on the thread method stack is valid after a thread returns and then resumes again. Although we can easily solve this by maintaining the context inside a separate object to which the thread is attached. 
- No preemption also means that a thread can keep CPU to itself for as long as it wishes. It is up to the programmer to release the CPU as quickly as possible. Since device drivers usually use interrupt requests to respond in realtime anyway, this limitation has not proven to be a problem so far. 
- No thread can ever call another thread or spawn a new thread. Threads are only single level. All other code called from inside the thread can be considered to execute atomically (except for when it is interrupted by an ISR). 
- Data that needs to be saved across multiple thread switches must be stored in memory (this is a problem with all multitasking though). 
- Longer response times for tasks - a task can lose cpu for as long as it takes the heavies task to finish. This is solved by programmer explicitly designing threads such that control is periodically released to the scheduler. 

A detailed evaluation of libk threading
--------------------------------------

Stackless threads in libk are designed to solve one specific problem: busy waits. I settled for this approach because it has been the most lightweight solution to this problem. Almost 98% of all embedded code uses busy waits - the standard way to solve this problem is by implementing a scheduler that is able to interrupt a currently running task in the middle of it's busy loop and switch to a different task. This however also comes with a lot of subtle problems that result in much more synchronization code all around the application to ensure atomic access to shared data. 

With protothreads, it is instead possible to minimize the amount of locking and synchronization necessary, while at the same time to enjoy a healthy degree of multitasking where CPU rarely is just idly spinning inside some delay loop.

The main area where this kind of multitasking really is useful is device drivers that do a lot of I/O. I/O operations are by far the greatest bottleneck in most embedded systems that don't use multitasking and instead resort to idly waiting for an I/O operation to complete. LibK solves this problem by doing minimal caching of data and also by never waiting for an I/O operation to complete and instead letting another task run while I/O is in progress. 

Another attractive feature of libk threading is that it is completely implemented in software - meaning that it will work the same on all hardware. It is after all just an array of "update" methods that the kernel schedules periodically. 

I have found that I could improve performance with protothreading almost 100x. When I eliminated all busy delay loops in the device drivers I have found that my application was able to run a lot faster and also it has become much more responsive. I have not howerver noticed a significant memory overhead. By far the main memory overhead (which is also a necessary evil) is caching data in memory so that it can be retained while another thread has control of the CPU. Most drivers use caching in one way or another. 
