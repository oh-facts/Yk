@echo off
pushd ..\res\
glslc default.vert -o default.vert.spv
glslc default.frag -o default.frag.spv
glslc gradient.comp -o gradient.comp.spv
popd
