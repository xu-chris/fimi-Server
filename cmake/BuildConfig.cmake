SET(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "possible configurations" FORCE)
# In case the user does not setup CMAKE_BUILD_TYPE, assume it's Debug
IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "build type default to Debug, set to Release to improve performance" FORCE)
ENDIF()

# Compiler flags and output directories
# Reset CXX flags before starting
SET(CMAKE_CXX_FLAGS "")
IF(MSVC)
	#We statically link to reduce dependancies
	FOREACH(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		IF(${flag_var} MATCHES "/MD")
			STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		ENDIF(${flag_var} MATCHES "/MD")
		IF(${flag_var} MATCHES "/MDd")
			STRING(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
		ENDIF(${flag_var} MATCHES "/MDd")
	ENDFOREACH(flag_var)

	# This is possibly needed for MSVC. See http://social.msdn.microsoft.com/forums/en-US/Vsexpressvc/thread/4cf463c1-5fee-4daa-b135-be8b06ab7f8f/
	ADD_DEFINITIONS(-DNOMINMAX)
	# Another MSVC quirk. http://stackoverflow.com/questions/6832666/lnk2019-when-including-asio-headers-solution-generated-with-cmake
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MP") # For multi-threaded compilation on MSVC

	# Enable source grouping directories
	SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
ELSE()
	IF("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0") # Disable optimization for faster builds
		#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3") # Use this is too slow in debug
		MESSAGE(STATUS "Building without optimization flags (-O0) in Debug mode.")
	ELSE()
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
	ENDIF()
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")# C++11 support on GCC
ENDIF()

# Have a separate binary directory
IF(MSVC)
  # Yet another MSVC thingy http://stackoverflow.com/questions/3457013/visual-studio-2008-creates-a-additional-bin-debug-folder-although-i-have-no-pre
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
ELSE()
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
ENDIF()

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

IF(MSVC)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
ENDIF()