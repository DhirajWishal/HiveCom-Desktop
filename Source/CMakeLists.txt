# Set the project name.
project(
	Desktop
)

# Set the project sources.
set(
	SOURCES

	"Main.cpp"
)

# Enable testing.
enable_testing()

# Setup the executable.
add_executable(
    Desktop

    ${SOURCES}
)

# Add the test.
add_test(NAME Tests  COMMAND Tests)

# Setup the link libraries.
target_link_libraries(
    Desktop 

    HiveCom
)

# Setup the include directories.
target_include_directories(
    Desktop

    PRIVATE "${HIVECOM_INCLUDE_DIR}"
)
