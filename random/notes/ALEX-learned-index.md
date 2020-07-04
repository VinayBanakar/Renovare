### [ALEX: An Updatable Adaptive Learned Index](https://arxiv.org/pdf/1905.08898.pdf), [repo](https://github.com/microsoft/ALEX)

Learned indexes (Kraska et al [1]) was limited to handle lookups on read only data, with no support to update operations and it used densely packed array which works well for static datasets but can result in high costs for shifting records if new records are inserted. Also, the prediction accuracy of the models can deteriorate as the data distribution changes over time, requiring repeated retraining.  

ALEX is a fully dynamic data structure that simultaneously provides efficient support for point lookups, short range queries, inserts, updates, deletes, and bulk loading.  
Aims:
1. Insert time should be competitive with B+Tree
2. lookup time should be faster than B+Tree and Learned Index
3. index storage space should be smaller than B+Tree and Learned Index
4. data storage space (leaf level) should be comparable to dynamic B+Tree  

* **storage layout**: ALEX builds a tree, but allows different nodes to grow and shrink at different rates. To store records in a data node, ALEX uses an array with gaps, a Gapped Array, which (1) amortizes the cost of shifting the keys for each insertion because gaps can absorb inserts, and (2) allows more accurate placement of data using model based inserts to ensure that records are located closely to the predicted position when possible. For efficient search, gaps are actually filled with adjacent keys. This is a space-time tradeoff.
* **Search**: it uses model-based inserts combined with exponential search starting from the predicted position. _This always beats binary search when models are accurate, as they predict a close enough position._ Because of which no need to store error bounds in RMI model [1].
* **Accurate models with dynamic data distributions**: ALEX achieves this by exploiting adaptive expansion, and node splitting mechanisms, paired with selective model retraining, which is triggered by intelligent policies based on simple cost models.

**Data layout**: Much like B+ trees, ALEX stores data records in leafs called data nodes. These nodes follow a linear regression model, which maps keys to position, and two fixed size Gapped arrays (for keys and payloads/pointers to variable size records). Gapped arrays fill the gaps with closest key to the right of the gap. To improve scanning a bitmap in maintained in the data node which tracks whether each location in node is occupied by gap or key. Internal nodes store a linear regression model and an array containing pointers to children nodes. Each internal node has a max node size set. ALEX uses models to 'compute' the location in pointers array of the next child pointer to follow. The role of the internal nodes in ALEX is to provide a flexible way to partition the key space so that linear models can accurately fit data nodes with roughly linear distribution. If the CDF of a key space is linear, it does not need further RMI resolution hence becomes a data node but if the CDF is non linear, you get a internal node for that space. In a sense, the tree height is adaptive to the workload unlike [1].  

**Access methods**:
_Lookups_ are done by traversing the computed pointers to children in internal nodes (perfect accuracy), until a data node is found. Here do exponential search for the key, once found look at the same position in payload array for value and if not found return null. For _range_, find the position and data node where first key is not less than starting key, then scan forward using bitmap (to skip gaps) and pointers in the node (to goto next node) until end key is found.  
For _Inserts in non-full data node_, traverse the same way to a data node as above, the model in data node tells the position to be inserted at. If position is not correct (resulting in unsorted array), do a exponential search to find correct position. If the position is a gap, insert the element if not create a gap by shifting the elements by 1 position in the direction of the closest gap and insert in the newly created gap.  
_Inserts in full data node_: As number of gaps decrease the inserts degrade, so ALEX does not wait until the Gapped array is 100% full. Density is the fraction of positions that are filled by elements, with d_l and d_u being upper and lower density bounds. A node is full when the next insert exceeds d_u. Density of the gapped array provides a way to tradeoff between space and lookup performance.
- Node expansion: To expand data node that contains n keys, a new larger gapped array is allocated with n/d_l size. Next scale or retrain te liner regression model and do model based inserts to the new array. New array will have d_l density.
- Node split: In splitting side way, if internal nodes array is not at max, then split the data node into two and update the parent internal node pointer array (there might even exist redundant pointers to same data node which could be replaced with). Splitting down converts data node to an internal node with two data nodes as children. [See Fig. 5]  
_Cost models_: Since the lookup and insert performance is directly correlated to average number of exponential searches and average number of shifts per insert, they are used as metrics to decide whether node should be expanded or split. In addition, ALEX uses a TraverseToLeaf cost model to predict the time for traversing from the root node to a data node to capture the cost of traversal. The two metrics used for this are, depth of node being traversed to and total size (bytes) inner nodes and data nodes metadata (i.e everything except for keys and payloads). The decision to either scale or retrain the model is based on a check that compares expected cost to empirical cost. If there isn't a significant deviation ( > 50%) then data nodes are scaled and no model retrained required, if there is then data nodes are expanded and retrained. Since making optimal split (any 2^n) decision is time consuming, ALEX just splits data node to two. The main reason for deviation is the inserted keys may not follow distribution of existing keys.  
_Updates_: Updates that modify the key are implemented by combining an insert and a delete. Updates that only modify the payload will look up the key and write the new value into the payload. Like B+Trees.  

> **_A key that is lower or higher than the existing key space would be inserted into the the left-most or right-most data node, respectively. A series of out-of-bounds inserts, such as an append-only insert workload, would result in poor performance because that data node has no mechanism to split the out-of-bounds key space._**

They handle this by expanding the root node (adding multiple data nodes as required with associated pointers in internal/root node array) whilst not affecting existing nodes. If max array size is hit for the root node, then ALEX creates a new root node, where its first child pointer points to old root node and so on. ALEX detects append only insert workload by maintaining the value of the maximum key in the node and keeping a counter for how many times an insert exceeds that maximum value.If most inserts exceed the maximum value, that implies append-only behavior, so the data node expands to the right without doing model based re-insertion.  

Fanout tree: Is a complete Binary Tree which helps decide the fanout for a single RMI node. A fanout of 1 means the RMI should be a data node.

**Complexity**: See section 5.2  
```
Let m be the maximum node size, defined in number of slots (in the
pointers array for internal nodes, in the Gapped Array for data nodes)
and let p be the minimum number of partitions such that when the key
space s is divided into p partitions of equal width, every partition
contains no more than md_u keys.
Both lookups and inserts do TraverseToLeaf in ⌈log_m(p)⌉ time. Within
the data node, exponential search for lookups is bounded in the worst
case by O(log(m)). In the best case, the data node model predicts the
key’s position perfectly, and lookup takesO(1) time.
```

> **_We learned that the added complexity of using a neural net for the root model usually is not justified by the resulting minor performance gains, which we also independently verified._**

[1] ./learned_indexes.md  

***
https://news.ycombinator.com/item?id=23584426