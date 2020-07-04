* Why is sequential i/o better than random even in SSDs?
> Because 1. No repeated page faults 2. Not transferring bits (flash page) that will not be used 3. kernel might do prefetching and load page caches. 4. And alas, less seeks after all.
> But SSD controllers supports parallel read/writes unlike hdds. 
Each bit write in SSD has to rewrite the whole page and this can't be done in place and page writes are always done on empty pages. These can be attributed to write amplifications on SSDs.
Writing complete pages is better than writing data smaller than the page size, since the smallest SSD unit storage is a page.
After extensive random writes, an Flash Transport Layer runs out of free pages, which then does GC: reads, consolidates and writes active pages in free blocks and freeing blocks occupied stale pages.

https://medium.com/databasss/on-disk-io-access-patterns-in-lsm-trees-2ba8dffc05f9

FTL log structures your writes, so it better to align with it.

RUM conjecture - http://daslab.seas.harvard.edu/rum-conjecture/ and https://medium.com/databasss/on-disk-storage-part-4-b-trees-30791060741

Given the requirement as any two of them (write, read and mem) what is the best you can do for the third?

Using skiplist as memtable might make reads cheaper. A very good primer on RocksDb https://www.cockroachlabs.com/blog/cockroachdb-on-rocksd/
* Read amp - Number of key comparisons (BigO) + number of seeks + cost of decompressing data
* Write amp - Number of total bytes written (1 logical write can trigger multiple actual writes in-memory and on disk)
* Space/mem amp - ratio of size of data to size of database.
* Cache amp - Database size in cache to enable point query resolving to atmost 1 I/O read. http://smalldatum.blogspot.com/2018/03/cache-amplification.html

Monkey - reduce false positive rates for lower lsm levels as they have exponentially larger keys. Do this by increasing bits per entry in bloom as you grow in levels.
By doing this sum of false positive rates O(e^-x) becomes independent of number of levels which means point read costs does not degrade at log rate on lower levels. Stabilizes point read costs.

Dostovevsky - Since majority of reads are likely to happen to lower level (as they are exponentially larger) the merges happening at higher level do not help
improve read amp a lot and moreover the cost of merges across levels are equal (higher level more smaller merges, lower level less merges but large). Lazy level
applies tierd merge policy at higher level and leveled greedy policy for lower. Higher level wait until level fills up before merge (lower flush rate), lower level keep merging and strive
to maintain atmost single file. Without exponentially decreasing false positive rates from lower to higher level this optimization will not help as point access will be unifrm (O(log_R (N) * R * e^-x))).
Long range reads perform same but short range reads bounds are worse. For writes every entry for higher level just has 1 merge [O(1)] and R merge operations in lower level [O(R)]. 
Decreases the rate at which writes degrade.
> But with more runs in higher level without being merged, wouldn't their read amp increase?

FluidLSM - Tiering optimized for writes, Lazy leveling for writes and point queries and Leveling for short ranges. FluidLSM generalizes these into a tree by having two metrics that dictate
number of runs/files in higher and lower levels. Based on the type of workload tune the LSM to behave as one of the above (this is static not during run time), for example, if short range queries 
then set both the metric to 1 rendering a levelling strategy and if workload are writes then set higher metric to R and lower to 1, rendering a lazy leveling. This is modelling only worse case workloads 
for now and does not model for skews. Making tuning dynamic is challenging as workloads keep changing and you end up chasing for optimal strategy, tuning is also costly depending on what strategy you are
changing from and to.

Wacky - In LSM-bush differences in levels are doubly exponential now. Wacky generalizes all the above mentioned LSM strategy, including LSM-bush. Further decreases write degradations.

