### 2026-03-18: Logging


### 2026-03-16: Platform time querying


### 2026-03-14: Platform file reading


### 2026-03-11: UTF8 and UTF16 conversions


### 2026-03-03: Some string functionality


### 2026-03-02: Scratch arenas
I implemented a scratch memory mechanism to handle temporary allocations for
computation without the overhead of the heap or constraints of the stack. By
providing each thread with a `ThreadContext` containing its own scratch arenas,
functions can borrow memory for local work and return it on exiting the scope.
This pattern effectively replaces the stack for large or dynamic obejcts; it is
significantly faster than traditional heap allocation and allows for more
flexibility. Its another step towards zero allocation architecture where I can
focus on logic rahter than pointer bookkeeping.

### 2026-03-01: Arena allocator
I am moving away from the standard malloc/free paradigm in favor of a custom
arena allocator as the viewer's primary memory strategy. Following the
philosophy popularized by Ryan Fleury, the allocator utilizes virtual memory to
reserve a large address space upfront, which a chaining mechanism to commit new
blocks if the initial reservation is exceeded. Its liberating to be able to
group objects by lifetime; instead of chasing individual pointers and complex
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
