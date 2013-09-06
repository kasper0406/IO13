cmake_minimum_required (VERSION 2.6)
project (IOProject1)

string(COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "Linux" LINUX)

IF(${FALSE}) # LINUX!
#  set(CMAKE_CXX_COMPILER "clang++")
  set(CMAKE_BUILD_TYPE Release)
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -std=c++0x -march=corei7-avx -mtune=corei7-avx -flto -funroll-loops -fpermissive -save-temps -g")

  add_library(
    ipcm_lib
    STATIC
    EXCLUDE_FROM_ALL
    /home/espeholt/ipcm/msr.o
    /home/espeholt/ipcm/pci.o
    /home/espeholt/ipcm/cpucounters.o
  )

  set_target_properties(
    ipcm_lib
    PROPERTIES
    LINKER_LANGUAGE CXX
  )
ENDIF(${FALSE})

IF(APPLE)
  set(CMAKE_CXX_COMPILER "clang++")
  set(CMAKE_BUILD_TYPE Release)
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
ENDIF(APPLE)

add_executable(Project1 main.cpp
                        input_stream.h
                        output_stream.h
                        test.h
                        utils.h
                        stream.h
                        f_stream.h
                        fread_input_stream.h
                        fwrite_output_stream.h
                        mmap_stream.h
                        mmap_input_stream.h
                        mmap_output_stream.h)
