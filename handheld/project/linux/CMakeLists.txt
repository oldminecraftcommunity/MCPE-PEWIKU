cmake_minimum_required(VERSION 3.16)

set(HANDHELD_DIR "${PROJECT_SOURCE_DIR}/handheld")
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

message(${CMAKE_LIBRARY_ARCHITECTURE})
add_executable(pewiku_linux ${PEWIKU_SOURCES})

target_include_directories(pewiku_linux PRIVATE
    "${SRC_DIR}"
    "${SRC_DIR}/raknet"
)

target_compile_definitions(pewiku_linux PRIVATE
    LINUX
    POSIX
    OPENGL_ES
)

find_package(Threads REQUIRED)
find_package(X11 REQUIRED)
find_package(PNG REQUIRED)
# find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_library(EGL_LIBRARY NAMES EGL REQUIRED)
find_library(GLES_LIBRARY NAMES GLESv1_CM GLESv2 GLES REQUIRED)

target_link_libraries(pewiku_linux PRIVATE
    Threads::Threads
    X11::X11
    PNG::PNG
#    GLEW::GLEW
    SDL2::SDL2
    ${EGL_LIBRARY}
    ${GLES1_LIBRARY}
    dl
    m
)

add_custom_command(TARGET pewiku_linux POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "$<TARGET_FILE_DIR:pewiku_linux>/data"
    COMMAND "${CMAKE_COMMAND}" -E create_symlink "${HANDHELD_DIR}/data" "$<TARGET_FILE_DIR:pewiku_linux>/data"
)
