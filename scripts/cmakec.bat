@echo off
pushd ..\.out\
cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE="Debug" -DYK_BUILD_SHARED=1 -G "Ninja"
popd