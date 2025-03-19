@echo off
mkdir X:\repos\builds\openGLTestProject
pushd X:\repos\builds\openGLTestProject
cl /EHsc msvcrt.lib glfw3.lib user32.lib gdi32.lib shell32.lib ..\..\GLFW\testproject\src\testgame.cpp ..\..\GLFW\testproject\src\glad.c  -IX:\glfw-3.4\include /link /libpath:X:\glfw-3.4\src /NODEFAULTLIB:libcmt
popd