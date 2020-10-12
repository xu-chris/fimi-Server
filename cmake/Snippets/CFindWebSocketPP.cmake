# Header only. Also using unsupported headers for advanced functions
SET(WebSocketPP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/websocketpp )

# Add the third-party headers directory to the search path
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/extern")

# Force WebSocket++ to use 100% C++11 mode, so that it doesn't try to look for Boost
# (Note that under MinGW, Boost.Thread is still required, due to a bug in MinGW that prevents the C++11 version from being used)
add_definitions(-D_WEBSOCKETPP_CPP11_STRICT_)
SET(WebSocketPP_FOUND ON)
MESSAGE(STATUS "Found WebSocket++")