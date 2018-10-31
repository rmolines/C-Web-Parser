cmake_minimum_required(VERSION 3.0.0)
project(webparser VERSION 0.1.0)

add_definitions(-O2 -pg -std=c++11)
add_subdirectory(cpr)

set(CURL_LIBRARY "-lcurl") 
set(PLATFORM_SPECIFIC_LIBS "-pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pg -std=c++11")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(Boost 1.4 COMPONENTS mpi serialization REQUIRED)
find_package (CURL)
find_package(MPI REQUIRED)

link_directories(/usr/local/lib)
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${CPR_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${MPI_INCLUDE_PATH})

# add_executable(webparser main_seq.cpp semaphore.cpp)
# add_executable(parallel_webparser main.cpp semaphore.cpp)
add_executable(mpi_webparser main_mpi.cpp)

# target_link_libraries(parallel_webparser nlohmann_json::nlohmann_json ${CURL_LIBRARIES} gumbo ${CPR_LIBRARIES})
# target_link_libraries(webparser nlohmann_json::nlohmann_json ${CURL_LIBRARIES} gumbo ${CPR_LIBRARIES})
target_link_libraries(mpi_webparser nlohmann_json::nlohmann_json ${CURL_LIBRARIES} gumbo ${CPR_LIBRARIES}
					${Boost_LIBRARIES} ${MPI_LIBRARIES})

# set_target_properties(webparser PROPERTIES COMPILE_FLAGS "-O2 -pg -std=c++11")


