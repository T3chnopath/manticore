# Packages
Nix Package managment still needs to be implemented. In the meantime, users must install CMake and gcc-arm.

**In Terminal**:
```
sudo apt install cmake
sudo apt install gcc-arm-none-eabi
```

# Flashing and Debugging
The Modular Vehicle Electronics System leverages the Segger suite of utilities.
Users must install Segger JLinkCommander for flashing, and Segger Ozone for debugging. Both are found [here](https://www.segger.com/downloads/jlink/).

# Building 
### Manually 
```
    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DMCU=< STM32H503 | STM32H563 > ..
    make
```
### Script
Run the build script in the project root:

``` ./scripts/build.sh < STM32H503 | STM32H563 > ```

