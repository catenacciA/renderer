# Real-Time Renderer with Metal

A (timid) real-time 3D renderer written in C++ with GPU acceleration via Metal.

![](https://i.postimg.cc/VvmcHbKB/Screenshot.png)


## Project Structure

<details>
<summary>Detailed File Breakdown</summary>

```
├── build/
│   └── renderer             # Compiled binary
├── include/
│   ├── AppDelegate.hpp
│   ├── Math.hpp              # Utility functions for vector and matrix math  
│   ├── Mesh.hpp            # Represents 3D models and geometry data
│   ├── MyMTKViewDelegate.hpp
│   └── Renderer.hpp
├── src/
│   ├── AppDelegate.cpp     # Manages the application
│   ├── Main.cpp
│   ├── MyMTKViewDelegate.cpp # Custom delegate for the MetalKit view
│   ├── Renderer.cpp        # Main rendering logic
│   └── shader.metal         # Metal shading code
├── third-party/
│   ├── metal-cpp/           # Metal C++ bindings
│   └── metal-cpp-extensions/
```

</details>

## Features

- **Real-Time 3D Rendering** using Metal
- **Custom Vertex & Fragment Shaders** for lighting and materials
- **Dynamic Point Lights** with attenuation and time-based pulsing
- **Modular Codebase** for easy extension and reuse

### Shader Overview

The Metal shader (`shader.metal`) includes:

- **Vertex Shader**
  - Transforms vertices into clip space
  - Calculates per-vertex world positions, normals, and colors
- **Fragment Shader**
  - Computes lighting using:
    - Ambient, diffuse, and specular components
    - Distance-based attenuation
    - Time-based pulsing effects


## Getting Started

### Prerequisites

- macOS with Xcode and Metal support
- `make`, `clang` installed

### Build Instructions

Run the following from the root directory:

```sh
make -j8
```

The output binary will be located at `./build/renderer`.

### Run the Renderer

```sh
./build/renderer
```


## 📄 License

This project is licensed under the terms outlined in the [LICENSE](LICENSE) file.

## Acknowledgments

- [Apple Metal](https://developer.apple.com/metal/) for the GPU framework
- [metal-cpp](third-party/metal-cpp/README.md) for enabling C++ bindings to Metal