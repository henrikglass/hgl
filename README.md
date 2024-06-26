# hgl
A collection of small easy-to-use header only libraries written in C.

| **library**           | **Category**                  | **Description**                                                                                         |
|-----------------------|-------------------------------|---------------------------------------------------------------------------------------------------------|
| hgl\_arena\_alloc.h   | Allocator                     | Arena allocator.                                                                                        |
| hgl\_stack\_alloc.h   | Allocator                     | Stack allocator (requires hgl\_arena\_alloc.h).                                                         |
| hgl\_pool\_alloc.h    | Allocator                     | Pool allocator.                                                                                         |
| hgl\_vector.h         | Data Structure                | Typed\* dynamic (resizable) array.                                                                      |
| hgl\_da.h             | Data Structure                | Macro-only generic dynamic array implementation.                                                        |
| hgl\_hash\_table.h    | Data Structure                | Typed\* Robin Hood style hash table/map.                                                                |
| hgl\_ring\_buffer.h   | Data Structure                | Typed\* circular buffer.                                                                                |
| hgl\_chan.h           | Data Structure/Thread Utility | Typed\* synchronized message passing utility. Similar to Go's channels.                                 |
| hgl\_buffered\_chan.h | Data Structure/Thread Utility | Typed\* and buffered synchronized message passing utility. Similar to Go's buffered channels.           |
| hgl\_string.h         | Data Strucutre/Utility        | Dynamic string builder, sized strings, and easy-to-use string operations.                               |
| hgl\_memdbg.h         | Utility                       | Quick and easy leak checking replacement for malloc, realloc, and free.                                 |
| hgl\_hotload.h        | Utility                       | Quick and easy hotloading.                                                                              |
| hgl\_profile.h        | Utility                       | Quick and dirty profiling.                                                                              |
| hgl\_process.h        | Utility                       | Utility for spawning, running, and chaining together subprocesses. Basically execvp minus the headache. |
| hgl\_binpack.h        | Utility                       | Powerful binary parsing and packing/unpacking with a simple regex-like language.                        |
| hgl\_cmd.h            | Utility                       | Simple command prompt user interface with tab completion.                                               |
| hgl\_flags.h          | Utility                       | Simple utility for parsing command-line arguments.                                                      |
| hgl\_io.h             | Utility                       | Simple file I/O.                                                                                        |
| hgl\_fft.h            | Algorithm                     | An implementation of the Fast Fourier Transform (FFT) using SIMD.                                       |
| hgl\_hamming.h        | Algorithm                     | Hamming(16,11) encoding and decoding system.                                                            |
| hglm.h                | Math                          | Vector math library with some SIMD support.                                                             |
| hglm\_aliases.h       | Math                          | Aliases for hglm.h without `hglm_` and `Hglm` prefixes, to make things less wordy.                      |
| hgl.h                 | Utility/Misc.                 | Misc. typedefs and macros that I use from time to time.                                                 |

\* In this context "typed" means that the type of data that is held by the data
   structure can be set at compile time by defining one or two macros before including.
