# System-R-Implementation

Implementation of the System-R paper for query execution in C++.

https://15799.courses.cs.cmu.edu/spring2025/papers/02-systemr/selinger-sigmod1979.pdf

Where the paper made a specific design choice, we take it — nested loop + merge join only, B-tree as the only index, manual `UPDATE STATISTICS`. LMDB is chosen specifically because it gives us a real page-oriented, B-tree-based storage engine (matching the paper's actual RSS model).
