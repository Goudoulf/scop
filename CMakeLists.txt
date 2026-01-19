cmake_minimum_required(VERSION 3.10.0)
project(
  Renderer
  VERSION 0.1.0
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${CMAKE_BINARY_DIR}/compile_commands.json
                ${CMAKE_SOURCE_DIR}/compile_commands.json
    )
endif()

add_subdirectory(external/Vulkan-Headers)


# to reduce compile by not bulding unescessary stuff
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
target_link_libraries(
  ${PROJECT_NAME} PRIVATE Vulkan::Headers glfw )
# target_compile_definitions(
#   ${PROJECT_NAME} PRIVATE VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL=0
#                           SDL_MAIN_USE_CALLBACKS VK_NO_PROTOTYPES)
