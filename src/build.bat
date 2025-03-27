@echo off
mkdir X:\repos\builds\openGLTestProject
pushd X:\repos\builds\openGLTestProject
cl /EHsc /Zi /DEBUG msvcrt.lib glfw3.lib user32.lib gdi32.lib shell32.lib X:\\repos\\learnOpenGL\\src\\testgame.cpp X:\\repos\\learnOpenGL\\src\\input.cpp X:\\repos\\learnOpenGL\\src\\imageloader.cpp X:\\repos\\learnOpenGL\\src\\glad.c -IX:\\glfw-3.4\\include -IX:\\assimp\\include /link /libpath:X:\\glfw-3.4\\src /libpath:X:\\assimp\\build\\lib assimp-vc143-mt.lib /NODEFAULTLIB:libcmt"
testgame.exe
popd
