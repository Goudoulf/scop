cmake_minimum_required(VERSION 3.10.0)


#####################
# SHADER COMPILATION
#####################

find_program(SLANGC slangc REQUIRED)

set(SHADER_SOURCES_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(SHADER_SOURCES
	${SHADER_SOURCES_DIR}/shader.slang
)

set(SHADER_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/shaders/)
set(SHADER_SPV_FILES)

foreach(SHADER ${SHADER_SOURCES})
	get_filename_component(FILE_NAME ${SHADER} NAME)
	set(SPV ${SHADER_OUTPUT_DIR}/${FILE_NAME}.spv)

	list(APPEND SHADER_SPV_FILES ${SPV})

	add_custom_command(
		OUTPUT ${SPV}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_DIR}
		COMMAND slangc ${SHADER} -lang slang -target spirv -profile glsl_450 -emit-spirv-directly -fvk-use-gl-layout -o ${SPV}
		DEPENDS ${SHADER}
		VERBATIM
	)
endforeach()

add_custom_target(shaders DEPENDS ${SHADER_SPV_FILES})


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
add_subdirectory(external/glm)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_SOURCE_DIR}/external/stb)


# to reduce compile by not bulding unescessary stuff
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

add_dependencies(${PROJECT_NAME} shaders)
add_library(tinyobjloader INTERFACE)
target_include_directories(tinyobjloader INTERFACE ${CMAKE_SOURCE_DIR}/external/tinyobjloader)

target_link_libraries(
  ${PROJECT_NAME} PRIVATE Vulkan::Headers glfw glm::glm stb tinyobjloader)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	SHADERS_DIR="${SHADER_OUTPUT_DIR}"
)

