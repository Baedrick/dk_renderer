### 2025-05-22: Shipping Resource Binary (Draft)
From the hello triangle code I realized that I need something better than reading
each individual shader binary from file then compiling it at runtime. A lot of
things can fail, like the file cant be found, reading fails, etc. I need the
build process and launching the appliction to be easy as possible for my university
appointed grader to be able to build and just run. I considered embedding the
shaders directly into the source code of the project but the issue with that is
it is a lot more complex than expected, because there is no standard way to do
so. There is #embed in C23 but that bumps build the requirements up and I'm not
ready for that. It makes things harder to incorporate since this is a cpp code-
base and c++23 doesnt have #embed. A solution is to write a small program to
parse binary programs and generate header files to embed. The problem is that
since compiling shaders requires compilation to spirv via glslangvalidator, I
need to then find those binaries and write a cpp header file out. The embedding
approach would increase compile times in the future because of the amount of
assets that are needed for rendering. Like a few shader files etc, unlike simpler
programs which can have one uber shader this renderer would need a lot more than
that which will increase compile times by a lot. I decided on an approach which
is a middle ground between both approaches of reading multiple files from disk
and embedding. The approach would be to pack all assets to ship with the renderer
into one .dkpak file. Then at startup I just read the file into memory and
initialize from there. If the file doesnt exist I can straight away shutdown
rather than halfway through initialization scream that something doesnt exist.

To start work on that I have to introduce a file directory iterator to be able
to iterate the file directories so I can grab the assets I need to pack into
the .dkpak file for export.

### 2026-05-22: ImGui, ImGui Backend, and Opensource Contributing (Draft)
RGFW doesnt have matured imgui backend implementation.
It didnt support the rgfw version that has been out for a while.
I took some time to write an updated backend and fix it to support unity
builds correctly. Then I opened a PR to merge my changed to the repository.

I worked on bringing imgui into the main application. Because rgfw imgui backend
is not very mature, a lot of the functionality was still lacking, requiring me
to dig through the source code or in the main function directly call stuff that
should've just worked, like using glfw you dont need to think about a lot of the
imgui updating. my main goal was just to bring imgui into the project first, ill
figure out how to improve the rgfw backend as i continue this project.

another problem with the backend that i might need to solve is that it included
the chrono header which brings a lot of other stl headers in. this slows down
compile times by a lot.

i managed to get imgui to show the demo window to make sure that i did set up
imgui correctly.

### 2026-05-21: Logging Datastructure & Semaphores (Draft)
I want logs to print to the renderer instead of needing to launch the renderer
from the console to logs. This requires changes to the way logs are stored in
memory because for a console window, basic user experience is being able to filter
logs by levels. The previous iteration of the logging concatenates the logs and
automatically indents them based on scope, but thats not useful when you need to
parse the logs and filter and colorize the text. This is where being able to
control the allocation strategy and not reaching for a generic data structure
shines because the cost of link lists (cache trashing) is negliable because the
nodes are allocated beside each other in memory. I wrote a hybrid data structure
called a multi-intrusive list which allows me to chain the same node in different
lists. So I created a list of pushes in order of log emission. I created homogenous
lists of log of the same level only. Because in 90% of cases, you want to see
logs of a specific level, or you want to see all logs. If you really needed to
filter by more than 2 types of logs (the 5% of cases), then we can traverse the
list in-order list and skip logs that are not desired.

I also implemented semaphores in the platform layer. The reason why I need it is
for inter-process communication. Last time I talked about not needing it because
I can use mutexes and conditional variables, but I need semaphores now because
those primitives are user level not system level, so sharing them across processes
is hard.

### 2026-05-20: Hello Triangle (Draft)
I reached a milestone today by successfully rendering a triangle in the codebase.
It took much longer than I expect since starting the project late february, but
Im happy that I have something to show for now with the work I put into the
project. Getting the triangle up did expose a lot of inefficiencies with the
codebase, like there are some functions that were too granular so doing a simple
action with them, like reading a file from disk, took a considerable amount of
code to make happen. So I know I need higher-level functions that I can just
say read from this file and give me the bytes back. Another thing is that it
showed me I need to consider a workflow for shipping shaders with the renderer
so that it will be simple and faster to initialize the renderer.

