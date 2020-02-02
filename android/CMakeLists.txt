cmake_minimum_required(VERSION 3.4.1)

set(THIS_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(TOP_DIR ${THIS_PROJECT_DIR}/../)

set(SOP_TOP_DIR ${TOP_DIR}/src_xplatform/sop)
include(${TOP_DIR}/src_xplatform/sop/sop_src/LibraryFiles.cmake)

file(TO_CMAKE_PATH ${PATH_TO_SUPERPOWERED} PATH_TO_SUPERPOWERED)

include_directories(
        ${TOP_DIR}/android/src/main/cpp
        ${PATH_TO_SUPERPOWERED}
        ${TOP_DIR}/src_xplatform/sop/ext/json/include/
        ${TOP_DIR}/src_xplatform/sop/sop_src
        ${TOP_DIR}/src_xplatform
        ${TOP_DIR}
)

file(GLOB OSAL_PLATFORM_SRC
        ${TASK_SCHED_SRC}
        ${OSAL_SRC}
        ${UTILS_SRC}
        ${PHONE_AL_SRC}
        ${MBEDTLS_BASICS_SRC}
        ${BUF_IO_SRC}
)

file(GLOB SUPERPOWERED_JNI_SRC
        ${TOP_DIR}/android/src/main/cpp/*.cpp
        ${TOP_DIR}/android/src/main/cpp/*.c
        ${TOP_DIR}/android/src/main/cpp/*.h
        ${TOP_DIR}/android/src/main/cpp/*.hpp
        ${OSAL_PLATFORM_SRC}


        ${TOP_DIR}/src_xplatform/json_commands/*.cpp
        ${TOP_DIR}/src_xplatform/json_commands/*.hpp
)


list(REMOVE_ITEM SUPERPOWERED_JNI_SRC src/main/cpp/Recorder.cpp)
list(REMOVE_ITEM SUPERPOWERED_JNI_SRC src/main/cpp/Audio.cpp)

add_library (
        Recorder
        SHARED
        src/main/cpp/Recorder.cpp
        ${PATH_TO_SUPERPOWERED}/AndroidIO/SuperpoweredAndroidAudioIO.cpp
)

add_library (
        Audio
        SHARED
        src/main/cpp/Audio.cpp
        ${PATH_TO_SUPERPOWERED}/AndroidIO/SuperpoweredAndroidAudioIO.cpp
)

add_library (
        SuperpoweredJni
        SHARED
        ${SUPERPOWERED_JNI_SRC}
        ${PATH_TO_SUPERPOWERED}/AndroidIO/SuperpoweredAndroidAudioIO.cpp
)

target_link_libraries (
        Recorder
        log
        android
        OpenSLES
        ${PATH_TO_SUPERPOWERED}/libSuperpoweredAndroid${ANDROID_ABI}.a
)

target_link_libraries (
        Audio
        log
        android
        OpenSLES
        ${PATH_TO_SUPERPOWERED}/libSuperpoweredAndroid${ANDROID_ABI}.a
)

target_link_libraries (
        SuperpoweredJni
        log
        android
        OpenSLES
        ${PATH_TO_SUPERPOWERED}/libSuperpoweredAndroid${ANDROID_ABI}.a
)