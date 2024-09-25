# hgl
A collection of small easy-to-use header only libraries written in C.

| **library**           | **Category**                  | **LoC** | **Description**                                                                                         |
|-----------------------|-------------------------------|---------|---------------------------------------------------------------------------------------------------------|
| hgl\_arena\_alloc.h   | Allocator                     |    72   | Arena allocator.                                                                                        |
| hgl\_stack\_alloc.h   | Allocator                     |   142   | Stack allocator (requires hgl\_arena\_alloc.h).                                                         |
| hgl\_pool\_alloc.h    | Allocator                     |    93   | Pool allocator.                                                                                         |
| hgl\_fs\_alloc.h      | Allocator                     |   176   | A "free stack" allocator. Similar to other free list allocators.                                        |
| hgl\_memdbg.h         | Allocator/Utility             |   159   | Quick and easy leak checking replacement for malloc, realloc, and free.                                 |
| hgl\_vector.h         | Data Structure                |   166   | Typed\* dynamic (resizable) array.                                                                      |
| hgl\_da.h             | Data Structure                |   111   | Macro-only generic dynamic array implementation.                                                        |
| hgl\_q.h              | Data Structure                |    48   | Macro-only generic queue (circular buffer) implementation.                                              |
| hgl\_hash\_table.h    | Data Structure                |   341   | Typed\* Robin Hood style hash table/map.                                                                |
| hgl\_htable.h         | Data Structure                |   198   | Basically hgl\_hash\_table.h, but without the type stuff.                                               |
| hgl\_hset.h           | Data Structure                |   195   | Basically hgl\_htable.h, but without the value.                                                         |
| hgl\_ring\_buffer.h   | Data Structure                |    83   | Typed\* circular buffer.                                                                                |
| hgl\_rbtree.h         | Data Structure                |   411   | Your CS professor's ye olde red-black tree.                                                             |
| hgl\_chan.h           | Data Structure/Thread Utility |   108   | Typed\* synchronized message passing utility. Similar to Go's channels.                                 |
| hgl\_buffered\_chan.h | Data Structure/Thread Utility |   127   | Typed\* and buffered synchronized message passing utility. Similar to Go's buffered channels.           |
| hgl\_string.h         | Data Strucutre/Utility        |   603   | Dynamic string builder, sized strings, and easy-to-use string operations.                               |
| hgl\_hotload.h        | Utility                       |   210   | Quick and easy hotloading.                                                                              |
| hgl\_profile.h        | Utility                       |   234   | Quick and dirty profiling.                                                                              |
| hgl\_process.h        | Utility                       |   281   | Utility for spawning, running, and chaining together subprocesses. Basically execvp minus the headache. |
| hgl\_serialize.h      | Utility                       |   264   | Powerful binary parsing and serializing/unserializing with a simple regex-like language.                |
| hgl\_cmd.h            | Utility                       |   396   | Simple command prompt user interface with tab completion.                                               |
| hgl\_flags.h          | Utility                       |   354   | Simple utility for parsing command-line arguments.                                                      |
| hgl\_ini.h            | Utility                       |   359   | Simple utility for parsing (and generating) \*.ini files                                                |
| hgl\_io.h             | Utility                       |   327   | Simple file I/O.                                                                                        |
| hgl\_fft.h            | Algorithm                     |   130   | An implementation of the Fast Fourier Transform (FFT) using SIMD.                                       |
| hgl\_hamming.h        | Algorithm                     |   172   | Hamming(16,11) encoding and decoding system.                                                            |
| hglm.h                | Math                          |   944   | Vector math library with some SIMD support.                                                             |
| hglm\_aliases.h       | Math                          |   104   | Aliases for hglm.h without `hglm_` and `Hglm` prefixes, to make things less wordy.                      |
| hgl.h                 | Utility/Misc.                 |    98   | Misc. typedefs and macros that I use from time to time.                                                 |

\* In this context "typed" means that the type of data that is held by the data
   structure can be set at compile time by defining one or two macros before including.

Total LoC: 7082 \* Last updated 2024-09-25
