### 2026-06-13: Milestone 1 completed

### 2026-05-29: Worries About The Future Of This Project
I just learned that the capstone project requires my work to directly benefit 
my internship company. The university assumes we use company time to work on 
the capstone. I think that rule is ridiculous. The common case for company-led 
capstones is that students receive busy work that never sees production. Even 
when interns join teams working on real projects, their contributions are often 
scattered. They rarely get to own a specific feature. To make matters worse, 
the university expects students to present only their isolated contributions 
without discussing the bigger picture of the project. This makes presenting a 
cohesive narrative incredibly difficult. In my case, I am building this 
renderer entirely outside of my internship hours.

Another requirement states the technical area of the capstone must align with 
the company's industry. That is also incredibly frustrating. What happens when 
a student only finds an internship in an industry they do not want to work in? 
The university administration consistently shows they do not know how to give 
students space to learn and experiment. They make backward decisions that block 
us from building skills we actually want to use after graduation.

I worry the professors will force me to submit the day-to-day work I do at my 
internship instead of this capstone. At my company, I am a technical artist. I 
make pipeline tools, write shaders, and setup lighting for Unity. Writing a 
GPU-driven renderer from scratch is adjacent to that, but they might argue it 
does not align perfectly. If they force me to submit my company work, I will 
struggle to build a narrative of my personal growth. Technical art tasks are 
usually fragmented across random production needs, whereas building an engine 
from scratch tells a clear story from start to finish.

### 2026-05-28: File Directory Iteration & First-Pass PAK Implementation
I wrote the file directory iterator in the platform layer and finished the 
first-pass implementation of the PAK format. I took inspiration from the Rad 
Debug Information (RDI) format. I really like the idea of putting things into 
homogenous arrays so the engine can jump directly to specific offsets. This 
approach is much faster for the CPU than traversing variable-width heterogenous 
data structures.

The initial `pakgen` implementation works, but it needs improvement. I adapted 
some logic from raddebugger, but my requirements differ because I supply data to 
both the CPU and the GPU. My plan places all CPU metadata at the start of the 
file, followed by the GPU bulk data. This layout means the application only 
needs to parse the front of the file. We can then read the trailing data directly 
into GPU memory. Grouping the data this way minimizes CPU cache misses and
avoids loading heavy geometry into system RAM. I plan to investigate unbuffered 
I/O later to speed up the GPU uploads. Right now, I am starting the second pass 
of `pakgen` to refine the data structures.

### 2026-05-24: RGFW ImGui Backend
I wrote my own backend for RGFW ImGui to support the latest library version. 
The existing integration was out of date. I submitted a pull request to merge 
my new backend into the official GitHub repository, but that is up to the 
author to accept. I also modified the coordinate handling to ensure that HiDPI 
screens are supported properly. Writing the backend myself ensures the renderer 
scales correctly across monitors without waiting on a third party.

### 2026-05-22: Shipping Resource Binary
Working on the hello triangle milestone showed me that reading individual shader 
files from disk at runtime is too fragile. Missing files or read errors create 
unnecessary points of failure. I want the build and execution process to be 
seamless so the grading professor does not have to troubleshoot the executable.

I looked into embedding shaders directly into the C++ source code. C23 
introduces `#embed`, but my current compiler setup does not support it without 
adding build complexity I want to avoid. Writing a custom script to convert 
SPIR-V binaries into C-style header arrays is another option. Doing so would 
massively inflate compile times later. The compiler has to parse huge arrays of 
hexadecimal text, and this engine will eventually require many distinct shaders 
rather than a single monolithic one.

I decided to pack all runtime assets into a single `.pak` file to strike a 
balance between loose disk files and embedded source code. The engine will 
allocate a single memory block and read the entire archive at startup. If the 
file is missing, the application terminates immediately rather than crashing 
halfway through initialization. To build this archive, I am adding a platform 
directory iterator to gather assets during the packing phase.

### 2026-05-22: ImGui, ImGui Backend, and Opensource Contributing
The existing Dear ImGui backend for RGFW lacked support for the latest version. 
I spent time writing an updated implementation and fixing bugs to make it 
compile properly within my unity build setup. After testing the changes, I 
opened a pull request to contribute these fixes back to the upstream repository.

Integrating ImGui highlighted some limitations in the RGFW backend. Mature 
backends like GLFW handle context updates automatically, but RGFW requires 
explicit manual state management. My priority right now is just establishing a 
working UI baseline for debugging. I will clean up the backend code later.

