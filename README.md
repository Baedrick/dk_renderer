# Toy Renderer
Real-time, physically based, gpu-driven toy renderer build with OpenGL.

# AI Usage Declaration
I used Large Language Models (LLMs) as a tool to assist with specific
development tasks without replacing the core work of building the rendering
engine. AI was primarily used to help understand and debug complex external
libraries or inadequately documented APIs. I also used AI to generate initial
proof-of-concept ideas/implementations for isolated features. In all cases,
these prototypes served only as references; they are completely rewritten by
hand to ensure they meet the renderer's paradigms and performance constraints.
Finally, I used AI to check spelling and improve the phrasing of my
documentation for better clarity.

Beyond these specific uses, I am solely responsible for the project's design,
architecture, and every line of code in the final implementation.

# Building
To build the binaries from source, you'll need [Microsoft C/C++ Build Tools v17
(2022) or later](https://aka.ms/vs/17/release/vs_buildtools.exe). The codebase
only supports compiling and linking with MSVC.

### Build Steps
Building the codebase is done through the terminal which is configured to call
MSVC from the commandline. This is done by calling `vcvarsall.bat x64` which is
included in the Microsoft C/C++ Build Tools. This script is automatically called
by `x64 Native Tools Command Prompt for VS <year>`. This command prompt can be
found by searching for `Native` from the Start Menu search.

Ensure that MSVC compiler is accessible from the command line by running:
```
cl
```
If everything is set up correctly, you should see something like this:
```
Microsoft (R) C/C++ Optimizing Compiler Version 19.44.35221 for x64
Copyright (C) Microsoft Corporation.  All rights reserved.

usage: cl [ option... ] filename... [ /link linkoption... ]
```
From the terminal, `cd` to the root directory of the repository and run
`build.bat` script by running the command. For a debug build:
```
build
```
For a release build:
```
build release
```
You should have similar output to the following:
```
[release mode]
[msvc compile]
[default mode, assuming `all` build]
[building all targets]
[building shaders]
viewer_main.cpp
cooker_main.cpp
```
If everything worked correctly, there will be a `bin` folder containing all
artifacts required for the viewer to function. By default, `build.bat` builds
all build targets in debug mode. See more build configuration information in
`build.bat`.

# Codebase
The codebase has the following top-level structure.
```
toy-viewer
|-- docs         # Development logs and planning documentation
|-- src          # All source code
|-- tools        # Tools used for builds
|-- build.bat    # Build script
`-- README.md
```
After setting up the codebase and building, the following directories will
also exist in the root level of the codebase:
- `bin`: All build artifacts.
- `.tmp`: All intermediate files when building.
