CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(PACKAGE mongoose)
PROJECT(${PACKAGE})

# Always build release. We never have to debug these packages
SET(CMAKE_BUILD_TYPE Release)

# Need PThread on Linux
FIND_PACKAGE(Threads)
# mongoose macro for WebSocket
ADD_DEFINITIONS(-DUSE_WEBSOCKET)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w -O3") # Disable all warnings and enable optimization. This is C
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Set sources
SET(SRCS mongoose.h mongoose.c)

# Build a static/shared lib for linking
IF(WIN32)
	ADD_LIBRARY(${PACKAGE} STATIC ${SRCS})
ELSE()
	ADD_LIBRARY(${PACKAGE} SHARED ${SRCS})
ENDIF()
TARGET_LINK_LIBRARIES(${PACKAGE} ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
