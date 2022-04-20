cmake_minimum_required(VERSION 3.5)
project(aru CXX)

if (WIN32)
    include(${CMAKE_TOOLCHAIN_FILE})
endif ()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME}
    controllers/authorization.cc
    controllers/authorization.hh
    controllers/beatmaps.cc
    controllers/beatmaps.hh
    controllers/settings.cc
    controllers/settings.hh
    controllers/unspecified.cc
    controllers/unspecified.hh
    controllers/users.cc
    controllers/users.hh
    utility/convert.cc
    utility/convert.hh
    utility/helpers.cc
    utility/helpers.hh
    utility/rate_limit.cc
    utility/rate_limit.hh
    utility/tokens.cc
    utility/tokens.hh
    main.cc
)

# Clang doesn't supported by drogon
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++20 -fcoroutines")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DNOMINMAX /UIN /std:c++latest")
else ()
    message(WARNING "unknown compiler, trying to use default configuration for coroutines")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fcoroutines")
endif ()

find_package(Drogon CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)
include_directories(${JSONCPP_INCLUDE_DIRS})

aux_source_directory(controllers CTL_SRC)

target_sources(${PROJECT_NAME}
    PRIVATE
    ${SRC_DIR}
    ${CTL_SRC}
)