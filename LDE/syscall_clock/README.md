TODO:
Maybe you can also use jiffies to measure time taken.
The period of 1 jiffy is typically 1ns, this and current jiffy can be read from
/proc/timer_list.

blog on performance vs accuracy of differnt measurement methods?


Technically jiffy in computer parlance is the duration of 1 tick of the system timer interrupt. It's not absolute though.
