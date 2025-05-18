cmake_minimum_required(VERSION 3.19)
project(FoxNAS LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network LinguistTools)

qt_standard_project_setup()

qt_add_executable(FoxNAS
    WIN32 MACOSX_BUNDLE
    icon.rc
    src/main.cpp
    src/LoginDialog.cpp
    src/LoginDialog.h
    src/LoginDialog.ui
    src/main.qrc
    src/util/IniSettings.hpp
    src/Addnasdialog.h src/Addnasdialog.cpp src/Addnasdialog.ui
    src/DetailDialog.h src/DetailDialog.cpp src/DetailDialog.ui
    src/util/ApiRequest.h src/util/ApiRequest.cpp
    src/NASLoginDialog.h src/NASLoginDialog.cpp src/NASLoginDialog.ui
    src/util/ApiUrl.h
    src/LoadingDialog.h src/LoadingDialog.cpp src/LoadingDialog.ui
    src/util/ApiUrl.cpp
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


if(WIN32)
    # Windows 使用 .rc 文件设置图标
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")
    target_sources(FoxNAS PRIVATE ${APP_ICON_RESOURCE_WINDOWS})
elseif(APPLE)
    # macOS 设置 bundle 和图标
    set(MACOSX_BUNDLE_ICON_FILE appicon.icns)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/appicon.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(FoxNAS PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE appicon.icns
    )
    target_sources(FoxNAS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/resource/favicon/design/favicon.icns)
elseif(UNIX)
    # Linux 添加 desktop entry 的安装步骤（推荐用 CPack 或 install 脚本处理）
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resource/favicon/FoxNAS.png DESTINATION share/icons/hicolor/256x256/apps)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icon.desktop DESTINATION share/applications)
endif()