The current backend also depends on the `<chrono>` header. This pulls in a 
massive amount of STL boilerplate and noticeably slows down compilation speeds. 
Fast iteration is the whole point of my unity build goals. For now, I rendered 
the ImGui demo window successfully. This confirms my pipeline bindings and 
render states are configured correctly.

### 2026-05-21: Logging Datastructure & Semaphores
I want to view log output directly in the renderer UI instead of forcing the 
application to launch from a system console. To make this usable, the UI needs 
to filter and color-code messages by severity. The old system just concatenated 
strings and applied visual indentation based on scope. Parsing and filtering 
that giant block of text at runtime wastes CPU cycles.

This problem highlights exactly why I prefer custom allocators over standard 
containers. Linked lists usually cause cache thrashing. By allocating nodes 
contiguously from my arenas, the CPU cache stays warm and the overhead becomes 
negligible. I designed a multi-intrusive list structure. Each log node links 
into a master chronological list and a secondary list based on its severity 
level. This layout gives the UI instant lookups when filtering. For complex 
filters, traversing the master list and skipping undesired elements remains fast.

I also added system-level semaphores to the platform layer for inter-process 
communication. Standard mutexes and condition variables work well for threads, 
but they do not bridge the process boundary easily. Semaphores will allow the 
renderer to coordinate safely with the external asset cooker.

### 2026-05-20: Hello Triangle
I rendered my first triangle today. Reaching this milestone took longer than 
expected since starting the project in February, but it validates the core 
architecture I started building back then.

Getting geometry on screen exposed some annoying inefficiencies in my code. The 
platform utilities are way too granular. Reading a file requires an excessive 
amount of boilerplate code. I need to write higher-level wrappers that load a 
file's raw bytes in a single function call. Reducing this friction keeps the 
application code readable. This process also confirmed I need a better asset 
packing workflow to speed up engine initialization.

### 2026-05-20: Command Line Parsing
I wrote a command-line parser inspired by raddebugger. It supports flags, 
key-value options, and positional strings. Option values require an explicit 
assignment syntax. Arguments without a leading dash act as passthrough inputs.

To support fast lookups, I wrote my first custom hash map from scratch using 
chaining for collision resolution. It felt great to validate classroom data 
structure theory in a practical context. Writing a specialized hash map instead 
of using the STL allows me to compose different data structures easily. I can 
adapt this into a linked hash map later if I need to preserve argument order. 

This is a comparison between the old and new way:
```cpp
// EXISTING
for (String8Node *n = cmd_line->first; n; n = n->next) {
  if (!str8_equals(n->string, "--child")) {
      plt_process_launch("dkrend --child");
      break;
  }
}

// NEW
if (!cmd_line_has_flag(cmd_line, "child")) {
  plt_process_launch("dkrend --child");
}
```

### 2026-05-19: RHI Backend Choosing and OpenGL Bootstrap (cont.)
I added macros to automatically select the target rendering API based on the 
operating system. I plan to transition to Direct3D 11 eventually, but for now 
I am sticking with OpenGL. My focus is on learning rendering algorithms rather 
than learning a new API surface. Good algorithmic knowledge translates well 
across all hardware. Mastering core concepts provides greater long-term value 
than simply memorizing the function calls of an arbitrary API.

I also finally fixed the bug in the OpenGL bootstrap code. Windows requires the 
pixel format of every new window to be set via `SetPixelFormat` before binding 
an OpenGL context. Now I can share the initial bootstrap context (HLGRC) across 
multiple application windows. This structure brings the engine closer to modern 
APIs and lets me build multi-window tools.

### 2026-05-18: Rename Build Targets
I renamed the build targets to match the project's long-term scope. Calling it 
a "viewer" felt limiting for a project intended to evolve into a full rendering 
engine. My vision aligns closely with Embark Studio's Kajiya engine. Naming it 
after OpenGL also made little sense since I will eventually migrate away from it. 
The project is now `dk_renderer`, and the main executable is `dkrend`.

### 2026-05-17: OpenGL Bootstrap
I spent time writing the OpenGL function loader for Win32. OpenGL requires an 
active context just to query the driver for function pointers. This differs 
completely from Vulkan's hybrid pointer querying or Direct3D's dynamic linking.

I create a hidden bootstrap window during startup, grab the modern function 
pointers, and capture a shared context. This context gets applied to all future 
windows. Keeping the hidden window open causes a minor resource leak, but it 
avoids the absolute headache of synchronizing multiple rendering contexts. This 
approach reduces driver overhead. I am currently stuck on a bug where new windows 
refuse to render. Win32 requires the window Device Context and the GL context to 
share the same pixel format. My configuration matches, so I need to find the 
missing step in the platform layer.

