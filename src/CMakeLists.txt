cmake_minimum_required(VERSION 3.27.0)

set(TARGET_LIST PARENT_SCOPE)

#   ===== BUILD LIBSMHQ.SO =====

set(TARGET shmq)
add_library(
    ${TARGET} SHARED
    ${CMAKE_SOURCE_DIR}/src/queue.cpp
    ${CMAKE_SOURCE_DIR}/src/shmq.cpp
)
target_include_directories(
    ${TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${THIRD_PARTY_FOLDER}/include
)
target_link_libraries(
    ${TARGET} PRIVATE
    ${THIRD_PARTY_FOLDER}/lib/libyaml-cpp.a
)

list(APPEND TARGET_LIST ${TARGET})
set(TARGET_LIST "${TARGET_LIST}" PARENT_SCOPE)
