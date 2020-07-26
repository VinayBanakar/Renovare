
***
[An Empirical Guide to the Behavior and Use of Scalable Persistent Memory](https://www.usenix.org/system/files/fast20-yang.pdf)  

Optane is more than a slow dense persistent DRAM.
Optane has [two modes](https://imgur.com/a/5SxyEvJ) it can operate in.
**App Direct mode: (Optane DC PMM)**   
OS differentiates between DRAM and Optane. You can decide whether data should be DRAM or Optane resident. You set up PM using file system, then you map it into memory space (DAX mode/direct access mode) and then you can do cache line load stores to it like dram. This PM is cache coherent and can be used in DMA and RDMA. Cache coherency across multi socket system, the hardware will tag cache lines on a cpu if a different socket cpu modified it, much like same mechanism used for memory. [see bottom for more]  

>> **_Optane provides in-place persistency: NANDs can't erase a page you will have to erase a big block, in Optane you can erase at smaller granularity (cache line size) and just write it in place. Garbage collection or write amplifications (other than 256B block aggregation of 4 64B cache lines, the block can be read-modify write though at device though unlike SSD) concepts don't exists in Optane DC. This will give consistent performance. They do wear levelling in the background when transactions happen though_** https://imgur.com/a/qqzapYw

Optane internals:  
When CPU issues a write, it will land in the Write Pending Queue (WPQ) in the [integrated Memory Controllers iMC](https://qphs.fs.quoracdn.net/main-qimg-0064ad079efb5b2095f76c10f0b22860.webp) and WPQ will gradually issues stores to the Optane device. On the device, Optane Controller which is responsible for scheduling writes to the media will receive the writes. The controller will issue writes in 256 byte blocks, but since CPU cache lines are 64 bytes the controller is responsible for merging 4 cache lines before it can issue a block write to media. Optane buffer on this controller will facilitate this merge, the buffer itself is about 16k in size. Also Address Indirection Table (AIT) is responsible for translating physical address to device address within the device. This table resides on the Optane media but is also cached on the on device DRAM. (Asynchronous Dram Refresh (ADR) domain)[https://imgur.com/a/pjB74Pp], is where everything in this domain will have enough standup power to flush the write pending queue to optane controller and optane buffer to optane media on power outage. Basically once the write hits WPQ, the hardware guarantees persistance. Optane is interleaved across NVDIMMS at 4KB granularity.  

* Sequential read - Optane 2x slower than DRAM (no access amplification)
* Random reads - Optane 3x slower than DRAM
* Single writes - Same (for both ntstore and clwb)  

_Optane bandwidth performance:_
* Scalable reads - Non-scalable writes (write bandwidth maxes out at 5 threads). Likely the write size bellow is 256B   
Maximum bandwidth: (in GB/s) [Check paper for hardware specs]  
| Media    | Read           | Write (ntstore)| Write (clwb)    |
| ---------|:--------------:|:--------------:|:---------------:|
| DRAM     | ~100 (20 thrds)| ~75  (24 thrds)| ~45   (24 thrds)|
| Optane-NI| ~6  (4-5 thrds)| ~2  (2-5 thrds)| ~2     (2 thrds)|
| Optane   | ~40  (20 thrds)| ~12 (2-5 thrds)| ~10 (7-12 thrds)|  
* Access size matters - In DRAM it doesn't matter how big or small you access size is it'll give you same bandwidth. In contrast, for Non Interleaving Optane bandwidth increases up until access size is 256B after which all access are stable (efficiently used).In interleaving Optane, peaks at 512B but valleys at 4K and again starts climbing after 4k.

**Advisories for using Optane**:  
1. Avoid small random accesses - If your access is smaller that 256B, it'll be blown up to 256B (block size) by which you'll lose b/w. Optane buffer is used to merge the adjacent accesses/writes. If you have large working set (i.e. bigger than Optane buffer ~16KB) per dim then you will increase write amplifications. Sequential accesses are good as you'll be using on device cache.
2. Use non temporal stores (ntstores) for large writes - Three ways to write, ntsore bypasses the cache hierarchy and is issues directly to the dimm, store and then evict the cache line using cache line write back (clwb), and finally do regular store and eventually it'll go out from the cache to the media. For _store + clwb_ you need to fetch the cache line from media, do the store and then evict the cache to media (becomes a read + write for every write, uses up double the bandwidth). With just store, same issue plus cache eviction policy probably not optimized (introduces randomness) for sequential pattern. Turns out (for 6 threads) _ntstore_ (12.5 GB/s) has 2x _store+clwb_ bandwidth (GB/s). Flush instructions have lower latency for small accesses, but ntstore has better latency for larger accesses. Using ntstore also avoids an additional read of the cache line from Optane memory, resulting in higher bandwidth.
3. Limit the number of threads accessing a single NVDIMM - Due to contention at two places, at Optane buffer and at iMC. More threads = access amplifications = lower bandwidth. A set of sequential access will have randomness introduced, because if multiple threads are accessing single NVDIMM, they trash on the on device cache which means Optane buffer misses the opportunity to merge adjacent 64 byte cache lines into 256 byte blocks. The short queues on iMC can be clogged. The contention is largest when the random access size is equal to the interleave size, if thread's access size is > the interleave size then it's access will be spread across multiple dimms whereas if we're hitting exactly at the interleaving size then these create bursts of access that all hit the same MC and the same dim. Since media is slow and iMC has small queues there is b/W dip at 4k access size (since interleave size is 4k).
4. Avoid mixed and multi-threaded NUMA accesses. NUMA effects impact Optane more than DRAM. Mixed workloads as supposed to pure reads or pure writes to remote optane, there will be degradation in performance.

**Memory mode:**  
DRAM is acting as cache for Optane on the same channel. The large pool of memory the OS sees is actually Optane cached by DRAM. DRAM acts as the direct map cache for Optane. A DRAM cache hit latency will be same as DRAM latency. If DRAM cache miss then latency will be DRAM access + Optane access. So writes aren't write though to Optane, but cache lines will be evicted some time later so it will be flushed to Optane then. Generally advised ratio of Optane to DRAM is 4/8:1 or 16:1 ratio.

***

A raid system on Optane? There is no hardware support for this today because if you RAID on one system and you lose it there'll be no point, hence doing this using RDMA across nodes will be helpful.

Storage over app direct mode: This is a driver on top of app direct mode (byte accessible) and use it like a storage device (operates in blocks like SSD/HDD). Higher endurance than enterprise class SSDs and high performance block storage (low latency, higher b/w, high IOPs). Optanes can share the same channel a DRAM is using and for Xeon upto 6 modules per CPU is supported.

Transactional DDR (DDR-T): This protocol runs on the same DDR4 physical bus but time shared with DRAM traffic. When CPU needs some data from Optane, it will tell ask it for the data then optane will retrieve it and when its about ready, optane controller will send a request command to the host requesting bus access. Host/cpu being the bus master will grant the access and Optane will load the data on the bus which will be consumed by the cpu. The access size is 64 bytes just like DRAM.    

Intel Optane DC Persistent Memory Architecture Overview - https://youtu.be/BShO6h8Lc1s  
DRAM memory timings - https://youtu.be/o59V3_4NvPM  
Profiles workloads and tune (gives miss rates and profiles Optane) - https://software.intel.com/content/www/us/en/develop/tools/vtune-profiler.html  