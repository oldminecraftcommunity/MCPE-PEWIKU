cmake_minimum_required(VERSION 3.16)

project(pewiku_linux LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(HANDHELD_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")
set(SRC_DIR "${HANDHELD_DIR}/src")

file(GLOB_RECURSE PEWIKU_SOURCES CONFIGURE_DEPENDS
    "${SRC_DIR}/*.c"
    "${SRC_DIR}/*.cpp"
)

set(PEWIKU_EXCLUDED_SOURCES
    "${SRC_DIR}/AppPlatform_win32.cpp"
    "${SRC_DIR}/main_android.cpp"
    "${SRC_DIR}/main_android_java.cpp"
    "${SRC_DIR}/main_dedicated.cpp"
    "${SRC_DIR}/main_rpi.cpp"
    "${SRC_DIR}/platform/audio/SoundSystemAL.cpp"
    "${SRC_DIR}/platform/audio/SoundSystemSL.cpp"
    "${SRC_DIR}/world/level/tile/SandTile.cpp"
    "${SRC_DIR}/raknet/RakNetSocket2_Windows_Linux_360.cpp"
    "${SRC_DIR}/raknet/RakNetSocket2_360_720.cpp"
    "${SRC_DIR}/raknet/RakNetSocket2_PS3_PS4.cpp"
    "${SRC_DIR}/raknet/RakNetSocket2_PS4.cpp"
    "${SRC_DIR}/raknet/RakNetSocket2_Vita.cpp"
    "${SRC_DIR}/raknet/RakNetSocket2_WindowsStore8.cpp"
    "${SRC_DIR}/raknet/VitaIncludes.cpp"
    "${SRC_DIR}/raknet/PS4Includes.cpp"
)
list(REMOVE_ITEM PEWIKU_SOURCES ${PEWIKU_EXCLUDED_SOURCES})

add_executable(pewiku ${PEWIKU_SOURCES})

target_include_directories(pewiku PRIVATE
    "${SRC_DIR}"
    "${SRC_DIR}/raknet"
)

target_compile_definitions(pewiku PRIVATE
    LINUX
    POSIX
    OPENGL_ES
)

find_package(Threads REQUIRED)
find_package(X11 REQUIRED)
find_package(PNG REQUIRED)
find_library(EGL_LIBRARY NAMES EGL REQUIRED)
find_library(GLES1_LIBRARY NAMES GLESv1_CM REQUIRED)

target_link_libraries(pewiku PRIVATE
    Threads::Threads
    X11::X11
    PNG::PNG
    ${EGL_LIBRARY}
    ${GLES1_LIBRARY}
    dl
    m
)

add_custom_command(TARGET pewiku POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "$<TARGET_FILE_DIR:pewiku>/data"
    COMMAND "${CMAKE_COMMAND}" -E create_symlink "${HANDHELD_DIR}/data" "$<TARGET_FILE_DIR:pewiku>/data"
)
