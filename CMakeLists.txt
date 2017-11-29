cmake_minimum_required(VERSION 3.8)
project(Gacha)

set(CMAKE_CXX_STANDARD 11)

# Add by YM -BRGIN-
set(CMAKE_C_FLAGS -O3 -mtune=native -march=native -mfpmath=both)
set(CMAKE_CXX_FLAGS -O3)
# Add by YM -END-

set(SOURCE_FILES main.cpp)
add_executable(Gacha ${SOURCE_FILES})