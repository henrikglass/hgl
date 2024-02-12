# hgl
A collection of small easy-to-use header only libraries written in C.

| **library**           | **LoC** | **Category**                  | **Description**                                                                               |
|-----------------------|---------|-------------------------------|-----------------------------------------------------------------------------------------------|
| hgl\_arena\_alloc.h   |   64    | Allocator                     | Arena allocator.                                                                              |
| hgl\_stack\_alloc.h   |   60    | Allocator                     | Stack allocator (requires hgl\_arena\_alloc.h).                                               |
| hgl\_pool\_alloc.h    |   77    | Allocator                     | Pool allocator.                                                                               |
| hgl\_vector.h         |  166    | Data Structure                | Typed\* dynamic (resizable) array.                                                            |
| hgl\_hash\_table.h    |  341    | Data Structure                | Typed\* Robin Hood style hash table/map.                                                      |
| hgl\_ring\_buffer.h   |   83    | Data Structure                | Typed\* circular buffer.                                                                      |
| hgl\_chan.h           |  108    | Data Structure/Thread Utility | Typed\* synchronized message passing utility. Similar to Go's channels.                       |
| hgl\_buffered\_chan.h |  127    | Data Structure/Thread Utility | Typed\* and buffered synchronized message passing utility. Similar to Go's buffered channels. |
| hgl\_string.h         |  422    | Data Strucutre/Utility        | Dynamic string builder, sized strings, and easy-to-use string operations.                     |
| hgl\_memdbg.h         |  123    | Utility                       | Quick and easy leak checking replacement for malloc, realloc, and free.                       |
| hgl\_hotload.h        |  210    | Utility                       | Quick and easy hotloading.                                                                    |
| hgl\_binpack.h        |  255    | Utility                       | Powerful binary parsing and packing/unpacking with a simple regex-like language.              |

Total lines of code: 2036.

\* In this context "typed" means that the type of data that is held by the data
   structure can be set at compile time by defining one or two macros before including.
