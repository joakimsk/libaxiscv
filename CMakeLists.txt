cmake_minimum_required (VERSION 2.8.9)

add_library(axis_cv_lib src/cv.cpp)

FIND_PACKAGE(CURL)
FIND_PACKAGE(BOOST)

target_include_directories(axis_cv_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CURL_LIBRARIES} ${BOOST_LIBRARIES})
set(CMAKE_CXX_FLAGS "-std=c++11")
