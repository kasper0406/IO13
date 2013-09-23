cmake_minimum_required (VERSION 2.6)
project (IOProject1)

string(COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "Linux" LINUX)

IF(LINUX)
  set(CMAKE_CXX_COMPILER "g++")
  set(CMAKE_BUILD_TYPE Release)
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
ENDIF(LINUX)

add_executable(Project1 main.cpp
                        input_stream.h
                        output_stream.h
                        test.h
                        utils.h
                        stream.h
                        merge_sort.h
                        f_stream.h
                        fread_input_stream.h
                        fwrite_output_stream.h
                        mmap_stream.h
                        mmap_input_stream.h
                        mmap_output_stream.h
                        buffered_input_stream.h
                        buffered_output_stream.h
                        read_input_stream.h
                        write_output_stream.h
                        rw_stream.h)
