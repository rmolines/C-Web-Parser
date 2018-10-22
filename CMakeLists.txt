cmake_minimum_required(VERSION 3.0.0)
project(webparser VERSION 0.1.0)

# set(CMAKE_CXX_STANDARD 11)
# set(THREADS_PREFER_PTHREAD_FLAG ON)
# find_package(Threads REQUIRED)

add_definitions(-O2 -pg -std=c++11)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pg -std=c++11 -pthread")

set(CURL_LIBRARY "-lcurl") 
add_executable(webparser main_seq.cpp semaphore.cpp)
set_target_properties(webparser PROPERTIES COMPILE_FLAGS "-O2 -pg -std=c++11")

add_executable(parallel_webparser main.cpp semaphore.cpp)

find_package(nlohmann_json 3.2.0 REQUIRED)


find_package (CURL)
if (CURL_FOUND)
	include_directories(${CURL_INCLUDE_DIRS})
else()
	MESSAGE(FATAL_ERROR "LibCurl is not found in your system.")
endif (CURL_FOUND)

add_subdirectory(cpr)

include_directories(${CPR_INCLUDE_DIRS})

link_directories(/usr/local/lib)



SET(PLATFORM_SPECIFIC_LIBS "-pthread")


target_link_libraries(parallel_webparser nlohmann_json::nlohmann_json ${CURL_LIBRARIES} gumbo ${CPR_LIBRARIES})
target_link_libraries(webparser nlohmann_json::nlohmann_json ${CURL_LIBRARIES} gumbo ${CPR_LIBRARIES})


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
