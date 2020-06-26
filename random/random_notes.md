* Why is sequential i/o better than random even in SSDs?
>> Because 1. No repeated page faults 2. Not transferring bits (flash page) that will not be used 3. kernel might do prefetching and load page caches. 4. And alas, less seeks after all.
>> But SSD controllers supports parallel read/writes unlike hdds. 
Each bit write in SSD has to rewrite the whole page and this can't be done in place and page writes are always done on empty pages. These can be attributed to write amplifications on SSDs.
Writing complete pages is better than writing data smaller than the page size, since the smallest SSD unit storage is a page.
After extensive random writes, an Flash Transport Layer runs out of free pages, which then does GC: reads, consolidates and writes active pages in free blocks and freeing blocks occupied stale pages.
https://medium.com/databasss/on-disk-io-access-patterns-in-lsm-trees-2ba8dffc05f9
RUM conjecture - http://daslab.seas.harvard.edu/rum-conjecture/ and https://medium.com/databasss/on-disk-storage-part-4-b-trees-30791060741
Using skiplist as memtable might make reads cheaper. A very good primer on RocksDb https://www.cockroachlabs.com/blog/cockroachdb-on-rocksd/

Does NVDIMM also have flash like GC/page granularity behavior?
