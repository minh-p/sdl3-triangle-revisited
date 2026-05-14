find_program(GLSLC glslc REQUIRED)

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_BINARY_DIR}/assets/shaders)

set(SHADERS vertex.vert fragment.frag)

foreach(SHADER ${SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)

    add_custom_command(
        OUTPUT ${SHADER_BINARY_DIR}/${SHADER_NAME}.spv
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
        COMMAND ${GLSLC}
            ${SHADER_SOURCE_DIR}/${SHADER}
            -o ${SHADER_BINARY_DIR}/${SHADER_NAME}.spv
        DEPENDS ${SHADER_SOURCE_DIR}/${SHADER}
        COMMENT "Compiling shader ${SHADER}"
    )

    list(APPEND COMPILED_SHADERS ${SHADER_BINARY_DIR}/${SHADER_NAME}.spv)
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${COMPILED_SHADERS})
add_executable(triangle)
add_dependencies(triangle compile_shaders)

set_target_properties(triangle PROPERTIES OUTPUT_NAME "triangle")

# Create an option to switch between a system sdl library and a vendored SDL library
option(MYGAME_VENDORED "Use vendored libraries" OFF)

if(MYGAME_VENDORED)
    # This assumes you have added SDL as a submodule in vendored/SDL
    add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
else()
    # 1. Look for a SDL3 package,
    # 2. look for the SDL3-shared component, and
    # 3. fail if the shared component cannot be found.
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()

target_link_libraries(
	triangle
    PRIVATE
        SDL3::SDL3
)

target_sources(
	triangle
	PRIVATE
		main.cpp
)