### 2026-05-20: Command Line Parsing
I implemented a simple command-line parser inspired by raddebugger's parser. It
handles flags, options with explicit value signifiers, and positional passthrough
strings. To keep the parsing logic predictable and straightforward, option values
must follow an explicit assignment syntax; standalone arguments that lack a
leading dash are categorized as passthrough inputs. To support fast lookups, I
implemented my first hash map from scratch. It uses a straightforward chaining
mechanism to resolve slot collisions, providing an excellent opportunity to
validate classroom datastructure theory in a practical context. Choosing to write
a specialized implementation rather than a generic container, allows for
composition of different datastructures. I can easily adapt this into a linked
hash map in the future if preserving the original argument parsing order becomes
necessary.

### 2026-05-19: RHI Backend Choosing and OpenGL Bootstrap (cont.)
I introduced a set of macros to automatically select the appropriate rendering
backend based on the target operating system when one is not explicitly specified.
I am considering a transition to Direct3D 11 and deprecating OpenGL in the long
term. I am currently unfamiliar with the API and prefer to prioritize exploring
rendering algorithms over learning a new interface. Mastering the core algorithms
provides greater long-term value than simply learning the function calls of an
arbitrary API.

I also resolved the bug for my OpenGL boostrapping process. I discovered that
every created window must have its pixel format explicitly configured via
`SetPixelFormat` to properly prepare the window's device context (HDC) for OpenGL
rendering. Afterwards, I can now share the OpenGL context  (HLGRC) created for
the hidden bootstrap window across other application windows. This enables
support for multi-window rendering and brings the interface of my render hardware
interface closer to the interface of modern graphics APIs.

### 2026-05-18: Rename Build Targets
I decided to rename my build targets to better reflect the long-term intent of
the project. The original "viewer" name felt unfitting for a project intended to
evolve into a full rendering engine; my vision aligns closer to Embark Studio's
Kajiya engine. Similarly, naming the project after OpenGL felt short-sighted
given my plans to eventually migrate to a modern API. For now, I have settled on
`dk_renderer` as the project name, with `dkrend` serving as the executable name
for the rendering application.

### 2026-05-17: OpenGL Bootstrap
I spent time boostrapping the OpenGL function loading mechanism on the Win32
platform. OpenGL differs significantly from modern graphics APIs; it requires
an active, modern rendering context simply to query the driver for modern function
pointers. Consequently, a temporary window with an active context must exist
before initialization can complete. This stands in contrast to Vulkan's hybrid
pointer querying model, Direct3D's dynamic linking, or WebGPU's static linking
approaches.

My strategy involves creating a hidden boostrap window during early initialization
to load these function pointers and capture a shared rendering context (HLGRC).
This context cna then be made current on any subsequent application windows.
While leaving the bootstrap window open introduces a minor resource leak, it
circumvents the complexities of multi-context resource sharing and synchronization.
However, I am currently encountering an issue where newly created windows fail
to render dispite being assigned the bootstrap context. This behavior is under-
documented; because the Win32 device context (HDC) and rendering context must
share a matching pixel format, and my configuration is uniform, the failure is
unexpected. Modern driver behavior typically tolerates this in practice, suggesting
I might have missed a platform layer step that I need to figure out.

### 2026-05-17: Process Launching
I implemeted the process launching logic within the Win32 platform layer. This
required a fair amount of experimentation due to the inherent quirks of the
Windows API; `CreateProcessW` offers multiple convoluted ways to specify execution
parameters, such as splitting or combing the binary path and command-line arguments
across different parameters. The official documentation is ambiguous, requiring
trial and error to establish a predictable path. I initially considered parsing
and modifying environment blocks to allow the parent process to append custom
variables to the child; however, I opted to scrap this feature to avoid unnecessary
architectural complexity. Allow the child process to inherit the parent's
environment by default is sufficient for my current needs, though I did preserve
the ability to specify an explicit working directory to simplify asset path
resolution.

During testing, I inadvertently triggered a fork bomb by forgetting that the
first argument in a command line string represents the name of the binary itself,
causing the application to continuously spawn recursive instances of itself. The
erroneous code block demonstrates the oversight:
```cpp
// pseudocode
for (String8Node *n = cmd_line->first; n; n = n->next) {
    if (!str8_equals(n->string, "--child")) {
        plt_process_launch("dkrend --child");
        break;
    }
}
```
Resolving this bug was a tedious process; the recursive loop exhausted system
resources and forced several system restarts before I found the bug. With that,
process launching is implemented and I am one step closer to coordinating the
renderer and an asset cooker.

### 2026-05-15: Documentation and Project Planning
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

### 2026-05-03: Opening and Closing a Window
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

### 2026-04-20: File and Window Dialogs
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
