CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9)

SET(PROJECT_NAME pose_estimator)
PROJECT(${PROJECT_NAME})
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

FIND_PACKAGE(Threads)

#set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -gencode arch=compute_32,code=sm_32") # ?
#set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -gencode arch=compute_35,code=sm_35") # ?
set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -gencode arch=compute_70,code=sm_70") # RTX 2080 (via Volta) (Win/Linux Machine)
set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -gencode arch=compute_72,code=sm_72") # RTX 2080 (via Volta) (Win/Linux Machine)
message(STATUS "CUDA_NVCC_FLAGS: ${CUDA_NVCC_FLAGS}")

IF(POLICY CMP0020)
  CMAKE_POLICY(SET CMP0020 OLD)
ENDIF()
IF(POLICY CMP0043)
  CMAKE_POLICY(SET CMP0043 OLD)
ENDIF()
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/BuildConfig.cmake)

# Find necessay extern packages
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/FindPackages.cmake)

# Optional packages only if they are installed or requested for
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/FindOptionalPackages.cmake)

# Include external libs as system to avoid getting their warnings
INCLUDE_DIRECTORIES(SYSTEM ${EXTERN_INCS})
# Then include other directories
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src
  )

ADD_SUBDIRECTORY(src)


SET(ALL_FILES ${ALL_CPP_FILES} ${ALL_HPP_FILES})
# Source group for Visual Studio
FOREACH(f ${ALL_FILES})
  FILE(RELATIVE_PATH SRCGR ${CMAKE_CURRENT_SOURCE_DIR} ${f})

  # Extract the folder, ie remove the filename part
  STRING(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})

  # Source_group expects \\ (double antislash), not / (slash)
  STRING(REPLACE / \\ SRCGR ${SRCGR})
  # MESSAGE(${f})
  # MESSAGE(${SRCGR})
  SOURCE_GROUP("${SRCGR}" FILES ${f})
ENDFOREACH()

cuda_add_executable(${PROJECT_NAME} ${ALL_CPP_FILES} ${ALL_HPP_FILES})

#MESSAGE(STATUS "All external libs: ${EXTERN_LIBS}")
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${EXTERN_LIBS})
list(APPEND CUDA_NVCC_FLAGS -Xcompiler;-some flag)
