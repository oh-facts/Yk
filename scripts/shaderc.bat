@echo off
pushd ..\res\
glslc default.vert -o vert.spv
glslc default.frag -o frag.spv
glslc gradient.comp -o gradient.comp.spv
popd
