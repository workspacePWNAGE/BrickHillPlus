#!/bin/bash

set -e

clear
echo "compiling"

rm -rf Build/
mkdir Build/
cp -r Content/ Build/Content/
g++ $(wx-config --cxxflags) source/main.cpp $(wx-config --libs) -o Build/BHP

echo "done"