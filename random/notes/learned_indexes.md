
###[The Case for Learned Index Structures](https://www.cl.cam.ac.uk/~ey204/teaching/ACS/R244_2018_2019/papers/Kraska_SIGMOD_2018.pdf)

B+ trees are basically regression trees (ML models), which 'predicts' the value given a key in O(log n). Indexing all keys is no efficient hence in a continuous series of data split logically by page size, only the beginning of the page is indexed. Hence, for a given key the value will be guaranteed to be between min-error of 0 and max-error of the page size. So this can be supplanted by a ML model (MLM) as long as it provides similar guarantees which gives O(1). For a new key B trees needs to be rebalanced, like MLM needs to be retrained.

> Can computations (multiplications in NN) be made as fast as traversing a tree? And what should the accuracy of the model be to be better than B tree?

They show a model can be faster for a point query on 100M keys (with a page size 100, making precision gain of 1/100 per node and log_100(N) traversals) as long as it has better precision gain than 1/100 per 50 * 8 arithmetic ops (cpu can do 8-16 SIMD/cycle). This assumes B tree pages are in cache, with cache miss happening more complex MLMs are possible. Further, utilizing GPUs can also improve latency (as performance of if-statement CPU executions are stagnated while GPU multiplications are growing). This paper however focus on ML impact without hardware changes.

The distribution of the data can be modeled as CDFs. To validate this, they initially build a 2 layer (32 neurons/layer) network using ReLU with timestamps as input features and positions in sorted arrays as labels. But they realized,
1. Tensorflow and python is designed for large models, so this performed worse than decision trees (B or Binary)
2. Models efficiently approximate the general shape of CDF but have problems at individual level. NNs require significantly more CPU and space to reduce error from a narrow scope (until this scope its fine) to an individual position (the last mile). In contrast, trees overfit the data naturally.
3. Trees keep their nodes in cache making computation efficient, but NNs require large number of multiplications to calculate weight.
To overcome these, they introduce LIF and RMI:  
**Learning Index Framework** automatically generates C++ index structures by extracting weights from a learnt model (NN) so that Tensorflow need not be used for inference (to avoid its instrumentation overheads). LIF can be further used to optimize them based on different index configs (ML models, page size and search strategies) but these introduce additional overheads. Besides from auto vectorization by the compiler no special SIMD [intrinsics](https://stackoverflow.com/a/2268599/11338006) are used.  
**Recursive Model Index**: Models perform better when the ratio of search space to filter space is small (With 2 layers, 100M -> 10K and 10K -> 100 is easier compared to 100M -> 100). Based on this observation, they propose a recursive regression model. That is, a hierarchy of models, where at each stage the model takes key as the input and based on it picks the next model, until final stage predicts the position. Today, they train it stage wise. Essentially, each model is responsible for certain area (need not be equally divided) in the key-space to make a better prediction. Read sec. **3.2** for advantages of this approach.  
**Hybrid index**: The advantage of above multi level model is that each stage can be different models or same with different configs. For example, top layer can be small ReLU NN to learn wide range of data distribution and at the bottom there can be large no. of simple regression models which are inexpensive in space and execution time. Moreover, even B+ trees can be in the last layer if the last subset of data is hard to learn. Hybrid indexes will bound the worst case performance of learned indexes to that of B-trees by replacing all models with trees in every level if data is particularly hard to learn.  

General convention is that binary search or scanning for records for small payloads is the fastest strategy to find a key but models actually predict the position of the key and not just a page (region) it might be in, models could be faster. There is much scope for research in indexing strings.  

[AVX instructions](https://lemire.me/blog/2018/09/07/avx-512-when-and-how-to-use-these-new-instructions/), [AVX-turbo](https://github.com/travisdowns/avx-turbo), [Intel AVX-512](https://software.intel.com/sites/default/files/managed/c5/15/architecture-instruction-set-extensions-programming-reference.pdf)  

**Learned hashmaps**: Hashtables are swamped with hash conflicts, requiring new strategies to resolve them (like linked list, buckets, etc) but this affects read performance and increases index size. If a model perfectly learned the empirical CDF of the keys, then no conflicts would exist. Hence. benefits of learned hash function over traditional HF, which maps a uniform space depends on how accurately the model represents the observed CDF and hash map architecture (conflict resolution policy, slots, value/payload, etc).
> **_Finally, we see the biggest potential for distributed settings. For example, NAM-DB [74] employs a hash function to look-up data on remote machines using RDMA. Because of the extremely high cost for every conflict (i.e., every conflict requires an additional RDMA request which is in the order of micro-seconds), the model execution time is negligible and even small reductions in the conflict rate can significantly improve the overall performance._**  

**Learned bloom filters**: Both range and point indexes learnt the distribution of keys but this needs to learn a function that separates keys from everything else. The model is trained on a dataset of keys and non-keys (historical or randomly generated). One approach is to frame the existence of key as a binary classification task (i.e whether x is key or non-key), to deal with non zero FNR an overflow BF is introduced for a subset of keys. Another way to building existence indexes is to learn a hash function with the goal to maximize collisions among keys and among non-keys while minimizing collisions of keys and non-keys. They show learned bloom filters perform well with smaller memory footprint and FPR.
[Locality sensitive hashing](https://en.wikipedia.org/wiki/Locality-sensitive_hashing)

**Future work includes**: Other ML models, Multi dimensional models, learned algorithms and GPU/TPUs.  
_Refer to Appendix D_  
Although, they show inserting key in middle is O(1). It was not clear how data is moved to reserve space for the new item or what the cost of it would be.  
> D.2 **_With more complex models, it might actually be possible to learn the actual pointers of the pages. Especially if a filesystem is used to determine the page on disk with a systematic numbering of the blocks on disk (e.g., block1,...,block100) the learning process can remain the same._**  

Why RRM when you can increase the layers of NN to improve accuracy?  
Complex models will increase the number of multiplications and additions (expensive to train and execute)
***

MLM to supplant route tables??  
Routing tables are usually have 64k to 100k or more entries, and incoming packet destination address is ANDed with every entry to identify the right interface the packet to be sent to. If nothing matches send it to default.
This is O(N) we could make it O(1) with NN classifiers where interface are the labels and destination address is the feature. Probably modern ASICs do this fast but this approach could help commodity switches.

The mystery of Triquetra:  
Read and writes should be sequential, data should be sorted for range queries but input data is not ordered. While doing all of this the index shouldn't take a lot (more than the data itself) of memory.