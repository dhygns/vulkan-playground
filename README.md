# Vulkan Playground 🧪

A minimal C++ project to learn and experiment with the Vulkan API.  
This repository walks through the Vulkan setup process using `GLFW` and step-by-step examples.

## ✨ Goals

- Understand Vulkan fundamentals through hands-on examples
- Build a reusable framework for graphics experiments
- Document each step for future reference and learning

## 📦 Dependencies

- [GLFW](https://www.glfw.org/) (for window/context management)
- [Vulkan SDK](https://vulkan.lunarg.com/) (latest version recommended)
- C++17 or later
- CMake (optional, if you're using it to build)

## 🛠️ Build Instructions

### 1. Install Vulkan SDK

Download and install from: https://vulkan.lunarg.com/

Make sure `VULKAN_SDK` is correctly set in your environment.

### 2. Clone the Repository

```bash
git clone https://github.com/your-username/vulkan-playground.git
cd vulkan-playground
```

### 3. Build (using g++)
```bash
g++ main.cpp -o VulkanApp -lglfw -lvulkan
./VulkanApp
```
Or use your favorite IDE or CMake setup.

## 📁 Project Structure
```bash
.
├── main.cpp          # Entry point with Vulkan instance creation
├── CMakeLists.txt    # (Optional) CMake build script
└── README.md
```

## 📌 Notes
- This project starts with instance creation and will gradually add physical device selection, swapchain setup, rendering pipeline, and more.
- Each commit or branch may correspond to a learning milestone.

## 📚 References
- [Vulkan Tutorial](https://vulkan-tutorial.com/)
- [Official Vulkan Docs](https://registry.khronos.org/vulkan/specs/)

