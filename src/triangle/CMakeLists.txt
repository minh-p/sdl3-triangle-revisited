add_executable(triangle)

set_target_properties(triangle PROPERTIES OUTPUT_NAME "triangle")

target_link_libraries(
	triangle
)

target_sources(
	triangle
	PRIVATE
		main.cpp
)
