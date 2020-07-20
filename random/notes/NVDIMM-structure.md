issues with hybrid memory systems (DRAM and NVDIMM):
1. Cache vs Main. memory: One of them as cache and other as main memory? or have two different memory spaces (dram and nvdimm) and expose that to the programmer. Having PCM is main memory and DRAM caching memory rows/blocks will improve performance, energy and endurance. Write filtering, dram absorbs multiple writes before writing once to PCM, also improve write latency. Similar to how DRAM is used with SSD, that amends the flash.
2. Granularity of data movement/management- fine or coarse: How data migration happen between DRAM and NVDIMM? at what page size? larger or smaller? high granularity means more mapping required - more meta data.
3. Hardware vs Software cooperation: Who does the management, hardware or software? or cooperative? There must be hardware support for speed.
4. When to migrate the datat?
5. How to design a scalable nd efficient large cache? For ex: if your DRAM is 16GB already and you use it cache 2TB phase change memory, then you have to manage a huge cache.  
Taken from [Onur Mutlu 16b slides](https://safari.ethz.ch/architecture/fall2019/lib/exe/fetch.php?media=onur-comparch-fall2019-lecture16b-emergingmemorytechnologies-afterlecture.pdf) #37.


One memory is fast and small and other is large and slow, which memory do you place the page in, to maximize system performance?
These memories are on different channels. Load should be balanced on both channels? if you are hell bent to operating through small memory because of its low latency, in high bandwidth scenarios you'll degrade performance. On top of that page migrations have performance and energy overhead.  
**Characterize data access patterns and guide data placement in hybrid memory**  
Sequential access: As fast as PCM as in DRAM (_check if this is true for Optane_) because of row buffers  
Random access: much faster in DRAM
**Store random access data on DRAM and streaming data on PCM**  
if you are not going to reuse the data you might as well use it from PCM than bring it to DRAM (do cost analysis before migrating data from PCM to DRAM, both migration overhead and space overhead in DRAM should be considered). ["Row Buffer Locality Aware Caching Policies for Hybrid Memories"](https://people.inf.ethz.ch/omutlu/pub/rowbuffer-aware-caching_iccd12.pdf) did it in hardware, but it is possible to identify access patterns in software and employ similar techniques.  

Weakness:
* They are all heuristics that consider only a limited part of memory access behavior: With more nad more info you do better
* Do not directly capture the overall system performance impact of data placement decisions. If you want to place a page in DRAM or PCM, ideally you want to capture what is the performance impact of placing it in DRAM and not placing it on DRAM.  

A performance model that indicates where you should place the data: If your application requires two pages to make progress and only one of them is migrated to DRAM then net performance gain is zero. Page migration decisions need to consider Memory-Level-Parallelism.  

**[Utility-Based Hybrid Memory Management](https://people.inf.ethz.ch/omutlu/pub/utility-based-hybrid-memory-management_cluster17.pdf)**  
A generalized mechanism (maybe ML can be applied here??) that 1. Directly estimates the performance benefits of migrating a page between any two types of memory. 2. Places only the performance-critical data on fast memory. For each page, use comprehensive characteristics to calculate estimated utility (i.e., performance impact) of migrating page from one memory to the other in the system. Utility is the metric associated with each page and is the performance impact of migration. System migrates only pages with highest marginal Utility, i.e pages that improve system performance the most when migrated.
> For each page, estimate utility using a performance model. Migrate only pages whose utility exceeds the migration threshold from slow mem to fast mem. Periodically adjust migration threshold.  
Utility is estimated from:
* Application stall time reduction: How much would migrating a page benefit of the application that the page belongs to?
* Application sensitivity: How much does the improvement of a single application's performance increase the overall performance?