cmake_minimum_required(VERSION 2.6)
project( musikCube )

set ( musikCube_VERSION_MAJOR 2)
set ( musikCube_VERSION_MINOR 0)

set (CMAKE_MODULE_PATH ${musikCube_SOURCE_DIR}/modules)

set (BOOST_LIBS
system
date_time
filesystem
iostreams
thread
regex
)

include_directories(${musikCube_SOURCE_DIR}/src/3rdparty/lib)

set (Boost_ADDITIONAL_VERSIONS "1.37.0" )
if(WIN32 AND NOT UNIX)
	find_package(Boost 1.42.0 REQUIRED ${BOOST_LIBS} )
	if (Boost_FOUND)
		add_definitions (
			-DHAVE_BOOST
			)
		#set (BOOST_LINK_LIBS 
		#	${Boost_LIBRARIES})
		include_directories (${Boost_INCLUDE_DIRS})
		link_directories(${Boost_LIBRARY_DIRS})
		#message(STATUS "Boost includes: " ${Boost_INCLUDE_DIRS})
		#message(STATUS "Boost libs: " ${Boost_LIBRARIES})
	else (Boost_FOUND)
		set (BOOST_LINK_LIBS
			boost_system
			boost_iostreams
			boost_filesystem
			boost_thread
			boost_regex
			boost_date_time
			)
		include_directories ("${musikCube_SOURCE_DIR}/src/3rdparty/boost")
	endif (Boost_FOUND)
	#set (VLD_ROOT_DIR ${musikCube_SOURCE_DIR}/src/3rdparty/lib)
	#set (VLD_INCLUDE_DIR ${musikCube_SOURCE_DIR}/src/3rdparty/include)
	#find_package(vld REQUIRED)
	#if (VLD_FOUND)
	#	message(STATUS "VLD Found")
	#	link_directories(${VLD_LIBRARIES_DIR})
	#else (VLD_FOUND)
	#	message(STATUS "VLD not found")
	#endif (VLD_FOUND)
	add_definitions (
		-DUNICODE
		-D_UNICODE
		)
else(WIN32 AND NOT UNIX)
	find_package(Boost 1.42.0 REQUIRED ${BOOST_LIBS}) 
	if(Boost_FOUND)
		add_definitions (-DHAVE_BOOST
				-D_FILE_OFFSET_BITS=64)
		set (BOOST_LINK_LIBS 
			${Boost_LIBRARIES})
	#	message(STATUS "Libs: " ${BOOST_LINK_LIBS})
	#	message(STATUS "Includes: " ${Boost_INCLUDE_DIRS})
	else(Boost_FOUND)
		set (BOOST_LINK_LIBS
			boost_system
			boost_iostreams
			boost_filesystem
			boost_thread
			boost_regex
			boost_date_time
			)
		include_directories ("${musikCube_SOURCE_DIR}/src/3rdparty/boost")
	endif(Boost_FOUND)
	find_package(Qt4 REQUIRED)
	#add_definitions(${QT_DEFINITIONS})
	#include_directories(${QT_INCLUDE_DIR})
	set (linux_LINK_LIBS
		dl
		expat
	)
	#include_directories( "/usr/include/taglib" )
	set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/Modules )
	#include(UseDoxygen)
	set(DOXYFILE_OUTPUT_DIRECTORY docs)
endif(WIN32 AND NOT UNIX)

SET (CMAKE_BUILD_TYPE DEBUG)
set (LIBRARY_OUTPUT_PATH ${musikCube_SOURCE_DIR}/bin/plugins)
set (EXECUTABLE_OUTPUT_PATH ${musikCube_SOURCE_DIR}/bin)

link_directories (${musikCube_SOURCE_DIR}/bin/plugins)

set (musikCube_LINK_LIBS
	${linux_LINK_LIBS}
	${BOOST_LINK_LIBS}
	sqlite3
	md5
)

include_directories (
			"${musikCube_SOURCE_DIR}/src"
			"${musikCube_SOURCE_DIR}/src/core"
			"${musikCube_SOURCE_DIR}/src/core/audio"
			"${musikCube_SOURCE_DIR}/src/3rdparty/include")

add_subdirectory(src/3rdparty)
add_subdirectory(src/core)
add_subdirectory(src/contrib)
add_subdirectory(src/win32cpp)
add_subdirectory(src/server)
add_subdirectory(src/square)
#add_subdirectory(src/cube)
