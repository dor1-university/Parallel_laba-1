cmake_minimum_required(VERSION 3.20)
project(Laba_1)

set(CMAKE_CXX_STANDARD 20)

add_executable(main_app main.cpp)
add_executable(task1_matrix task1_matrix.cpp)
add_executable(task2_bank task2_bank.cpp)

find_package(Boost REQUIRED COMPONENTS thread system)
if(Boost_FOUND)
    target_include_directories(task1_matrix PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(task1_matrix PRIVATE ${Boost_LIBRARIES})
endif()
