# Assignment2
add_executable (EDAN35_Assignment2)
target_sources (
	EDAN35_Assignment2
	PRIVATE
		[[assignment2.hpp]]
		[[assignment2.cpp]]
)
target_link_libraries (EDAN35_Assignment2 PRIVATE assignment_setup)
install (TARGETS EDAN35_Assignment2 DESTINATION bin)
copy_dlls (EDAN35_Assignment2 "${CMAKE_CURRENT_BINARY_DIR}")

# Snow Project
add_executable (EDAN35_SnowProject)
target_sources (
	EDAN35_SnowProject
	PRIVATE
		[[snowproject.hpp]]
		[[snowproject.cpp]]
)
target_link_libraries (
	EDAN35_SnowProject
	PRIVATE assignment_setup parametric_shapes
)
copy_dlls (EDAN35_SnowProject "${CMAKE_CURRENT_BINARY_DIR}")

install (
	TARGETS
		EDAN35_Assignment2
		EDAN35_SnowProject
	DESTINATION [[bin]]
)
