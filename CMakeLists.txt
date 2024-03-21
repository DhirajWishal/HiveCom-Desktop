
# Set the minimum required CMake version.
cmake_minimum_required(VERSION 3.22.3)

# Set the main project information.
project(
	HiveCom-Desktop

	VERSION 0.0.1
	DESCRIPTION "Desktop application demonstration of HiveCom"
)

# Set global flags.
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the required packages.
find_package(OpenSSL REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

# Set the HiveCom include directory.
set(HIVECOM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/HiveCom/Include)

# Add the subdirectories.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Source)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/HiveCom)

# Setup the compile_flags.txt file to assist clangd.
file(
    WRITE ${CMAKE_CURRENT_SOURCE_DIR}/compile_flags.txt

    "-std=c++${CMAKE_CXX_STANDARD}\n"
    "-I${LIBOQS_INCLUDE_DIR}\n"
    "-I${OPENSSL_INCLUDE_DIR}\n"
    "-I${SPDLOG_INCLUDE_DIR}\n"
    "-I${MAIN_INCLUDE_DIR}\n"
)

# Set the desktop app as the start-up project.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Desktop)
