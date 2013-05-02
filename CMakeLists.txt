cmake_minimum_required (VERSION 2.6)
project (AsciiIngest.x)
set(AIDIR "${PROJECT_SOURCE_DIR}/AsciiIngest")

############################################################################
## SET THIS TO THE MYSQL BASE PATH IF IT IS NOT AT A STANDARD PLACE
############################################################################
set(MYSQL_BASE_PATH "/usr/local/mysql")

if(NOT DEFINED DBINGESTOR_LIBRARY_PATH)
       set(DBINGESTOR_LIBRARY_PATH "/usr/local/lib")
endif()

if(NOT DEFINED DBINGESTOR_INCLUDE_PATH)
       set(DBINGESTOR_INCLUDE_PATH "/usr/local/include")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/Modules/")

set(CMAKE_BUILD_TYPE Release)
set(SQLITE3_BUILD_IFFOUND 1)
set(MYSQL_BUILD_IFFOUND 1)
set(ODBC_BUILD_IFFOUND 1)

include_directories ("${PROJECT_SOURCE_DIR}/AsciiIngest")
include_directories ("${DBINGESTOR_INCLUDE_PATH}")
link_directories ("${DBINGESTOR_LIBRARY_PATH}")

if(MSVC)
set(CMAKE_CXX_FLAGS "/EHsc")
else()
#set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++0x")
endif()

file(GLOB FILES_SRC "${AIDIR}/*.h" "${AIDIR}/*.cpp")

#MESSAGE(STATUS "Dir: " ${DIDIR})

SET(Boost_USE_MULTITHREAD ON)
find_package (Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

find_package (SQLITE3)
message("Found SQLITE3: ${SQLITE3_FOUND}")
if(SQLITE3_FOUND AND SQLITE3_BUILD_IFFOUND)
	include_directories(${SQLITE3_INCLUDE_DIR})
	add_definitions(-DDB_SQLITE3)
endif()

find_package (Mysql)
message("Found MySQL: ${MYSQL_FOUND}")
if(MYSQL_FOUND AND MYSQL_BUILD_IFFOUND)
	include_directories(${MYSQL_INCLUDE_DIR})
	add_definitions(-DDB_MYSQL)
endif()

find_package (ODBC)
message("Found ODBC: ${ODBC_FOUND}")
if(ODBC_FOUND AND ODBC_BUILD_IFFOUND)
	include_directories(${ODBC_INCLUDE_DIRS})
	add_definitions(-DDB_ODBC)
endif()

add_executable (AsciiIngest.x ${FILES_SRC})

target_link_libraries(AsciiIngest.x ${Boost_PROGRAM_OPTIONS_LIBRARY} DBIngestor)

if(SQLITE3_FOUND)
        target_link_libraries(AsciiIngest.x ${SQLITE3_LIBRARIES})
endif()

if(MYSQL_FOUND)
        target_link_libraries(AsciiIngest.x ${MYSQL_LIBRARY})
endif()

if(ODBC_FOUND)
        target_link_libraries(AsciiIngest.x ${ODBC_LIBRARIES})
endif()

