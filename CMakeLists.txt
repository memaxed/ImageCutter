cmake_minimum_required(VERSION 4.1.1)
project(imgcut)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp src/cpp/image.cpp src/cpp/imageprocessor.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
