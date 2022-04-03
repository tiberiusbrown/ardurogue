cmake_minimum_required(VERSION 3.15)

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0117 NEW)

project(ardurogue)

set(WINFLAG)
if(WIN32)
    set(WINFLAG WIN32)
endif()

add_executable(
    ${PROJECT_NAME} ${WINFLAG}
    .editorconfig
    main_win32.cpp
    draw.cpp
    entity.cpp
    game.hpp
    game.cpp
    generate.cpp
    light.cpp
    monsters.cpp
    sprintf.cpp
    status.cpp
    strings.cpp
    )

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_link_options(${PROJECT_NAME} PUBLIC
        /DEBUG
        /MAP
        /EMITPOGOPHASEINFO
        /MANIFEST:NO
        /DYNAMICBASE:NO
        /FIXED
        /INCREMENTAL:NO
        /OPT:REF
        /OPT:ICF=16
        /SAFESEH:NO
        $<$<NOT:$<CONFIG:Debug>>:/ENTRY:main>
        $<$<NOT:$<CONFIG:Debug>>:/NODEFAULTLIB>
    )
    target_compile_definitions(${PROJECT_NAME} PUBLIC -D_HAS_EXCEPTIONS=0)
    target_compile_options(${PROJECT_NAME} PUBLIC /GR- /GS-)
    #if(CMAKE_CXX_FLAGS MATCHES "/EHsc ")
    #    string(REPLACE "/EHsc" "/EHs-c-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    #endif()
    #if(CMAKE_CXX_FLAGS MATCHES "/EHsc$")
    #    string(REPLACE "/EHsc" "/EHs-c-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    #endif()
endif()

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
