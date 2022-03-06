# Cluster
C++ containers that extend the concept of [eastl::segmented_vector](https://github.com/electronicarts/EASTL/blob/master/include/EASTL/segmented_vector.h) but allow the segments to be dynamic in size.

`eastl::segmented_vector` uses what is more or less an invasive doubly linked-list of array blocks, where the size of the arrays are the same for every 'segment' and are determined at compile-time. The benefits of this are:
- Elements do not move in memory when the size of the container grows, because the memory is not reallocated
- Still maintains some contiguity due to the use of arrays.
but comes at the cost of a slightly more complicated iteration pattern and fragmented memory.

For these Cluster containers, we swap the concept of a `segment` for a `cluster` which does not have its size determined at compile time, meaning that we can use a scaling allocation pattern. By default, the containers double the size of any new clusters that are added, meaning that you end up with the order of `log2(size)` clusters compared to `size / segment_length` segments -- the result being a reduction in fragmentation and an increase in relative contiguity. This is similar to the type of allocation pattern found in [plf::colony](https://github.com/mattreecebentley/plf_colony).

- **cluster_vector** is a cluster implementation of `eastl::segmented_vector`
- **cluster_map** is a cluster implementation of a slot-map or handle-map, and has some similarities to `plf::colony` -- "An unordered data container providing fast iteration/insertion/erasure while maintaining pointer/iterator/reference validity to non-erased elements.". 

## Using the containers

Just add the include folder to your include path and include `ClusterVector.h` or `ClusterMap.h` in your files. All common defines are in `Common.h` and are almost entirely lifted from EASTL definitions (but are all renamed and namespaced to avoid collision). Platform support has not been well tested, and container tests are currently minimal.

## Building the tests

Build or generate with CMake in the `test/` folder :)
