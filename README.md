# Bouncing Balls - Qt / C++ Project

## Requirements (Ubuntu Linux)
```bash
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev qt6-multimedia-dev libqt6multimediawidgets6
```
*(If Qt5 is used, install `qtbase5-dev` and `qtmultimedia5-dev`)*

## Build & Run Instructions
```bash
chmod +x setup_project.sh
./setup_project.sh
```
Or manually:
```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
./BouncingBalls
```