### 2026-05-17: Process Launching
I added process launching to the Win32 platform layer. `CreateProcessW` is 
horribly documented and offers confusing ways to pass command-line arguments. I 
spent a lot of time experimenting to find a reliable method. I originally planned 
to pass custom environment blocks to the child process, but I dropped the idea 
to keep the architecture simple. The child inheriting the parent's environment 
works fine for now. I did keep the ability to set the working directory to help 
resolve asset paths.

During testing, I accidentally created a fork bomb. I forgot that the first 
command-line argument is the binary name. The renderer launched endless copies 
of itself recursively. The erroneous code block demonstrates the oversight:
```cpp
// pseudocode
for (String8Node *n = cmd_line->first; n; n = n->next) {
    if (!str8_equals(n->string, "--child")) {
        plt_process_launch("dkrend --child");
        break;
    }
}
```
Resolving this bug was tedious. The recursive loop exhausted my system resources 
and forced several hard reboots before I found the bug. Process launching is now 
working, which gets me one step closer to coordinating the asset cooker.

### 2026-05-15: Documentation and Project Planning
The semester started, making this engine my official capstone project. I will 
develop it alongside my internship. The university requires formal roadmaps and 
time tracking for grading. I am choosing to work ahead of schedule so the engine 
is presentable for the upcoming career fair.

I updated the documentation and set specific milestones. My development time 
will be sporadic moving forward due to my job. Keeping a strict roadmap helps 
me pick up where I left off without losing focus.

### 2026-05-10: Hashing
I integrated the xxHash library for fast, non-cryptographic hashing. I need this 
to build custom hash tables later. I integrated it specifically because its 
single-header design drops cleanly into the codebase. I wanted to avoid the 
bloat of the STL and keep my unity build script simple. xxHash provides excellent 
speed and low collision rates, which means less CPU time spent on string lookups 
during asset loading.

### 2026-05-10: OpenGL and Render Hardware Interface (RHI)
I am delaying the design of a full Render Hardware Interface (RHI). Building 
abstractions before knowing the exact requirements of a GPU-driven pipeline 
leads to wasted effort and "what-if" code. When I build it, I will model it 
after WebGPU. Sebastien Aaltonen uses a similar approach, and studying his work 
helps immensely since I lack a senior graphics mentor in my immediate circle.

I will need to deviate from WebGPU to support persistent memory mapping. High-
performance data transfers require persistent mapped buffers so the CPU can 
write directly to GPU-visible memory without driver intervention. For now, the 
viewer will call OpenGL directly to keep things simple while I prototype.

### 2026-05-03: Opening and Closing a Window
I added window management using RGFW. Unlike GLFW, RGFW is a lightweight single 
header that I can easily modify to use my arena allocator. More importantly, 
GLFW forces a callback-driven architecture that complicates state management. 
RGFW exposes a simple event queue. Polling the queue keeps the control flow 
linear and predictable, which feels much more idiomatic to the rest of the 
codebase.

### 2026-05-01: Platform Core & Graphics Separation
I split the platform layer into core systems and graphical systems. This 
separation means command-line tools like the asset compiler do not run heavy 
code like COM initialization. Keeping the console applications lightweight 
improves their startup time. Both types of applications share the same base 
initialization path before branching into their specific logic.

### 2026-04-20: File and Window Dialogs
I rewrote the Win32 file dialog functions to use my arena memory model and 
string slices. I also added a graphical error dialog box. Using native dialogs 
avoids pulling in a heavy UI framework just for error reporting. Working on this 
highlighted the need for path normalization utilities, which I am leaving for a 
future update.

### 2026-04-08: Atomic Operations
I spent time learning atomic operations to build lock-free algorithms. I plan 
to use them for a lock-free ring buffer for inter-thread communication. Skipping 
OS-level mutexes in hot paths eliminates expensive kernel context switches. This 
keeps the CPU focused on executing useful rendering work instead of stalling on 
locks.

### 2026-04-03: Thread Primitives
I added thread creation, mutexes, reader-writer locks, and condition variables 
to the platform layer. I skipped semaphores initially because mutexes and 
condition variables handle most producer-consumer patterns well enough.

All synchronization objects are stored as a unified "fat struct" inside an 
arena-backed free list. This design prioritizes architectural clarity. Grouping 
these objects prevents memory fragmentation and provides a consistent backing 
store. I could split these into specialized arrays for performance later, but 
ease of debugging matters more right now.

### 2026-03-28: Profiler integration
I integrated the Tracy profiler directly into the codebase. I gated it behind 
a `DK_PROFILE_ENABLE` macro so it compiles out entirely in release builds.

