cmake_minimum_required(VERSION 2.8)
project(embedded-python-threads)

# Turn on C++11 support (for threading)
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# And now back to our regularly scheduled broadcast.
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

# Find Python 2.7 (for Python access)
FIND_PACKAGE(PythonLibs 2.7 REQUIRED)
include_directories( ${PYTHON_INCLUDE_DIRS} )
target_link_libraries(${PROJECT_NAME} ${PYTHON_LIBRARIES})
