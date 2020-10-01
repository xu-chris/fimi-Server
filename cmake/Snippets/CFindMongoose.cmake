SET(MONGOOSE_ROOT_PATH ${CMAKE_SOURCE_DIR}/extern/mongoose)
SET(MONGOOSE_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/extern/mongoose/ ${CMAKE_SOURCE_DIR}/extern/mongoose/mongoose)

if (MSVC_VERSION EQUAL 1900) # this is Visual Studio 14 2015
	LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/extern/mongoose/mongoose/lib/vc140)
	SET(MONGOOSE_LIBS optimized mongoose debug mongoosed)
else() # assume Linux
	LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/extern/mongoose/mongoose/lib/linux)
	SET(MONGOOSE_LIBS mongoose)
endif()
SET(MONGOOSE_FOUND ON)