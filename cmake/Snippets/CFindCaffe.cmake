# search for Caffe
FIND_PACKAGE(Caffe)
IF(Caffe_FOUND)
  INCLUDE_DIRECTORIES(${Caffe_INCLUDE_DIRS}) # Not needed for CMake >= 2.8.11
  MESSAGE(STATUS "Found Caffe")
ENDIF()
