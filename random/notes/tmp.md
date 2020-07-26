 With the proliferation of commodity SSDs in this decade, NVDIMs and Nvme are here to replace it as OS cache layer.

 > Improving the hit ratio in the storage system will no necessarily improve the average response time. This can be justified by the fact that a caching algorithm with a lower hit ratio that mostly redirect sequential requests to HDDs will likely provide higher performance than a caching algorithm with higher hit ratio that mostly redirect random requests to HDDs. This is due to HDDs demonstrate at least two orders of magnitude higher performance in sequential requests compared to random requests
 
 if you have lot of sequential writes, would writing directly to SSDs without serious performance degradation help increase NVM/NVDIMMs lifetime (their by reducing cost)

 The age old attempt to adapt data placement as quickly as possible by predicting workload patterns for best performance.  
There has been about 5 decades of heuristics for deciding what data/pages must be kept or replaced for thousand different workloads.

 Does the caching approach exhibit poor generality?

Prioritize file system metadata requests based on their impact on overall performance for a workload.

Traditional cache systems migrates data page to cache on miss. Most basic ones are LRU or its variant.

In cloud based system designs you should try to avoid file-based caching, as it makes cloning and auto-scaling more difficult. Cache objects.

LRU is bad for sequential flooding and so is clock.
Clock replacement policy - You basically two chance before being replaced so that frequent pages are retained.
two queue replacement policy - one queue behaves like LRU and other is for sequential  

Optimize for both recency and frequency, regret ratio  https://stackoverflow.com/a/33928539