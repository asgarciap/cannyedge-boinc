# CMakeList.txt: proyecto de CMake para cannyedge-boinc
cmake_minimum_required (VERSION 3.5)

set(contrib_lib_SRCS 
    contrib/bmp.c 
    contrib/CannyEdgeDetector.cpp
   )

if(NOT DEFINED BOINC_DIR)
    message(FATAL_ERROR "You must specify the BOINC_DIR using -DBOINC_DIR")
endif()

include_directories(.
    ${BOINC_DIR}
    ${BOINC_DIR}/api
    ${BOINC_DIR}/lib
)

link_directories(.
    ${BOINC_DIR}
    ${BOINC_DIR}/lib
    ${BOINC_DIR}api
)

add_library(contrib STATIC ${contrib_lib_SRCS})

add_executable (cannyedgeapp cannyedgeapp.cpp)

target_link_libraries(cannyedgeapp contrib ${BOINC_DIR}/lib/.libs/libboinc.a ${BOINC_DIR}/api/.libs/libboinc_api.a ${BOINC_DIR}/lib/.libs/libboinc.a pthread)
