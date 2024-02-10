### Yk

An attempt at a Vulkan renderer

2/10 1323

<img src="https://github.com/oh-facts/Yk/assets/125090383/3ac7e48e-3672-4aa1-9598-d3bef4b6ad89" alt="drawing" width="900"/>

## Quickstart

```
git clone https://github.com/oh-facts/Yk --recursive
```

Unzip [this](https://drive.google.com/drive/folders/1HdYLU3ol2YaoFkgIoVk8wFlecx9lTBLY?usp=sharing) (drive link) and put `res/` in the root directory.

```
cd yk/scripts

./innit.bat
./shaderc.bat
./cmakec.bat
./ninjac.bat
```

binaries are built in `.out/`

A symbolic link for `res/` is created inside `.out/`. Your OS may disallow this and throw an error. Allow this. If you are on windows,
enabling developer mode will fix this.

look at `scripts/cmakec.bat` to see the cmake command being used. I leave it as a bat file for ease of writing this.

`res` folder **must** be unzipped and placed in root directory.

It should look something like this
```
Yk/
├─ scripts/
├─ src/
├─ res/
│  ├─ models
│  ├─ textures
├─ README.md
...
```

All dependencies are inside `vendor/` so no need to download anything. However, the Vulkan SDK is required to compile the renderer

Here is the cmake command incase you are interested
```
cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE="Debug" -DYK_BUILD_SHARED=1 -G "Ninja"
```

`-DYK_BUILD_SHARED=1` is compulsory. Pick whatever toolchain you like. I test on msvc and clang + ninja.

  ## Note:

- Only compiles on windows currently. Linux is next. Won't compile on mac ever unless Apple decides to get their shit together (Apple's NiH syndrome is worse than mine).

- Vulkan 1.3 support is required. Please stop emailing me saying that it doesn't run.

- cmake and ninja is optional. look at `scripts/ninjac.bat` and the `cmakelists.txt` if you want to write your own build cmd.

- vulkan spir-v tools is needed (download vulkan sdk) to compile shaders to spir-v.

- feel free to contribute. I won't merge if I don't like the code. I rarely like what I write myself.

- I will make a wiki to better explain design goals

- engine is foss. Do whatever you want with it, except claiming you made it. Games that I make with it will be property of my studio, dear dear

- 21st century schizoid man


## credits

- **[fire in the sky](https://sketchfab.com/3d-models/fire-in-the-sky-06d903f5dc5245699adf38192f77fbb6)**: model in the screenshot, made by Seo hyewon

-  **[cgltf](https://github.com/jkuhlmann/cgltf/tree/master)**: gltf model parser

-  **[glm](https://github.com/g-truc/glm/tree/master)**: math
  
-  **[stb_image](https://github.com/nothings/stb)**: image loader
  
- **[vma](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)**: allocator for vulkan resources

- **[vulkan sdk](https://www.lunarg.com/vulkan-sdk/)**: graphics api
