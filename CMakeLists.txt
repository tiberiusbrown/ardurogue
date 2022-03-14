cmake_minimum_required(VERSION 2.8.11)

project(ardurogue)

set(WINFLAG)
if(WIN32)
    set(WINFLAG WIN32)
endif()

add_executable(
    ${PROJECT_NAME} ${WINFLAG}
    main_win32.cpp
    draw.cpp
    game.hpp
    game.cpp
    generate.cpp
    light.cpp
    )

#set(FORCE_STATIC_VCRT ON  CACHE BOOL "" FORCE)
#set(SDL_SHARED        OFF CACHE BOOL "" FORCE)
#set(SDL_AUDIO         ON  CACHE BOOL "" FORCE)
#set(SDL_CPUINFO       ON  CACHE BOOL "" FORCE)
#if(MSVC)
#    set(SDL_DLOPEN    OFF CACHE BOOL "" FORCE)
#elseif(UNIX)
#    set(SDL_DLOPEN    ON CACHE BOOL "" FORCE)
#endif()
#set(SDL_EVENTS        ON  CACHE BOOL "" FORCE)
#set(SDL_FILE          OFF CACHE BOOL "" FORCE)
#set(SDL_FILESYSTEM    ON  CACHE BOOL "" FORCE)
#set(SDL_HAPTIC        ON  CACHE BOOL "" FORCE)
#set(SDL_JOYSTICK      ON  CACHE BOOL "" FORCE)
#set(SDL_LOADSO        ON  CACHE BOOL "" FORCE)
#set(SDL_POWER         ON  CACHE BOOL "" FORCE)
#set(SDL_RENDER        OFF CACHE BOOL "" FORCE)
#set(SDL_THREADS       ON  CACHE BOOL "" FORCE)
#set(SDL_TIMERS        ON  CACHE BOOL "" FORCE)
#set(SDL_VIDEO         ON  CACHE BOOL "" FORCE)
#set(SSE3              OFF CACHE BOOL "" FORCE)
#add_subdirectory(SDL)
#target_compile_definitions(SDL2main PUBLIC -DDECLSPEC=)
#target_compile_definitions(SDL2-static PUBLIC -DDECLSPEC=)
#target_compile_definitions(${PROJECT_NAME} PUBLIC -DHAVE_M_PI)
#
#target_link_libraries(${PROJECT_NAME} SDL2-static SDL2main)
#target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
#	$<TARGET_PROPERTY:SDL2-static,INCLUDE_DIRECTORIES>
#	)