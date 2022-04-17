cmake_minimum_required(VERSION 3.21)
project(ErrorHandling)

set(CMAKE_CXX_STANDARD 20)

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(REQUIRES
                doctest/2.4.8
                fmt/8.1.1
                ms-gsl/4.0.0
                BASIC_SETUP
                BUILD missing)

add_executable(ErrorHandling main.cpp result.h storage.h error.h macros.h assert.h define_error.h common_errors.h formatting.h)
target_link_libraries(ErrorHandling ${CONAN_LIBS})
