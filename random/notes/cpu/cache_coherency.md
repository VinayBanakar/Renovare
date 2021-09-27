https://youtu.be/PZ5kX5w5YUY?t=5872

http://lastweek.io/notes/cache_coherence/#case-study
  
L3 cache slices:  
L3 caches on recent Intel processors are built up of multiple independent slices.  Physical addresses are mapped across the slices using an undocumented hash function with cache line granularity.  I.e., consecutive cache lines will be mapped to different L3 slices.    The hash function was designed to spread the traffic approximately evenly across the slices no matter what the access pattern.  (The obvious exception is all cores accessing a single cache line, since a single cache line has to be mapped to a single L3 slice.)   The use of multiple "slices" is intended to increase bandwidth, not to reduce latency.  Unloaded latency is typically slightly higher with a multi-slice cache, since there is no (practical) way to prevent most of your accesses from being to "remote" L3 slices.   This is more than made up for by the increased throughput of the multi-slice L3.


https://software.intel.com/sites/products/collateral/hpc/vtune/performance_analysis_guide.pdf :: Core Memory Subsystem  

The level 1 caches have TLBs of 64 and 128 entries respectively for the data and instruction caches. There is a shared 512 entry second level TLB. There is a 32 entry DTLB for the large 2/4MB pages should the application allocate and access any large pages. There are 7 large page ITLB entries per HT. When a translation entry cannot be found in the DTLBs the hardware page walker (HPW) works with the OS translation data structures to retrieve the needed translation and updates the DTLBs. The hardware page walker begins its search in the cache for the table entry and then can continue searching in memory if the page containing the entry required is not found. Cacheline coherency in a multi core multi socket system must be maintained to ensure that the correct values for the data variables can be retrieved. This has traditionally been done through the use of a 4 value state for each copy of each cacheline. The four state (MESI) cacheline protocol allows for a coherent use of data in a multi-core, multi-socket platform. A line that is only read can be shared and the cacheline access protocol supports this by allowing multiple copies of the cacheline to coexist in the multiple cores. Under these conditions, the multiple copies of the cacheline would be in what is called a Shared state (S). A cacheline can be put in an Exclusive state (E) in response to a “read for ownership” (RFO) in order to store a value. All instructions containing a lock prefix will result in a (RFO) since they always result in a write to the cache line. The F0 lock prefix will be present in the opcode or is implied by the xchg and cmpxchg instructions when a memory access is one of the operands. The exclusive state ensures exclusive access of the line. Once one of the copies is modified the cacheline’s state is changed to Modified (M). That change of state is propagated to the other cores, whose copies are changed to the Invalid state (I). With the introduction of the Intel® QuickPath Interconnect protocol the 4 MESI states are supplemented with a fifth, Forward (F) state, for lines forwarded from on socket to another.  

```
Depends on processor frequency and DRAM speed among other things,
L1 access 				    ~4 cycles (64 bytes)
L2 access 				    ~10 cycles 
L3 CACHE hit, line unshared                 ~40 cycles
L3 CACHE hit, shared line in another core   ~65 cycles
L3 CACHE hit, modified in another core      ~75 cycles
remote L3 CACHE                             ~100-300 cycles
Local Dram                                  ~60 ns
Remote Dram                                 ~100 ns
```
Nowadays, latency to an adjacent L3 cache is smaller than the latency to the local memory.
https://www.nas.nasa.gov/hecc/support/kb/skylake-processors_550.html  
More recent,
The cache hierarchy of Skylake is as follows:
* L1 instruction cache: 32 KB, private to each core; 64 B/line; 8-way
* L1 data cache: 32 KB, private to each core; 64 B/line; 8-way; fastest latency: 4 cycles
* L2 cache: 1 MB, private to each core; ; 64 B/line; 16-way; fastest latency: 12 cycles
* L3 cache: shared non-inclusive 1.375 MB/core; total of 27.5 MB, shared by 20 cores in each socket; fully associative; fastest latency: 44 cycles


Data prefetches:  
https://software.intel.com/sites/default/files/managed/9e/bc/64-ia-32-architectures-optimization-manual.pdf 2.5.5.4
Data Prefetch to L1 Data Cache
Data prefetching is triggered by load operations when the following conditions are met:
• Load is from writeback memory type.
• The prefetched data is within the same 4K byte page as the load instruction that triggered it.
• No fence is in progress in the pipeline.
• Not many other load misses are in progress.
• There is not a continuous stream of stores.

L2 cache prefetch triggers wrt to stores are unclear.

**Hardware cache prefetching:** 
https://software.intel.com/content/www/us/en/develop/articles/intelr-memory-latency-checker.html intel Memory Latency Checker
https://software.intel.com/content/www/us/en/develop/articles/disclosure-of-hw-prefetcher-control-on-some-intel-processors.html  
https://stackoverflow.com/a/787758/11338006 enable/dissable h/w cache prefetch. Here likely you will need to use 0x1a4 offset
intel msr tools https://github.com/intel/msr-tools.git
```bash
modprobe msr
./rdmsr 0x1a4 #Should return 0 - All prefetching enabled
./wrmsr -a 0x1a4 0xf
./rdmsr 0x1a4 #all disabled
./wrmsr -a 0x1a4 0x0 #rest
```

https://software.intel.com/content/www/us/en/develop/articles/how-memory-is-accessed.html
