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

IO operations in libk
---------------------

All io operations in libk are derived from the io_device subsystem. This means that all io operations are asynchronous and should therefore only be called from within a protothread. So whenever you need to do some io, you also must always have a task running that will process the io for you. All IO operations must also return into the main scheduler. This means that the following will not work: 

    while(!io_write(dev, timeout, buffer, size)); // blocking loop
    
Instead one needs to do this: 

    PT_ASYNC_WRITE(pt, dev, timeout, buffer, size); 
    
Inside a context of a protothread. 

Asynchronous IO for embedded systems
------------------------------------

Asynchronous io is nearly always a bit cumbersome to implement. It nearly always results in a more complicated program and in nearly all cases it requires the program to be structured as a state machine. When all IO operations are non blocking, the program needs to be structured in such a way that it can both wait for the io operation to complete and also to be able to do other things while it is waiting for the io operation to complete. If the system was multithreaded and blocking, we could easily just have the operating system put the process to sleep and then awaken it when io is ready. But in the perfect world we want to have many processes in the same application that never block the rest of the application while they are waiting for io. For this we need something else. Unfortunately there is no way we can escape structuring the program as a state machine. But we can do so in the most syntactically pleasant way. 

If all io operations maintain internal state over the curse of multiple calls, we can also say that each function that does io is also a state machine that cycles through a number of states while it starts and waits for different io operations to complete. We can hide this fact from the programmer by writing a set of clever macros that will make it easier for us to implement the methods that need to cycle through several states. Luckily this has already been written and it is best known as protothreads. 

Protothreads are just a set of macros that make it easier to write switch/case state machines that always cycle from one state to the next in a sequence. It is like async.each_series() in jquery. With protothreads we can structure multiple asynchronous operations in a sequence and hide all of the switch/case logic behind the scenes. 

But this is not just a good way to structure individual methods - it also makes it possible to write hierarchies of these state machine methods. This is particularly useful in the context of doing io because we usually have many io operations that call other io operations that call even more io operations. And all of these io operations are asynchronous so without protothreads it would be a nightmare to write such io code. But with protothreads we can make this work very simple. 

ASYNC/AWAIT pattern in C and C++
--------------------------------

The async/await pattern allows methods to be asynchronous and take arbitrary number of frames to execute. This is extremely useful when programming something that changes over time - for example if we need to animate some object over a number of frames. 

Unfortunately async/await is not natively supported (yet) in C and C++, but we can get away with simulating this behavior using address labels feature which allows us to jump to an arbitrary location inside a method. 

Limitations of async methods
----------------------------

Asynchronous methods have several limitations though: 

* They can not declare any local variables - all variables have to be part of the async context that is stored elsewhere and has a lifetime longer than the scope of the method itself. This is quite natural, since async methods return and resume multiple times.
* An async method starts from the top the first time it is called, then goes to the bottom (if it can) and once it finishes it resumes at the top again. We can also have methods that never finish and instead run in a loop. This is perfectly fine as well since we can easily jump into a loop using the address labels technique. 
