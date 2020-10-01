# OpenCV
FIND_PACKAGE(OpenCV 3 REQUIRED)
IF(OpenCV_FOUND)
  #MESSAGE(STATUS "Found OpenCV includes at ${OpenCV_DIR} and libs at ${OpenCV_LIBS}")
  INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS}) # Not needed for CMake >= 2.8.11
  MESSAGE(STATUS "Found OpenCV")
ENDIF()
