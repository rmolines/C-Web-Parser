cmake_minimum_required(VERSION 3.0.0)
project(webparser VERSION 0.1.0)

set(CURL_LIBRARY "-lcurl") 
add_executable(webparser main.cpp json.hpp)

find_package (CURL)
if (CURL_FOUND)
	include_directories(${CURL_INCLUDE_DIRS})
	target_link_libraries(webparser ${CURL_LIBRARIES})
else()
	MESSAGE(FATAL_ERROR "LibCurl is not found in your system.")
endif (CURL_FOUND)

add_subdirectory(cpr)

include_directories(${CPR_INCLUDE_DIRS})

link_directories(/usr/local/lib)


target_link_libraries(webparser ${CPR_LIBRARIES})
target_link_libraries(webparser gumbo) 


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
