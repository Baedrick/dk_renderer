### 2026-05-15: Documentation & Project Planning
The academic semester has begun and this engine is now officially my capstone
project. Development will continue alongside my internship as required by my
university curriculum. Part of the project criteria involves demonstrating
formal planning through roadmapping and rigorous time allocation. While the
final presentation is several months away, I am choosing to push ahead of the
schedule to ensure the project is in a showcase-ready state for the university
career fair. I spent this session updating the project documentation and
defining specific development milestone goals for grading and visibility for
potential employers. Work done will be more sporadic moving forward due to the
concurrent internship, but maintaining a clear roadmap ensures progress remains
focused during available windows.

### 2026-05-10: Hashing
I implemented hashing utilities to support fast, non-cryptographic hashing of
arbitrary data blocks and strings. This is a prerequisite for building custom
hash tables later in the engine's development. Rather than writing an algorithm
from scratch or relying on the STL, I chose to integrate the xxHash library. Its
reputation for extreme speed and low collision rates aligns with my performance
goals. Furthermore, its single-header design made the integration process
trivial; it drops cleanly into the codebase without introducing complex build
dependencies, or unnecessary abstraction layers. This ensures my codebase and
build script remains easy to reason about, given that its just a unity build.

### 2026-05-10: OpenGL and Render Hardware Interface (RHI)
I am currently holding off on implementing a full Render Hardware Interface (RHI)
because I do not yet know the exact requirements my GPU-driven rendering pipeline
will demand. Building abstractions too early often leads to unnecessary complexity
and "what-if" code. However, when the time comes, I plan to model the interface
heavily after WebGPU. Its design is sane, and maps well to modern graphics APIs.
Furthermore, Sebastien Aaltonen is also designing his RHI around WebGPU concepts.
Following his approach provides me with an implicit mentor, which is invaluable
since I don't have a senior graphics programmer in my immediate circle to learn
from.

One notable deviation from WebGPU will be the need for persistent mapping. I know
that my engine will require persistent memory mapping for high-performance data
transfers to the GPU, a feature WebGPU currently lacks. I will have to design a
custom solution for this within the RHI when I eventually tackle it. For now,
the viewer layer will interact with OpenGL directly to keep things simple while
I prototype.

### 2026-05-03: Opening & Closing a Window
I implemented the ability to open and close windows in my rendering engine.
Rather than relying on a heavy framework like GLFW, I integrated RGFW. Its
lightweight, single-header nature makes it trivial to drop into the project.
Crucially, it allows me to easily modify the source code to support my custom
arena memory allocation strategy . Furthermore, unlike GLFW, which forces a
callback-driven architecture that complicates state management, RGFW exposes a
simple even queue. This allows me to poll for events within the main loop,
keeping the control flow simple and idiomatic to the rest of the codebase.

### 2026-05-01: Platform Core & Graphics Separation
I separated the platform layer into core systems and graphical interfaces. This
distinction allows the engine to support both graphical applications, like the
viewer, and console applications, like the asset compiler, without running
unnecessary code like COM initialization for console applications. I also
refactored the platform entry point to use a shared initialization path,
ensuring common codebase setup is executed before branching into specific
application logic.

### 2026-04-20: File & Window Dialogs
I ported Win32 file dialog functions to integrate with the engine's memory
model and string structures from my other project. The translation process
highlighted the need for further path manipulation utilities, such as
normalization, which I am leaving for a future update. I also implemented a
native graphical dialog box for error reporting to the user, although I may just
use IMGUI for all of the viewer's user interface.

### 2026-04-08: Atomic Operations
I spent some time familiarizing myself with atomic operations to implement them
as the foundation for lock-free algorithms within the engine. My immediate use
case is high-throughput inter-thread communication, such as a lock-free ring
buffer. Circumventing OS-level mutexes in hot paths eliminates the overhead
of context switches. This ensures the CPU spends its time executing useful
work rather than stalling on locks.

### 2026-04-03: Thread Primitives
I implemented the foundational threading and synchronization primitives in the
platform layer, including thread creation, mutexes, reader-writer locks, and
condition variables. I’ve intentionally omitted semaphores; mutexes and condition
variables provide sufficient control for producer-consumer patterns, and I can
move toward lock-free algorithms if I eventually need more granular performance.

For managing primitive allocations, I’m using a "fat struct" approach where all
synchronization objects are represented by a unified entity stored in an
arena-backed free list. This simplifies the handle system and provides a
consistent backing store. While I could eventually split these into specialized
arrays or use a reader-writer lock for the allocator to improve performance, the
current design prioritizes architectural clarity and ease of debugging for the
engine's scale.

### 2026-03-28: Profiler integration
I integrated Tracy profiler to provide high-fidelity instrumentation and
visualization for identifying performance bottlenecks. To maintain consistency
with my unity build setup, I have included the necessary Tracy source code
directly into the project and base layer. The macro DK_PROFILE_ENABLE keeps
profiling gated to keep the overhead out of shipping builds.

I've intentionally avoided building an abstraction layer or generic profiling
wrapper at this stage. Since I have no immediate plan to swap Tracy for another
tool, a wrapper would only introduce unnecessary boilerplate and "what-if"
complexity. By using Tracy's macros directly, I maintain a clear, one-to-one
mapping between the instrumentation in the source and results in the profiler.

### 2026-03-18: Logging
I implemented a deferred logging system that prioritizes performance and clarity
by avoiding the overhead of immediate I/O. Unlike traditional loggers that flush
to the console or disk as events occur, frequently triggering expensive context
switches to kernel mode, I’ve adopted a design heavily inspired by the RAD
Debugger (raddbugger). Log messages are accumulated into per-thread arena
buffers throughout the frame and are only consolidated into a single string at
the frame's end. This allows for a single, efficient flush to the operating
system or a UI overlay. I also implemented hierarchical log scopes using RAII;
this allows the system to automatically handle indentation and visual depth,
making it much easier to trace the execution flow and pinpoint exactly where a
log was emitted within a complex call stack.

### 2026-03-16: Platform time querying
I implemented high-resolution time querying in the platform layer to provide the
precision needed for calculating delta time and maintaining a consistent
simulation step. On Win32, I’m utilizing `QueryPerformanceCounter` and
`QueryPerformanceFrequency` to get microsecond-accurate timestamps. While these
primitives could form the basis of a custom profiling system, I’ve decided to
integrate Tracy when I tackle a profiling system. Having used Tracy in previous
projects, I’ve found its instrumentation and visualization to be excellent; it’s
a mature tool that will save me from reinventing the wheel when I need to find
bottlenecks later. I’ve also intentionally avoided implementing datetime or
calendar utilities. For a graphics engine, knowing how long a frame took is the
primary concern; I’d rather skip the intricacies of timezones and formatting
until there’s a genuine reason to tackle them.

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
I am starting a toy viewer as my Capstone Project . It is a personal graphics
engine and sandbox for getting proficiency in GPU-driven renering, that is,
offloading culling and draw call generation to the GPU. My goal is to move
beyond academic theory and appraoch this like a professional: implement,
profile, optimize.

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
Surfels or Dynamic Diffuse GI (DDGI). These plans are flexible; I want the
freedom to follow wherever technical curiosity leads.

### yyyy-mm-dd: Summary
- Goal
- Progress & plans
- Struggles faced (optional)
- Decisions made (optional)
- Reflection
