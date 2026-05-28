### Timeline
- 15 June 2026 - Milestone 1
    - Focus: Project scaffolding, Asset Pipeline, Basic Rendering
    - Project proposal submission
    - Build script for both viewer and cooker targets
    - Process launching, joining, and pipe reading to read the cooker's stdout to viewer's ui
    - Toy Cooker: Parse basic glTF files and serialize them into a custom binary format
    - Toy Viewer: Load custom binary format and display viewer
    - Hello Model via simple forward rendering pass to validate graphics code
      - Damaged Helmet as sample model to validate
    - View imported model resource in ui as a tree of resources (materials, textures, meshes)
    - Main light shadows
- 9 Aug 2026 - Milestone 2
    - Focus: GPU-Driven Core & Render Graph
    - General memory allocator for persistent GPU buffer mapping
    - GPU-driven drawing with frustum culling and indirect draw commands
    - Render Graph for automatic pass ordering and synchronization barriers
    - Skybox/HDRI for the background and ambient light
    - Deferred shading of opaque objects
    - Forward shading of transparent objects
- 6 December 2026 - Milestone 3
    - Focus: Advanced Lighting & Debug Rendering
    - Implement real-time global illumination (VXGI, DDGI, or Surfels)
    - Light clustering for efficient shading of multiple light sources
    - Debug rendering of passes
- 19 Feb 2027 - Career Fair
    - Focus: Minimum viable project (MVP) to show potential employers
    - Must have everything planned implemented but doesn't need to be polished.
    - Command palette for quick access to rendering configurations and settings.
- 22 Mar 2027 - Milestone 4
    - Focus: Academic deliverables
    - Polish from feedback during career fair.
    - Draft final thesis, highlighting how classroom knowledge was applied and expanded on.
    - Prepare presentation slides and rehearse oral demonstration.

### Capstone Assessment Domains
- Quantity and Quality of Technical Work
  - Candidates are expected to complete a capstone project that is of reasonable
    complexity, industry relevance, and that allows scope for the candidate to
    demonstrate the various aspects of software engineering and information security.
- Application of Knowledge
  - Candidates are expected to utilize knowledge gained in the classroom, both
    across a spectrum of modules, as well as in depth within modules to complete
    their project, in addition to utilizing knowledge beyond the classroom to do so.
- Analysis and Solution Formulation
  - Candidates are expected to analyse existing works, and, combined with
    analysis of the project problem, propose appropriate, detailed approaches
    and solutions to the problem.
- Project Management and Individual Initiative
  - Candidates are expected to practise appropriate time and resource
    management, and to take initiative and ownership of their project
- Professional and Interpersonal Conduct
  - Candidates are expected to keep in contact with their assigned Academic
    Supervisor and demonstrate strong professionalism in completing their
    project.
- Written Communication and Reports
  - Candidates are expected to demonstrate written communication skills via progress
    reports at the end of every intermediate trimester, building up to a final
    project report at the end of the project
- Oral Presentation and Project Output Showcase
  - Candidates are expected to demonstrate effective technical presentation
    skills through the preparation and delivery of a project presentation, and
    optionally demonstration of the completed works

### dkrend
- [ ][Must Have] Reading of cooker logs to display
- [ ][Must Have] Console in UI to show logs
- [ ][Must Have] Hello Model to validate graphics code
- [ ][Must Have] Application command/event list
- [ ][Must Have] Inter-Process Communication with Toy Cooker
- [ ][Must Have] View imported model resources in the UI as a tree of resources
- [ ][Nice To Have] Hot-reloading of cooked model binaries
- [x][Must Have] ImGui show demo window
- [x][Must Have] Hello Triangle to validate graphics code
- [x][Must Have] Hello Triangle shader code
- [x][Must Have] Top level functions for init, update, and shutdown
- [x][Must Have] Update build script to have viewer as a build target

### dkcook
- [ ][Must Have] Inter-Process Communication with Toy Viewer
- [x][Must Have] Top level functions for entry to cooking
- [x][Must Have] Update build script to have cooker as a build target

