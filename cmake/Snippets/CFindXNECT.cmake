# Eigen, header only. Also using unsupported headers for advanced functions
SET(xnect_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/xnect/include )

if (MSVC_VERSION EQUAL 1900) # this is Visual Studio 14 2015
	LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/extern/xnect/lib/vc140)
else() # assume linux
	LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/extern/xnect/lib/linux)

endif()

SET(xnect_LIBS xnect)
SET(xnect_FOUND ON)