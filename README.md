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
TODO

# Codebase
The codebase has the following top-level structure.
```
toy-viewer
|-- docs         # Development logs and planning documentation
|-- src          # All source code
|-- build.bat    # Build script
`-- README.md
```
After setting up the codebase and building, the following directories will
also exist in the root level of the codebase:
- `bin`: All build artifacts.
- `.tmp`: All intermediate files when building.
