cmake_minimum_required(VERSION 3.4.1)

set(THIS_PROJECT_DIR ${CMAKE_SOURCE_DIR})
set(TOP_DIR ${THIS_PROJECT_DIR}/../)

file(TO_CMAKE_PATH ${PATH_TO_SUPERPOWERED} PATH_TO_SUPERPOWERED)

include_directories(
        ${TOP_DIR}/android/src/main/cpp
        ${PATH_TO_SUPERPOWERED}
)

file(GLOB SUPERPOWERED_JNI_SRC
        ${TOP_DIR}/android/src/main/cpp/*.cpp
        ${TOP_DIR}/android/src/main/cpp/*.c
        ${TOP_DIR}/android/src/main/cpp/*.h
        ${TOP_DIR}/android/src/main/cpp/*.hpp
        ${TOP_DIR}/src_json_to_superpowered/*.cpp
        ${TOP_DIR}/src_json_to_superpowered/*.hpp
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