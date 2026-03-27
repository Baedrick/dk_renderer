### 2026-03-27: 

### 2026-03-18: Logging
I worked on a deferred logging system that works with the existing memory model.
The goal is to create a thread-safe hierarchial logging system, 

### 2026-03-16: Platform time querying
I added simple high-resolution time querying into the engine. This will be used
for computing delta time (time it took to simulate one frame). I think I want to
use an external library for profiling like Tracy. I do not think that I'll need
local time conversion utilities for the time being.

### 2026-03-14: Platform file reading & writing
I wrote file reading and writing functionalities for Win32. The design I had in
mind was to support reading and writing files in ranges/chunks, since I want to
make use of multithreading in the future. I opted for OVERLAPPED I/O to avoid
the stateful file pointer of the operating system and makes the read calls
naturally thread-safe without needing mutexes around the file handle. Writing
files are done in chunks of 1MB because of how the Windows operating system
writes data to disk. It needs to allocate an equivalent block of non-paged
memory or lock user pages into physical ram for the underlying I/O request.
Limiting writes to 1MB ensures that we don't exhaust these kernel resources or
fail on massive buffers. Furthermore, chunking allows for disk queue fairness;
rather than a single massive write hogging the disk, smaller requests allow the
scheduler to interleave operations, keeping the system responsive while multiple
threads are executing I/O simultaneously.

### 2026-03-11: UTF8 and UTF16 conversions
I spend the last two days learning the nuances of Unicode to implement my own
UTF-8 and UTF-16 conversion utilities. Since Windows natively uses UTF-16, the
engine needs a way to communicate with the operating system. I am strictly using
the "W" versions of the Win32 API to avoid problems like the Worsefit security
exploit and slow performance of the "A" functions which performs a conversion
from UTF-8 to UTF-16 anyway. The plan forward to to keep the entire engine in
UTF-8 and only convert when the platform expects it in a different encoding.

### 2026-03-03: Some string functionality
My goal was to write string functionality that integrates well with arena
allocation and have C-API compatibility. Typically, strings are built once and
then treated as immutable. I designed my string structures to be slices, then
have functions to build the string using scratch arenas and finalize a built
string with null-termination. I think I achieved my goal because it is much
nicer to use compared to `std::string` and `std::string_view`.

### 2026-03-02: Scratch arenas
I implemented a scratch memory mechanism to handle temporary allocations for
computation without the overhead of the heap or constraints of the stack. By
providing each thread with a `ThreadContext` containing its own scratch arenas,
functions can borrow memory for local work and return it on exiting the scope.
This pattern effectively replaces the stack for large or dynamic obejcts; it is
significantly faster than traditional heap allocation and allows for more
flexibility. Its another step towards the simple architecture where I can focus
on logic rahter than pointer bookkeeping.

### 2026-03-01: Arena allocator
I am moving away from the standard malloc/free paradigm in favor of a custom
arena allocator as the viewer's primary memory strategy. Following the
philosophy popularized by Ryan Fleury, the allocator utilizes virtual memory to
reserve a large address space upfront, which a chaining mechanism to commit new
blocks if the initial reservation is exceeded. Its great to be able to group
objects by lifetime; instead of chasing individual pointers and complex
releasing logic. I can reclaim entire blocks of memory instantly by resetting an
integer. It simplifies the architecture and ensures that memory management never
gets in the way of the renderer's performance.

### 2026-02-27: Unity build script
I am moving to a unity build to eliminate the friction of modern toolchains. I
found that incremental linking often takes longer than simply compiling from
scratch. Furthermore, switching between git branches often leaves build
artefacts in a bad state, often requiring full recompiles. I get near instant
compile times by using a simple batch file that treats the project as a single
translation unit, avoiding the complexity of cmake. This speed is great for
graphics programming; it allows for rapid iteration without losing my train of
thought. The script is extensible enough to eventually handle multiple build
targets like compiling shaders or compiling supporting programs like a dedicated
asset compiler. Keeping it simple ensures I can focus on code instead of
fighting the toolchain.

### 2026-02-24: Repository created
I am starting a toy viewer, a personal graphics engine and sandbox for getting
proficiency in GPU-driven renering, that is, offloading culling and draw call
generation to the GPU. My goal is to move beyond academic theory and appraoch
this like a professional: implement, profile, optimize.

In school, I was taught to use modern C++ and heavy rely on Object Oriented
Programming (OOP), template metaprogramming, and the STL. During my team-based
projects, these modern C++ practices are often mandatory because they are the
baseline of our curriculum. However, I found that these deep abstraction layers
and the often cryptic compiler errors make it increasingly difficult to
introduce new features or debug existing ones.

I'm going to move towards a Handmade philosophy to programming, popularized by
high profile people like Casey Muratori and Ryan Fleury. I'm going to stick to
a procedural approach, using structs and global functions that act on those
structs, and calling OpenGL directly rather than hiding it behind leaky wrappers.

Once the renderer is stable, I plan to explore higher-level systems like a render
graph for automatic pass ordering, and real-time global illumination like
Surfels or Dynamic Diffuse GI (DDGI). These plans are flexible; as a technical
artist, I want the freedom to follow wherever technical curiosity leads.

### yyyy-mm-dd: Summary
- Goal
- Progress & plans
- Struggles faced (optional)
- Decisions made (optional)
- Reflection
