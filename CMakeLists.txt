cmake_minimum_required(VERSION 3.15)
project(MorseTrainer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(morse_trainer
    src/main.cpp
    src/morse.cpp
    src/audio.cpp
    src/ui.cpp
    src/trainer.cpp
    src/stats.cpp
    src/utils.cpp
    src/input.cpp
)

target_include_directories(morse_trainer PRIVATE src)

if(WIN32)
    target_link_libraries(morse_trainer PRIVATE winmm)
elseif(UNIX)
    target_link_libraries(morse_trainer PRIVATE pthread)
endif()

install(TARGETS morse_trainer DESTINATION bin)
