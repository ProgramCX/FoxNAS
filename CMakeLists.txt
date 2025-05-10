cmake_minimum_required(VERSION 3.19)
project(FoxNAS LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network LinguistTools)

qt_standard_project_setup()

qt_add_executable(FoxNAS
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/LoginDialog.cpp
    src/LoginDialog.h
    src/LoginDialog.ui
    src/main.qrc
    src/util/IniSettings.hpp
)

qt_add_translations(
    TARGETS FoxNAS
    TS_FILES FoxNAS_zh_CN.ts
)

target_link_libraries(FoxNAS
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
)

target_include_directories(FoxNAS
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/util
)

include(GNUInstallDirs)

install(TARGETS FoxNAS
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET FoxNAS
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