### Codebase
- [ ][Must Have] Remove Array<u8> from file reading and use buffer8 instead
- [ ][Must Have] Resource packing tool to export spirv binaries as one blob
- [ ][Must Have] Packed resource format, only needs to support spriv for now
- [ ][Must Have] Update readme with project description and goals
- [ ][Must Have] General purpose allocator for persistent gpu buffer
- [ ][Must Have] Set up UI layer
- [ ][Must Have] Ring buffer for Inter-Process Communication
- [ ][Must Have] Path helpers and normalization
- [ ][Must Have] Render Graph for automatic ordering of passes
- [ ][Must Have] Real-time global illumination (VXGI, DDGI, Surfels)
- [ ][Should Have] Light clustering for shading
- [ ][Should Have] Render Graph resource aliasing and allocation
- [ ][Nice To Have] Command Palette for quick access to configurations and settings
- [ ][Nice To Have] Broadly, WebGPU interface as graphics api abstraction for RHI layer
- [ ][Nice To Have] Remove RGFW windowing and replace with codebase windowing
- [ ][Nice To Have] Support large page allocations for arena performance
- [x][Must Have] Write buffer8 implementation for storing bytes
- [x][Must Have] Clean up string implementation to have consistent calling convention
- [x][Must Have] Write RGFW v2.0.0 imgui backend
- [x][Must Have] Remove chrono from RGFW imgui backend because of compile times
- [x][Must Have] File directory iteration
- [x][Must Have] String16 helpers and user literal for win32 code
- [x][Must Have] Import Dear ImGui
- [x][Must Have] Update build script to build shaders to shader folder in bin
- [x][Must Have] Frame arena and measuring frame time
- [x][Must Have] Obtain user program data path from os for config storing and logs
- [x][Must Have] Platform high-level file reading/writing helpers
- [x][Must Have] Refactor logging to allow for separation of log level and in-order append
- [x][Must Have] Refactor list helpers to support named next and prev pointers
- [x][Must Have] Win32 semaphores for IPC synchronization
- [x][Must Have] Command line argument parsing for entry point
- [x][Must Have] Draft of readme document with how to build
- [x][Must Have] Fix OpenGL boostrapping for win32
- [x][Must Have] Add glsl shaders to Spir-V compilation as build target
- [x][Must Have] Dummy window for modern OpenGL context function loading before opening main window
- [x][Should Have] Mechanism for manually and automatically choosing rendering backend
- [x][Must Have] Process launching and joining
- [x][Must Have] Update readme document with project structure
- [x][Must Have] Move RHI OpenGL to its own file
- [x][Must Have] Change string8 literal constructor to a user defined literal
- [x][Must Have] String hashing helpers
- [x][Must Have] Add byte hashing functions
- [x][Must Have] Import xxHash v0.8.3
- [x][Must Have] Set up OpenGL function loading in RHI layer
- [x][Must Have] Render Hardware Interface layer stubs
- [x][Must Have] Import glad v2.0.8 for opengl function loading
- [x][Must Have] Fix tracy incorrectly allowing windows.h to define min and max
- [x][Must Have] Window allocation in platform graphical layer
- [x][Must Have] List queue helpers: push front, push back, insert, remove
- [x][Must Have] Forward list queue helpers
- [x][Must Have] Modify RGFW with window pointers for intrusive list
- [x][Must Have] Move common entry point to base layer
- [x][Must Have] Update build script for msvc to select entry for console/window builds
- [x][Must Have] Split win32 platform layer into core and graphical implementation
- [x][Must Have] Win32 platform layer message box for graphical messages
- [x][Must Have] Win32 platform layer file dialog open, open multiple, save, pick folder
- [x][Must Have] Move WinMain initialization to common win32 entry point for initialization
- [x][Must Have] Win32 platform layer show in file browser
- [x][Must Have] Import RGFW v2.0.0-dev for window abstraction library
- [x][Must Have] Win32 platform layer shared memory creation for Inter-Process Communication
- [x][Must Have] Win32 platform layer directory creation
- [x][Must Have] Atomic intrinsic functions for common types (u32, u64, pointers)
- [x][Must Have] Mutex creation, locking, and release
- [x][Must Have] Conditional variable creation, waiting, signalling, release
- [x][Must Have] Read write mutex creation, locking, and release
- [x][Must Have] Mutex creation, locking, and release
- [x][Must Have] Win32 platform layer thread entry point
- [x][Must Have] Thread creation, joining, detaching, and entry for Win32 platform layer
- [x][Must Have] Update readme with AI usage declaration
- [x][Must Have] Add profiling functions to base layer
- [x][Must Have] Import and set up tracy profiler v13.1
- [x][Must Have] Clean up code variable naming consistency
- [x][Must Have] Support variadic arguments for log formatting
- [x][Must Have] Logging context and log scopes for defer logging of information and errors
- [x][Must Have] Glue and stringify macros for log scopes
- [x][Must Have] String indentation formatting
- [x][Must Have] Win32 platform layer high resolution time querying/measuring
- [x][Must Have] Clean up string and ensure code naming is consistent
- [x][Must Have] Win32 file reading, writing, and querying of file attributes
- [x][Must Have] Platform file reading interface
- [x][Must Have] Win32 WinMain wide char argument parsing and conversion for common entry point
- [x][Must Have] String splitting and joining
- [x][Must Have] String utf8 and utf16 conversion functions
- [x][Must Have] String access operator overloading
- [x][Must Have] Update copyright notices to be more concise (less characters)
- [x][Must Have] Update build script to support build targets for building multiple applications
- [x][Must Have] String formatting and integer conversion
- [x][Must Have] Import stb_snprintf for string formatting
- [x][Must Have] String constructors, stylization, matching, slicing
- [x][Must Have] Character (char) classification and conversion functions
- [x][Must Have] Add thread local scratch arenas
- [x][Must Have] Thread context allocation and selection for scratch arenas
- [x][Must Have] Add address sanitizer functions for memory allocations
- [x][Must Have] Arena allocator default parameters for allocation
- [x][Must Have] Arena allocators with chaining and uses virtual memory allocation
- [x][Must Have] Win32 platform layer virtual memory allocation
- [x][Must Have] Update build script to build using c++20 for designated initializers
- [x][Must Have] Move main from viewer compile file to platform layer for common code initialization
- [x][Must Have] Create stub files and core defines for the base layer
- [x][Must Have] Batch script for building as unity build
