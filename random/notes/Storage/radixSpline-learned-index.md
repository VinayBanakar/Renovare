### [RadixSpline: A Single-Pass Learned Index](https://arxiv.org/pdf/2004.14541.pdf), http://learned.systems/rs

A learned index that can be built in a single pass over sorted data and is competitive in size and lookup performance compared to other learned indexes (RMI [1]). They argue updates need not be supported by learned indexes (like ALEX [2] and PGM do) but should be able to build them efficiently. One example is LSM trees, where updates are inserts and you can perform training during merges/compaction as merges themselves expensive, building a one pass learned index could only incur negligible constant overhead. However, they claim existing learned indexes do not follow efficient build. ALEX, PGM and FITing-tree (current learned indexes) all have expensive bulk loading, either require multiple training passes or significant effort per new element and complex[3]!?. Also in general other learned indexes exhibit variable latencies and large tail latency (one unlucky key).  

> Notably, the proposals of FITing-Tree [6] and PGM [5] also support single-pass builds. However, for both of these indexes the amount of work per element is logarithmic in the number of levels (similar to inserts in a BTree). With RS, we propose the first single-pass learned index with a constant amount of work per new element.

**Index design**:
> Being an ordered index, RS supports both equality and range predicates (e.g., lower bound lookups). RS is built in two steps. First, a linear spline is fit to the CDF of the data that guarantees a certain error bound. This results in a set of spline points which can be significantly smaller than the underlying data. Second, we build a radix table (a flat radix structure) that serves as an approximate index into the spline points.  
RS maps a key to index on data sorted by lookup key in a flat array. RS index contains two components:
* Spline points: is the subset of keys, so that [spline interpolation](https://www.youtube.com/playlist?list=PLDAB608CD1A9A0D55) for any lookup key will result in a predicted lookup location within a preset error bound. linear interpolation is used between two spline points that surrounds the key to predict its actual location. Because the spline interpolation is error-bounded, only a (small) range of the underlying data needs to be searched. The placement of spline knots/points on the CDF function is the learned params done greedily. Identifying optimal spline knots over CDF is an open question (a paper exists that shows you can do it in nlog(n) but no implementation).
* Radix table: helps to quickly locate the correct range of spline points to be examined for a given lookup key. They limit the possible spline points to search over for every b-length prefix of a prefix lookup key.
> The build process itself is very straightforward and extremely fast: we first allocate an array of the appropriate size (2 r many entries), then we go through all spline points and whenever we encounter a new r-bit prefix b, we insert the offset of the spline point (a uint32_t value) into the slot at offset b in the radix table. Since the spline points are ordered, the radix table is filled in consecutive order from left to right. As an optimization, we eliminate common prefix bits shared by all keys when building the radix table.  

**Lookups**:
> We first extract an r-bit prefix b of the lookup key. Then, we use the extracted bits b to make an offset access into the radix table retrieving the two pointers stored at positions b and b +1. These pointers define a narrowed search range on the spline points. Next, we search this range for the two spline points surrounding the lookup key using binary search. Subsequently, we perform a linear interpolation between these two spline points to obtain an estimated position p of the key. Finally, we perform a binary search within the error bounds (p ± e) to find the first occurrence of the key. _More predictable lookup performance compared to RMI but RMI's average lookup performance is better._   

> **_LSM Performance. To validate the applicability of RS to LSMs, we performed a preliminary experiment where we substitute the BTree index with a RadixSpline in RocksDB. We use the osmc dataset and executed 400 M operations, 50% reads and 50% writes (cf. Figure 5). When using RS, the average write time increased by ≈ 4%, but the average read time decreased by over 20%. The total execution time fell to 521 seconds from 712 seconds with a BTree. In addition, the RS variant used ≈ 45% less memory, potentially creating space for larger Bloom filters or increased caching. While obviously preliminary, this experiment indicates potential benefits of RS in LSMs._**  

[Benchmarking Learned Indexes](https://arxiv.org/pdf/2006.12804.pdf):  
In Figure 7, they show that RMI (blue) and RS (green) are the pareto front, with one exception ins osm workload. Here, the red line (RBS - Radix binary Search) is the pareto front.  

**Current work**:
* Auto tuning of spline error(ε) and number of radix bits(r).
* LSM integration (rocksDB), at the compaction phase build the single pass over the merged output and build an RS index.
* Better outlier handling

Somehow they don't explain much about how inserts are done, if they are simply appended to the array then the assumption is that data distribution won't vary too much i.e. the array need not be reshuffled to maintain a proper CDF as it'll remain sorted.
**Inserts**: 
1. Adjust the spline knots to have a good approximation of the CDF after inserts, but to honor the error bound the aprox. model should be precise in terms of this error bound. Updating is pretty straight forward. If inserts are being in the middle its preferred to do a single pass over the array but appends are simple, since its a single pass you keep iterating for the new data and you can update it for append-only inserts.

> **_[LSM-based Storage Techniques: A Survey](https://arxiv.org/abs/1812.07527_**)_**

[FITing-Tree](https://arxiv.org/pdf/1801.10207.pdf): B tree on top of linear regression segments

> Don't be fooled by the many books on complexity or by the many complex and arcane algorithms you find in this book or elsewhere. Although there are no textbooks on simplicity, simple systems work and complex don't. - Jim Gray [3]

[1] ./learned_indexes.md  
[2] ./ALEX-learned-index.md  

***
RocksDB does binary searches over block indexes, that are maintained per SSTable. Whenever you do SSTable you do a binary search (this is essentially the lookup).

> How to cluster the data into pieces which are statistically correlated or query correlated in the past, today we try to redo whole search problem for every query as it comes but in practice most queries follow patterns. Patterns dictated by the params of the application and it's all about finding the right cluster of elements which satisfies the answer.

Qd-trees - https://arxiv.org/pdf/2004.10898.pdf  
Given a table you want to want the find the optimal clustering/block based on the data statistics and query workloads, so next time you can minimize the number of IOs.  

[The Case for a Learned Sorting Algorithm](https://dl.acm.org/doi/pdf/10.1145/3318464.3389752)  
An RMI model (could use RS) to fit a CDF over a sample of the data and use that for an approximate placement of the elements in a sorted array and do one pass of insertion sort to correct any model errors.  

PGM-index - https://pgm.di.unipi.it/