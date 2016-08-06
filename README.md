First attempt at a trie structure. Much of the design was modeled after the std::map.

Some performance tests are included comparing basic operations to the std::map. The trie outperforms in certain tests in expected ways, but is heavy on memory use. Looking for ways to improve.

Non-exhaustive list of todos:
1) finish iterators, possibly change what represents the 'end' of the trie, improve speed of iterations
2) implement copying and comparison of whole tries
3) reduce memory footprint
4) experiment with different methods of acquiring an index from the alphabet
5) more exhaustive testing
