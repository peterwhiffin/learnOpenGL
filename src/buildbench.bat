@echo off
mkdir X:\repos\builds\benchmark
pushd X:\repos\builds\benchmark
cl ..\..\learnOpenGL\src\quatbenchmark.cpp -IX:\glfw-3.4\include /link /libpath:X:\glfw-3.4\src
popd