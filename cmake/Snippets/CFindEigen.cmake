# Eigen, header only. Also using unsupported headers for advanced functions
SET(Eigen_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/eigen ${CMAKE_SOURCE_DIR}/extern/eigen/unsupported)
# Disable this when you need vectorization (maybe for speed?)
#IF(WIN32)
	#ADD_DEFINITIONS(-DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT)
	#MESSAGE("DISABLING EIGEN VECTORIZATION ON WIN32. MAY AFFECT PERFORMANCE.")
#ENDIF()
#MESSAGE(STATUS "Set Eigen includes directory to ${Eigen_INCLUDE_DIR}")
MESSAGE(STATUS "Found Eigen")