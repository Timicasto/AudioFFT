cmake_minimum_required(VERSION 3.24)
project(AudioFFT)

find_package(ALSA REQUIRED)
find_package(PulseAudio REQUIRED)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)

set(CMAKE_CXX_FLAGS_RELEASE -Og)

include_directories(includes)

add_executable(AudioFFT main.cpp capture.cpp capture.h GL.cpp GL.h glad.c Engine.cpp Engine.h Device.cpp Device.h)

set(PULSE_LIBS ${PULSE_LIBS} "pulse")
target_link_libraries(AudioFFT GL glfw pulse pulse-simple ${PULSEAUDIO_LIBRARIES})
include_directories(${PULSEAUDIO_INCLUDE_DIRS})
