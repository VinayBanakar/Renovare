TODO:
* Ticket inflation mechanism amongst untrusted peers?
* Current Ticket tranfer methods used in distributed systems?
* Should OS schedulers (which are predominantly fair share) change to cater special workloads like HPC/ML? if so how?
* How fast should the timer interrupt (HZ) be based on the kind of workload (especially with lot of VMs)? (100-1000) power saving vs performance https://lwn.net/Articles/145973/
>> See hrtimers https://www.kernel.org/doc/Documentation/timers/hrtimers.txt and dynamic ticks (allows system to run at full HZ during load, and skips ticks when possible while idle).
* A tool to interpret meaningful stuff from /proc/sched_debug

* MLFQ in Go
* Lottery/Stride scheduling in Go?




## Random
Okay I totally don't understand this; My timer interrupt frequency (CLK_TCK) is 300HZ which means my jiffies global count for the core will be incremented every 3 ms, but increment of number of interrupts every 1s in /proc/interrupt is not equal to 300.
Also getconf CLK_TCK is not same as zcat /proc/config.gz | grep 'CONFIG_HZ', are these two variables representing something else all together?
>> Depending on other kernel parameters (tickless and friends), the kernel won't schedule a tick it doesn't need - so if there's one process that's CPU bound while all the other processes are being driven by I/O not by the tick, the kernel may decide that the CPU bound job can run up to 750 milliseconds, and schedule just one tick 0.75 seconds from now and skip all the intervening ticks.

## adaptive-ticks CPUs
If you have one active process in a system, for example a long-running CPU-intensive task. That’s nearly identical to an idle system. It’s pointless to interrupt the task every X ms for no good reason: it’s merely OS jitter slowing your work ever so slightly. Linux can also stop the fixed-rate tick in this one-process scenario, in what’s called adaptive-tick mode.
https://github.com/torvalds/linux/blob/v3.17/Documentation/timers/NO_HZ.txt#L100

Also, you could also use SCHED_FIFO if you don't want a process to be switched before its completion or invoking I/O. It will also be switched if another process of higher PRI shows up in the exection queue or it calls sched_yield.

For global scheduling info:
/proc/sched_debug  /proc/schedstat

For thread level scheduling info:
/proc/PID/sched   /proc/PID/schedstat
/linux/Documentation/scheduler/sched-stats.rst


# Processor power states
https://metebalci.com/blog/a-minimum-complete-tutorial-of-cpu-power-management-c-states-and-p-states/