I specifically avoided writing an abstraction layer or a generic profiling 
wrapper. I have no plans to swap Tracy for another tool, so a wrapper just 
adds unnecessary boilerplate and "what-if" complexity. Calling the macros 
directly ensures I have low-overhead instrumentation that maps perfectly from 
the source code to the profiler's UI.

### 2026-03-18: Logging
I built a deferred logging system heavily inspired by the RAD Debugger. 
Traditional loggers flush to the console immediately, triggering expensive 
context switches to kernel mode. My system accumulates logs into per-thread 
arena buffers and flushes them once at the end of the frame. Batching the I/O 
this way massively reduces CPU overhead. I also added hierarchical scopes using 
RAII to automatically handle indentation. This makes it much easier to trace the 
execution flow and see exactly where a log was emitted.

### 2026-03-16: Platform time querying
I added time querying using `QueryPerformanceCounter` and 
`QueryPerformanceFrequency` on Win32. These functions provide the microsecond 
precision needed for frame timing. I am intentionally ignoring calendar and 
timezone logic. A graphics engine only cares about delta time. Skipping date 
logic keeps the platform layer lean.

### 2026-03-14: Platform file reading & writing
I wrote Win32 file I/O functions using OVERLAPPED I/O. I wanted to avoid the 
operating system's stateful file pointer so reads are naturally thread-safe 
without needing mutexes around the file handle. I limited write chunks to 1MB. 
Windows locks user pages into physical RAM for I/O requests. Limiting the chunk 
size prevents exhausting kernel resources. More importantly, chunking allows for 
disk queue fairness. The OS scheduler can interleave smaller operations, keeping 
the system responsive while multiple threads run I/O.

### 2026-03-11: UTF8 and UTF16 conversions
I spent the last two days learning the nuances of Unicode to write custom UTF-8 
and UTF-16 converters. Windows natively uses UTF-16, so the engine needs to 
convert strings when calling the OS. I strictly use the "W" Win32 API functions 
to avoid the Worsefit security exploit and the performance cost of the OS doing 
the conversion internally anyway. The engine runs entirely on UTF-8 internally 
to save memory.

### 2026-03-03: Some string functionality
I built string structures that act as slices. I wanted string functionality that 
integrates perfectly with arena allocation and C-API compatibility. Standard 
library strings allocate on the heap, which fragments memory. My strings are 
built using scratch arenas and finalized with null-termination only when needed. 
It feels much nicer to use compared to `std::string` and `std::string_view`.

### 2026-03-02: Scratch arenas
I added scratch arenas to the `ThreadContext`. Functions can borrow memory for 
temporary work and release it upon exiting the scope. This acts like a dynamic 
stack for large objects. It avoids the overhead of heap allocation and requires 
zero pointer bookkeeping. Returning the memory is as simple as resetting an 
integer.

### 2026-03-01: Arena allocator
I replaced `malloc` and `free` with a custom arena allocator based on Ryan 
Fleury's philosophy. The allocator reserves a massive virtual memory address 
space and commits physical pages as needed. Grouping allocations by lifetime 
means I can free an entire subsystem instantly. This eliminates pointer chasing 
and ensures memory management never blocks the renderer's performance.

### 2026-02-27: Unity build script
I switched to a unity build to avoid the friction of modern toolchains. I found 
that incremental linking often left build artifacts in a bad state when 
switching git branches, requiring full recompiles anyway. Compiling the project 
as a single translation unit using a simple batch file is incredibly fast. That 
speed allows for rapid iteration without losing my train of thought. Keeping the 
build simple ensures I focus on code instead of fighting CMake.

### 2026-02-24: Repository created
I started this repository for my capstone project. My goal is to build a GPU-
driven renderer and learn how to offload draw call generation to the hardware. 
I want to approach this like a professional: implement, profile, and optimize.

University classes force the use of modern C++ with heavy Object-Oriented 
Programming and template metaprogramming. I found that deep class hierarchies 
and cryptic compiler errors make it incredibly difficult to introduce features 
or debug existing ones. 

I am adopting a Handmade philosophy inspired by Casey Muratori and Ryan Fleury. 
I am using a procedural approach in C++. Structs contain data, and global 
functions act on that data. I will call OpenGL directly instead of hiding it 
behind leaky wrappers. Once the renderer is stable, I plan to explore higher-
level systems like a render graph or Dynamic Diffuse GI.

### yyyy-mm-dd: Summary
- Goal
- Progress & plans
- Struggles faced (optional)
- Decisions made (optional)
- Reflection
