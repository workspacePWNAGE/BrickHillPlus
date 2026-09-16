#!/bin/bash

set -e

clear
echo "compiling..."

rm -rf Build/
mkdir -p Build/Windows/
mkdir -p Build/Linux/
cp -r Content/ Build/Linux/Content/
cp -r Content/ Build/Windows/Content/

echo "linux"
g++ $(wx-config --cxxflags) source/main.cpp $(wx-config --libs) -static-libgcc -static-libstdc++ -o Build/Linux/BHP

echo "windows"
cp Libs/* Build/Windows/
x86_64-w64-mingw32-g++ $(x86_64-w64-mingw32-wx-config-static --cxxflags) source/main.cpp -static -L/usr/x86_64-w64-mingw32/static/lib $(x86_64-w64-mingw32-wx-config-static --libs) -o Build/Windows/BHP.exe

echo "done"