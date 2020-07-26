issues with hybrid memory systems (DRAM and NVDIMM):
1. Cache vs Main. memory: One of them as cache and other as main memory? or have two different memory spaces (dram and nvdimm) and expose that to the programmer. Having PCM is main memory and DRAM caching memory rows/blocks will improve performance, energy and endurance. Write filtering, dram absorbs multiple writes before writing once to PCM, also improve write latency. Similar to how DRAM is used with SSD, that amends the flash.
2. Granularity of data movement/management- fine or coarse: How data migration happen between DRAM and NVDIMM? at what page size? larger or smaller? high granularity means more mapping required - more meta data.
3. Hardware vs Software cooperation: Who does the management, hardware or software? or cooperative? There must be hardware support for speed.
4. When to migrate the data?
5. How to design a scalable and efficient large cache? For ex: if your DRAM is 16GB already and you use it cache 2TB phase change memory, then you have to manage a huge cache.  

Taken from [Onur Mutlu 16b slides](https://safari.ethz.ch/architecture/fall2019/lib/exe/fetch.php?media=onur-comparch-fall2019-lecture16b-emergingmemorytechnologies-afterlecture.pdf) #37.


One memory is fast and small and other is large and slow, which memory do you place the page in, to maximize system performance?
These memories are on different channels. Load should be balanced on both channels? if you are hell bent to operating through small memory because of its low latency, in high bandwidth scenarios you'll degrade performance. On top of that page migrations have performance and energy overhead.  
**Characterize data access patterns and guide data placement in hybrid memory**  
Sequential access: As fast as PCM as in DRAM (_check if this is true for Optane_) because of row buffers  
Random access: much faster in DRAM  
**Store random access data on DRAM and streaming data on PCM**  
if you are not going to reuse the data you might as well use it from PCM than bring it to DRAM (do cost analysis before migrating data from PCM to DRAM, both migration overhead and space overhead in DRAM should be considered). ["Row Buffer Locality Aware Caching Policies for Hybrid Memories"](https://people.inf.ethz.ch/omutlu/pub/rowbuffer-aware-caching_iccd12.pdf) did it in hardware, but it is possible to identify access patterns in software and employ similar techniques.  

>> There is one row buffer associated with every single memory bank (logical group across multiple chips in a Rank for memory parallelism to keep memory controller/bus busy). When a request is made the entire row of bank is placed in the row buffer (4kB, 8kB or 16kB in size) even when let's say 64 byte is requested. Memory controller is only making a request for 64 byte cache line. If application exhibits spatial locality then it'll make requests for other elements present in the row buffer (which will be faster).  

Weakness:
* They are all heuristics that consider only a limited part of memory access behavior: With more and more info you do better
* Do not directly capture the overall system performance impact of data placement decisions. If you want to place a page in DRAM or PCM, ideally you want to capture what is the performance impact of placing it in DRAM and not placing it on DRAM.  

A performance model that indicates where you should place the data: If your application requires two pages to make progress and only one of them is migrated to DRAM then net performance gain is zero. Page migration decisions need to consider Memory-Level-Parallelism.  

**[Utility-Based Hybrid Memory Management](https://people.inf.ethz.ch/omutlu/pub/utility-based-hybrid-memory-management_cluster17.pdf)**  
A generalized mechanism (_maybe ML can be applied here??_) that 1. Directly estimates the performance benefits of migrating a page between any two types of memory. 2. Places only the performance-critical data on fast memory. For each page, use comprehensive characteristics to calculate estimated utility (i.e., performance impact) of migrating page from one memory to the other in the system. Utility is the metric associated with each page and is the performance impact of migration. System migrates only pages with highest marginal Utility, i.e pages that improve system performance the most when migrated.
> For each page, estimate utility using a performance model. Migrate only pages whose utility exceeds the migration threshold from slow mem to fast mem. Periodically adjust migration threshold.  
Utility is estimated from:
* Application stall time reduction: How much would migrating a page benefit of the application that the page belongs to?
* Application sensitivity: How much does the improvement of a single application's performance increase the overall performance?

Opportunities:  
* **Merging of memory and storage**: A single interface to manage all data. Traditionally, data structures on DRAM as its byte addressable and filesystem f or disk as its block addressable. Now you have fast, byte addr and nonvolatile, so can you use a single software/programming interface to access all the data? _PMM uses access and hint information to allocate, locate, migrate and access data in the heterogeneous array of devices._ Have just load store interface for everything and don't waste time on system calls. _If you have fast device don't have slow software to manage it._
* New applications: Eg: ultra-fast checkpoint and restore.
* More robust system design: Eg: reducing data loss.
* Processing tightly-coupled with memory: Enabling efficient search and filtering.
* Crash consistency problems in persistent memory: Current, manually declaration of persistent components, each should need a new implementation of functions that guarantee consistency and third party code can be inconsistent. If you do a store directly to PM and if you get a crash you may end up in inconsistent state so you usually buffer the value somewhere and write the result back into PM when you know the transaction is done.
* Security and privacy issues in persistent memory  

[A Case for Efficient Hardware/Software Cooperative Management of Storage and Memory](https://users.ece.cmu.edu/~omutlu/pub/persistent-memory-management_weed13.pdf)  

[ThyNVM: Enabling Software-Transparent Crash Consistency in Persistent Memory Systems](https://people.inf.ethz.ch/omutlu/pub/ThyNVM-transparent-crash-consistency-for-persistent-memory_micro15.pdf)  
Goal: software transparent consistency in persistent memory systems.
Idea: Periodically checkpoint whole system state; recover to previous checkpoint on crash.  

>> **http://www.gem5.org**  
>> **https://github.com/CMU-SAFARI/ramulator**

***
>**Goal**: Provide large memory capacity while providing least latency by placing pages in fast memory using a statistical model that learns from access patterns.  
>**Assumptions**: NVM page size = DRAM page size and NVM row buffer = DRAM row buffer and NVM row hit/miss latency ~ DRAM row hit/miss latency (hit < miss). Memory address space partitioned between DRAM and NVM but writes are only made to DRAM (to maintain latency and NVM's endurance and energy consumption) and lookups are of the order L1/L2->L3->DRAM->NVM (Which means all metadata is stored in DRAM).    

**Page attributes**: (Once a granularity is set - 4kB lets say)
* How often is this page read from? Temporal locality effects on individual application perf
* Percentage of reads offered by this page at system level? Can estimate overall system perf improvement on migration to fast mem.
* Percentage of overall read hits by its application? High means most of application pages are already cached, if not then net performance will be low or zero due to high latency access to PCM, so strive to put it in fast mem.
* If entire row buffer/cache line of the page is being read by application? If so don't bother migrating, as this is a sequential read and there won't be performance gains on moving it to DRAM. If page in DRAM, then this row can likely be sent back to NVM.  

When a page is accessed the ML model will decide whether page should be migrated from PCM to DRAM or vice versa. You could speed up latencies even further by prefetching performance critical data from NVM to DRAM. Naive way would be to use above page level attributes as feature params and do binary classification of wether data should be in DRAM or NVM. If sequential reads align with PCM's row buffer size then don't consider them for migration as there might be no improvement in performance.  

**Potential problems**:
1. Cpu overhead of updating the metadata for all the pages.
2. For write heavy workload you will always be limited by DRAM bandwidth.
3. The cost of migrating random pages (cpu and bus load)
4. Cost of retraining the model    
***


**_Any implications of NVM on NUMA?_** 