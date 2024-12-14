# Snow Project
add_executable (EDAN35_SnowProject)
target_sources (
	EDAN35_SnowProject
	PRIVATE
		[[snowproject.hpp]]
		[[snowproject.cpp]]
 "particleSystem.cpp")
target_link_libraries (
	EDAN35_SnowProject
	PRIVATE assignment_setup parametric_shapes
)
copy_dlls (EDAN35_SnowProject "${CMAKE_CURRENT_BINARY_DIR}")

install (
	TARGETS
		EDAN35_SnowProject
	DESTINATION [[bin]]
)
