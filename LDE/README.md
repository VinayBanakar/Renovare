A way to measure context switch cost in time between two process
taskset -c 0 ./cntx_sw_bench 100000

* Syscall noise is still not accounted for.
* Alternate: perf record -e context-switches -a
* futex (stay in uspace as much as possible by calling fewer system calls) over mutex for reduced U-K transition overhead.
* Wehn measuring cntx switch cost between two porcess/threads remember that overhead on hyperthreaded cores will be cheaper because these cores have space to store both states.

Evolution of linux context switch
http://www.maizure.org/projects/evolution_x86_context_switch_linux/
