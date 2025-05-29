cmake_minimum_required(VERSION 3.2) 

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "-DNDEBUG")

project(BitTorrent
        VERSION 1.0
        DESCRIPTION "simple torrenting app"
        LANGUAGES C CXX
        )

include_directories(src/include)

set(BINARY ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${BINARY} ${SOURCES})
target_link_libraries(${BINARY} -lcrypto -lpthread)

add_library(${BINARY}_lib STATIC ${SOURCES})
target_link_libraries(${BINARY}_lib -lcrypto -lpthread)



