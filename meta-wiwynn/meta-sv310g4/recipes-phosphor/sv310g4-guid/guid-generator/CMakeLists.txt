cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -lstdc++fs \
    -Werror \
    -Wall \
    -Wextra \
    -Wnon-virtual-dtor \
    -Wold-style-cast \
    -Wcast-align \
    -Wunused \
    -Woverloaded-virtual \
    -Wpedantic \
    -Wmisleading-indentation \
    -Wduplicated-cond \
    -Wduplicated-branches \
    -Wlogical-op \
    -Wnull-dereference \
    -Wuseless-cast \
    -Wdouble-promotion \
    -Wformat=2 \
    -Wno-sign-compare \
    -Wno-reorder \
    -Wshadow \
    -Wconversion \
"
)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

project (guid-generator CXX)

set (LINK_LIBS stdc++fs sdbusplus -lobmcmisc -lobmci2c -lsystemd)

link_directories (${EXTERNAL_INSTALL_LOCATION}/lib)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable (guid-generator src/main.cpp)
target_link_libraries (guid-generator ${LINK_LIBS})

set (SERVICE_FILE_SRC_DIR ${PROJECT_SOURCE_DIR}/service)
set (
    SERVICE_FILES
    ${SERVICE_FILE_SRC_DIR}/xyz.openbmc_project.guid-generator.service
)
set (SERVICE_FILE_INSTALL_DIR /lib/systemd/system/)

install (TARGETS guid-generator DESTINATION bin)
install (FILES ${SERVICE_FILES} DESTINATION ${SERVICE_FILE_INSTALL_DIR})
