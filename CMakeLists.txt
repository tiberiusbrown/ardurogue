cmake_minimum_required(VERSION 3.15)

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0117 NEW)

project(ardurogue)

set(PRE_CONFIGURE_FILE "git.hpp.in")
# write to source folder cause arduino ide :(
set(POST_CONFIGURE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/git.hpp")
include(git_watcher.cmake)
add_library(git INTERFACE ${POST_CONFIGURE_FILE})
target_include_directories(git INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_dependencies(git check_git)

set(WINFLAG)
if(WIN32)
    set(WINFLAG WIN32)
endif()

add_executable(
    ${PROJECT_NAME} ${WINFLAG}
    README.md
    .editorconfig
    game.hpp
    main_win32.cpp
    draw.cpp
    entity.cpp
    font.cpp
    game.cpp
    generate.cpp
    light.cpp
    menus.cpp
    monsters.cpp
    save.cpp
    sprintf.cpp
    stack.cpp
    status.cpp
    strings.cpp
    use.cpp
    )

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PUBLIC git)

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
