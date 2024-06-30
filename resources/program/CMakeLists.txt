cmake_minimum_required(VERSION 3.5)
project(VeridictGame VERSION 1.0 LANGUAGES C CXX)

# Spécifier où les bibliothèques dynamiques et statiques seront construites (dans le dossier `lib`)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../assets/lib)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

# json
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.10.5 # Vous pouvez changer la version ici
)

FetchContent_MakeAvailable(json)

# raylib
find_package(raylib 5.0 QUIET)
if (NOT raylib_FOUND)
    FetchContent_Declare(raylib URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz)
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED)
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(raylib)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        # Set BUILD_SHARED_LIBS to ON to build raylib as a shared library
        set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
        add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
    endif()
endif()


# raygui
set(RAYGUI_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../extern/raygui)
set(RAYGUI_SRC ${RAYGUI_ROOT}/src)
set(RAYGUI_EXAMPLES ${RAYGUI_ROOT}/examples)

add_library(raygui INTERFACE)
file(GLOB sources ${RAYGUI_SRC}/*.h)
set(RAYGUI_HEADERS ${sources})
install(FILES
        ${RAYGUI_HEADERS} DESTINATION include
)
target_include_directories(raygui INTERFACE ${RAYGUI_SRC})


function(add_files_recursively result curdir)
    file(GLOB_RECURSE files LIST_DIRECTORIES false ${curdir}/*.cpp ${curdir}/*.c ${curdir}/*.hpp ${curdir}/*.h)
    set(${result} ${files} PARENT_SCOPE)
endfunction()


set(SOURCE_FILES "")
add_files_recursively(SOURCE_FILES ${PROJECT_SOURCE_DIR}/../assets/scripts)

# Ajoutez la bibliothèque partagée
include_directories(${PROJECT_SOURCE_DIR}/../assets/scripts/VerCore)
add_library(lib STATIC IMPORTED)
set_target_properties(lib PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libVeridictEngine.a)

if(SOURCE_FILES)
    add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
    target_link_libraries(${PROJECT_NAME} PUBLIC raylib raygui nlohmann_json::nlohmann_json dl lib)

    # Définir explicitement le langage de la cible
    set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
else()
    message(FATAL_ERROR "Aucun fichier source (.cpp, .hpp, .h) trouvé dans ${PROJECT_SOURCE_DIR}/../scripts")
endif()

# Inclure les dossiers d'en-têtes
target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# Installer la bibliothèque dans un dossier spécifique après la construction
install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/../assets/scripts
)
