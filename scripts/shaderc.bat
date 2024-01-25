@echo off
pushd ..\src\gpu
glslc default.vert -o ..\..\res\shaders\default.vert.spv
glslc default.frag -o ..\..\res\shaders\default.frag.spv
glslc gradient.comp -o..\..\res\shaders\gradient.comp.spv
glslc mesh.vert -o ..\..\res\shaders\mesh.vert.spv
glslc tex_img.frag -o ..\..\res\shaders\tex_img.frag.spv
popd