SIGMOD'16 - [Design Tradeoffs of Data Access Methods]( https://stratos.seas.harvard.edu/files/stratos/files/designaccessmethods-tutorial.pdf)
* logarithm access:
	* SB-Trees for sequntial disk access for long range intervals.
	* Bouded disorder access method (reduce index mem size) / Also refer to succinct DS
	* [LOCS uses SSD parallelism](https://dl.acm.org/doi/pdf/10.1145/2592798.2592804)
	* [BW-trees](http://www.cs.cmu.edu/~huanche1/publications/open_bwtree.pdf)
* Adaptive indexing / Differential updates
	* [Database cracking](https://stratos.seas.harvard.edu/files/IKM_CIDR07.pdf) - reorder data to be contiguous based on query to optimize reads
	* [Adaptive merging](https://www.vldb.org/pvldb/vol4/p586-idreos.pdf) - Tradeoff between index creation and read optimization as index layout converges for a specific workloads
	* Stepped merge and Materialized sort merge - are write optimized by buffering sorted updates in-mem and flushed as large immutable sorted runs.
* Space efficient
	* Upbit - Better update perf but reduced read perf (against bitmap) with more metadata.
	* Zonemaps and column imprints for effectively scanning/skipping.
	* [BF-Trees](https://dl.acm.org/doi/pdf/10.14778/2733085.2733094) use bloom filters as leaf nodes (B+ inner nodes) for approximate (sparse in this case) indexing, other probabilistic DS are quotient filters and cuckoo filters.
* High dimensional data
	* R trees and its variants (but they all suffer from curse of dimensionality)
	* iSAX and ADS for time series data.

BW-trees - http://www.cs.cmu.edu/~huanche1/publications/open_bwtree.pdf 

lmdb -  https://www.youtube.com/watch?v=tEa5sAh-kVk&index=3&list=PLSE8ODhjZXjakeQR57ZdN5slUu2oPUr1Y&t=178s and http://www.lmdb.tech/doc/
* If your workload is random-writes heavy, choose lsm
* If your workload is serial-writes heavy, both are similar
* If your workload is read-heavy (random or not) go for lmdb
* http://www.lmdb.tech/bench/ondisk/
However, LMDB maybe bad for write intensive workoads: (with a grain of salt)
- LMDB by default provides full ACID semantics, which means that after every key-value write committed, 
it needs to sync to disk. Apparently if this happens tens of times per second, your system performance will suffer.
- LMDB provides a super-fast asynchronous mode (`MDB_NOSYNC`), and this is the one most often benchmarked. Writes are super-fast with this. 
But a little known fact is that you lose all of ACID, meaning that a system crash can cause total loss of the database. Only use `MDB_NOSYNC` if your data is expendable.
- lmdb lacks a WAL so might not be good for write intensive loads (it uses shadow paging though to avoid in-place updates) 

SILK: Spikes in Log-Structured Merge Key-Value - They observed 99th percentile latency spiking upto 1 sec in write workloads (although they finish in memory/memtable). The spike is due 
to I/O bottlenecks caused by client operations and background compaction operations. More precisely, L0 is full and immutable memtables can't be flushed blocking writes. L0 is perhaps full
when lower layers are busy compacting (L0 -> L1 compaction is too slow) and taking over I/O bandwidth. Basically, no coordination or priority between levels compacting.
In RocksDB you can provid compaction bandwidth limit. SILK I/O scheduler prioritizes lower level flush (so L0 is never full), preempt high level compactions (so bandwidth exists 
for compaction/flash on lower levels), and allocates I/O opportunistically for higher levels (so compactions don't fall behind). sILK monitors client I/O bandwidth and triggers compactions
when load is low but continues to prioritize flush and L0 -> L1 compactions (RocksDB provides number of background threads you can set, so reduce it once client loads picks). Even with 0 client
load it is not a good idea to have lot of parallel compactions (Options::max_background_jobs) as finishing the job will be slower (SILK sets it to max 4).
You could potentially engineer a client load that avoid SILK opportunistic compactions making it hard for LSM to catch up and do it long enough you will see spikes.
RocksDB also allows compaction priority - https://rocksdb.org/blog/2016/01/29/compaction_pri.html
> Maybe make use of SSD parallel I/O to amortize compaction costs at time T, so that we can make room for client flush? but this still could block memTable -> L0 write as compaction is not ordered.
> How about occasionally merging immutable memtables in ordered non overlapping manner before writing to L0 (have higher RAM) to reduce write amp? So L0-L1 compaction will no different from others.

Fractal vs B+ vs LSM trees: http://www.pandademo.com/wp-content/uploads/2017/12/A-Comparison-of-Fractal-Trees-to-Log-Structured-Merge-LSM-Trees.pdf
> TokuDb - Shows fractal does better in asymptotic analysis compared to the other two.
So buffer the ops until memory/nodes are available in memory, once it is do all the operations in received order and do one write to disk. 
Operations are buffered in internal nodes, they are initially added to root node and will be pushed down to internal nodes (which will be eventually fetched from disk).
> Writes always go through root node though which can cause contention on multi threaded workload. LSM in general are more flexible, for example if you want to improve read amp then perform
more compactions and reduce layers/runs per layer and when you want to reduce write amp delay compactions as much as possible (But don't let compactions stale). 
> Dynamically Modify level merge multiplier bigger for reads and smaller for writes ^  

> **_(LSM-based Storage Techniques: A Survey)[https://arxiv.org/abs/1812.07527_**]**_  

Does NVDIMM also have flash like GC/page granularity behavior?

Yes Optane in memory mode has wear leveling, so some GC should be involved.
https://thessdguy.com/intels-optane-two-confusing-modes-part-2-memory-mode/
