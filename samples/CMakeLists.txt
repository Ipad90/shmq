cmake_minimum_required(VERSION 3.27.0)

set(TARGET_LIST PARENT_SCOPE)

#   ===== reader =====

set(TARGET reader)
add_executable(
    ${TARGET}
    ${CMAKE_SOURCE_DIR}/samples/reader.cpp
)
target_include_directories(
    ${TARGET} PRIVATE
    ${THIRD_PARTY_FOLDER}/include
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(
    ${TARGET} PRIVATE
    ${THIRD_PARTY_FOLDER}/lib/libyaml-cpp.a
    shmq
)
list(APPEND TARGET_LIST ${TARGET})
set(TARGET_LIST "${TARGET_LIST}" PARENT_SCOPE)

#   ===== writer =====

set(TARGET writer)
add_executable(
    ${TARGET}
    ${CMAKE_SOURCE_DIR}/samples/writer.cpp
)
target_include_directories(
    ${TARGET} PRIVATE
    ${THIRD_PARTY_FOLDER}/include
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(
    ${TARGET} PRIVATE
    ${THIRD_PARTY_FOLDER}/lib/libyaml-cpp.a
    shmq
)
list(APPEND TARGET_LIST ${TARGET})
set(TARGET_LIST "${TARGET_LIST}" PARENT_SCOPE)

#   ===== reader_using_lib =====

set(TARGET reader_using_lib)
add_executable(
    ${TARGET}
    ${CMAKE_SOURCE_DIR}/samples/reader_lib.cpp
)
target_include_directories(
    ${TARGET} PRIVATE
    ${THIRD_PARTY_FOLDER}/include
    ${CRIMSONZ_FOLDER}/include
)
target_link_libraries(
    ${TARGET} PRIVATE
    ${THIRD_PARTY_FOLDER}/lib/libyaml-cpp.a
    shmq
)
list(APPEND TARGET_LIST ${TARGET})
set(TARGET_LIST "${TARGET_LIST}" PARENT_SCOPE)

#   ===== writer_using_lib =====

set(TARGET writer_using_lib)
add_executable(
    ${TARGET}
    ${CMAKE_SOURCE_DIR}/samples/writer_lib.cpp
)
target_include_directories(
    ${TARGET} PRIVATE
    ${THIRD_PARTY_FOLDER}/include
    ${CRIMSONZ_FOLDER}/include
)
target_link_libraries(
    ${TARGET} PRIVATE
    ${THIRD_PARTY_FOLDER}/lib/libyaml-cpp.a
    shmq
)
list(APPEND TARGET_LIST ${TARGET})
set(TARGET_LIST "${TARGET_LIST}" PARENT_SCOPE)