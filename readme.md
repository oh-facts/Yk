### Yk

An attempt at a Vulkan renderer

1/26 2122

<img src="https://github.com/oh-facts/Yk/assets/125090383/812c393e-b138-4b24-b8ef-93e82e46b852" alt="drawing" width="900"/>

## Quickstart

```
git clone https://github.com/oh-facts/Yk --recursive

cd yk/scripts

./shaderc.bat
./ninjac.bat
```

binaries are built in `out/`

`res` folder **must** be unzipped and placed in root directory. Here is the [drive link](https://drive.google.com/file/d/1Xw25ORHTO86MLTrudOooCfAxHa2gG-ZH/view?usp=sharing)

  ## Note:

- Only compiles on windows currently. Linux is next. Won't compile on mac ever unless Apple decides to get their shit together (Apple's NiH syndrome is worse than mine).

- Vulkan 1.3 support is required. Please stop emailing me saying that it doesn't run.

- cmake and ninja is optional. look at `scripts/ninjac.bat` and the `cmakelists.txt` if you want to write your own build cmd.

- vulkan spir-v tools is needed (download vulkan sdk) to compile shaders to spir-v.

- feel free to contribute. I won't merge if I don't like the code. I rarely like what I write myself.

- I will make a wiki to better explain design goals

-  engine is open source. games I make with it will be property of my studio, dear dear

- 21st century schizoid man


## 3rd party libraries

-  **[cgltf](https://github.com/jkuhlmann/cgltf/tree/master)**: gltf model parser

-  **[glm](https://github.com/g-truc/glm/tree/master)**: math
  
-  **[stb_image](https://github.com/nothings/stb)**: image loader
  
- **[vma](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)**: allocator for vulkan resources

- **[vulkan sdk](https://www.lunarg.com/vulkan-sdk/)**: graphics api
