@echo off
echo Building %1.slim
%SLIM_HOME%/slim.exe -i . %1.slim
g++ %1.cpp %1_main.cpp -o %1.exe
echo on