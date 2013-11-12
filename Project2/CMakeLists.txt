cmake_minimum_required (VERSION 2.6)
project (IOProject2)

string(COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "Linux" LINUX)

IF(LINUX)
  set(CMAKE_CXX_COMPILER "g++")
  set(CMAKE_BUILD_TYPE Release)
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -funroll-loops -flto -DNDEBUG -DLINUX")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
ENDIF(LINUX)

IF(APPLE)
  set(CMAKE_CXX_COMPILER "clang++")
  set(CMAKE_BUILD_TYPE Release)
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -funroll-loops -flto -DNDEBUG")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
ENDIF(APPLE)

add_executable(Project2 main.cpp
                        dummy_stream.hpp
                        f_stream.hpp
                        mmap_stream.hpp
                        mmap_file_stream.hpp
                        sys_stream.hpp
                        buffered_stream.hpp
                        external_heap.hpp
                        block.hpp
                        cached_stream.hpp)

add_executable(Project2Test
                        test.cpp
                        dummy_stream.hpp
                        f_stream.hpp
                        mmap_stream.hpp
                        mmap_file_stream.hpp
                        sys_stream.hpp
                        buffered_stream.hpp
                        external_heap.hpp
                        block.hpp
                        cached_stream.hpp)
