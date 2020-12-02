```
trace-cmd record -p function_graph --max-graph-depth 2 -e syscalls ./hello  
trace-cmd record -p function_graph -g __x64_sys_write ./hello # to only look at write function  
trace-cmd record -p function_graph -g __x64_sys_write -n rw_verify_area -O nofuncgraph-irqs -F ./hello # trace write without rw_verify_area or irqs   
trace-cmd record -e sched_wakeup -e sched_switch -e sys_enter_write ./hello  
---
# print buffer value of register si  
echo 'p:data tty_audit_add_data buf=+0(%si):string' > /sys/kernel/tracing/kprobe_events  
trace-cmd record -e data ./hello  
---
```
https://git.kernel.org/pub/scm/linux/kernel/git/rostedt/trace-cmd.git  
https://git.kernel.org/pub/scm/utils/trace-cmd/trace-cmd.git
