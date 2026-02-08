# Animation System

This project implements a simple animation system for humanoid characters.

## Features

- **Skeleton Loading**: Loads skeleton data from `.skel` files.
- **Skin Loading**: Loads skin data from `.skin` files.
- **Animation Loading**: Loads animation data from `.anim` files.
- **Animation Evaluation**: Evaluates animations and applies them to the skeleton.
- **Interactive Control**: Allows interactive control of the skeleton's degrees of freedom (DOFs).

## Usage

```bash
.\build\Debug\menv.exe <skeleton_file> <skin_file> <animation_file>
```

### Controls

- **[UP / DOWN ARROW]**: Cycle through joints
- **[LEFT / RIGHT ARROW]**: Cycle through DOFs (Rotate X, Y, Z)
- **[+ / =]**: Increase current DOF value
- **[-]**: Decrease current DOF value
- **[R]**: Reset camera position
- **[0]**: Reload the current skeleton/skin file
- **[ESC]**: Exit application

## Files

- `include/Skeleton.h`: Skeleton class definition
- `src/Skeleton.cpp`: Skeleton class implementation
- `include/Skin.h`: Skin class definition
- `src/Skin.cpp`: Skin class implementation
- `include/Animation.h`: Animation class definition
- `src/Animation.cpp`: Animation class implementation
- `main.cpp`: Main application entry point
- `shaders/`: GLSL shaders for rendering

## Build

```bash
cmake -B build -S .
cmake --build build --config Release
```

## Results
![Demo](demo/demo.mp4)
