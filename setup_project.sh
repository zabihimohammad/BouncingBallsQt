#!/bin/bash
set -e

echo "=== Installing dependencies on Ubuntu ==="
sudo apt-get update
sudo apt-get install -y build-essential cmake qt6-base-dev qt6-multimedia-dev libqt6multimediawidgets6 || sudo apt-get install -y build-essential cmake qtbase5-dev qtmultimedia5-dev

echo "=== Building Bouncing Balls Project ==="
mkdir -p build
cd build
cmake ..
make -j$(nproc)

echo "=== Build Complete! Run with: ./build/BouncingBalls ==="
