# CPP-Tutorials

This attempts to give a broad overview on C++ language and showcase the
benefits that come with its OOP features as compared to C language.

In this tutorial, binary trees are considered. The learner is supposed to build
on this knowledge and practice with other data structures and related
algorithms.

In addition to learning C++, the learner will also gain skills in using CMAKE
as a cross-platform build generation system and GoogleTest for testing.

Note that in case of AVLNode, it can be further simplified while dealing
with duplicates, and this is left for the learner to practice.

Also, the same applies for the Tree. In general, we can build a more simplified
tree that utilizes only a KeyType which includes what now add in ValueType
and thus making the tree simpler for most use cases. In this case, the Node
itself can be defined within Tree class template. The idea behind using current
Node and Tree templates with KeyType and ValueType is for teaching / learning
purposes and becoming more skilled at template metaprogramming.