# Set the project name.
project(
	HiveCom-Desktop
    VERSION 0.1
)

# Set the project sources.
set(
	SOURCES

	"Main.cpp"
	"CertificateProvider.cpp"
	"CertificateProvider.hpp"
	"DesktopDataLink.cpp"
	"DesktopDataLink.hpp"
	"MainWindow.cpp"
	"MainWindow.hpp"

    "ReflectionDataLink.hpp"
    "ReflectionDataLink.cpp"

	"UI/MainWindow.ui"

    "Resources/Resources.qrc"

    "Resources/SignKeys/public.DesktopApp-Sign.bin"
    "Resources/SignKeys/private.DesktopApp-Sign.bin"
)

# Add the source group.
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(${PROJECT_NAME} SHARED
            ${SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(${PROJECT_NAME}
            ${SOURCES}
        )
    endif()
endif()

target_link_libraries(
    ${PROJECT_NAME} 
    PRIVATE 
    
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::HttpServer

    HiveCom
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()

# Setup the include directories.
target_include_directories(
    ${PROJECT_NAME}

    PRIVATE "${HIVECOM_INCLUDE_DIR}"
)

# Run the deploy script.
if(WIN32)
    add_custom_command(
        TARGET ${PROJECT_NAME} 
        POST_BUILD

        COMMAND ${QT_DIR}/../../../bin/windeployqt.exe $<TARGET_FILE:${PROJECT_NAME}>
        COMMENT "Run the deployment script for the output file."
    )
endif()

# Add all the additional dependencies.
add_dependencies(
    ${PROJECT_NAME} 

    HiveCom-CLI
)

# Copy the dependencies.
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD 

    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:HiveCom-CLI> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "Copy the required target files to the output directory."
)
