cmake_minimum_required(VERSION 3.27)
project(LearningEnet)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SRC_FILES
        src/*.cpp
        src/*.h
)

add_executable(LearningEnet ${SRC_FILES})
target_include_directories(LearningEnet PRIVATE src)
target_include_directories(LearningEnet PRIVATE lib/cereal/include)
