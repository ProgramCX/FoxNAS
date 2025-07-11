cmake_minimum_required(VERSION 3.19)
project(FoxNAS LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network WebSockets LinguistTools Charts Svg Concurrent)

qt_standard_project_setup()

set(RESOURCES src/main.qrc)

qt_add_resources(RESOURCES_CPP ${RESOURCES})

qt_add_executable(FoxNAS
    WIN32 MACOSX_BUNDLE

    # 图标与资源
    icon.rc
    src/main.qrc
    ${RESOURCES_CPP}

    # 主入口
    src/main.cpp

    # 🔹 主窗口
    src/MainWindow.h
    src/MainWindow.cpp
    src/MainWindow.ui

    # 🔹 登录与引导
    src/LoginDialog.h
    src/LoginDialog.cpp
    src/LoginDialog.ui
    src/NASLoginDialog.h
    src/NASLoginDialog.cpp
    src/NASLoginDialog.ui
    src/LoadingDialog.h
    src/LoadingDialog.cpp
    src/LoadingDialog.ui
    src/Addnasdialog.h
    src/Addnasdialog.cpp
    src/Addnasdialog.ui
    src/DetailDialog.h
    src/DetailDialog.cpp
    src/DetailDialog.ui

    # 🔹 主界面 - Tab 页面模块
    src/MainTabWidgets/OverViewForm.h
    src/MainTabWidgets/OverViewForm.cpp
    src/MainTabWidgets/OverViewForm.ui

    src/MainTabWidgets/OverviewDiskItem.h
    src/MainTabWidgets/OverviewDiskItem.cpp
    src/MainTabWidgets/OverviewDiskItem.ui

    src/MainTabWidgets/FileManagementForm.h
    src/MainTabWidgets/FileManagementForm.cpp
    src/MainTabWidgets/FileManagementForm.ui

    src/MainTabWidgets/UserManagementForm.h
    src/MainTabWidgets/UserManagementForm.cpp
    src/MainTabWidgets/UserManagementForm.ui

    src/MainTabWidgets/SettingsForm.h
    src/MainTabWidgets/SettingsForm.cpp
    src/MainTabWidgets/SettingsForm.ui

    src/MainTabWidgets/InfoForm.h
    src/MainTabWidgets/InfoForm.cpp
    src/MainTabWidgets/InfoForm.ui

    src/MainTabWidgets/DDNSForm.h
    src/MainTabWidgets/DDNSForm.cpp
    src/MainTabWidgets/DDNSForm.ui

    # 🔹 Tab 子模块：文件管理
    src/MainTabWidgets/FileManagement/FileManagementTabForm.h
    src/MainTabWidgets/FileManagement/FileManagementTabForm.cpp
    src/MainTabWidgets/FileManagement/FileManagementTabForm.ui

    src/MainTabWidgets/FileManagement/DirsAuthedSelectDialog.h
    src/MainTabWidgets/FileManagement/DirsAuthedSelectDialog.cpp
    src/MainTabWidgets/FileManagement/DirsAuthedSelectDialog.ui

    src/MainTabWidgets/FileManagement/FileOperationFailedDialog.h
    src/MainTabWidgets/FileManagement/FileOperationFailedDialog.cpp
    src/MainTabWidgets/FileManagement/FileOperationFailedDialog.ui

    src/MainTabWidgets/FileManagement/FileTreeView.h
    src/MainTabWidgets/FileManagement/FileTreeView.cpp

    src/MainTabWidgets/FileManagement/FilePropertyDialog.h
    src/MainTabWidgets/FileManagement/FilePropertyDialog.cpp
    src/MainTabWidgets/FileManagement/FilePropertyDialog.ui

    # 🔹 模型与节点（文件系统）
    src/model/FileSystemRemoteModel.h
    src/model/FileSystemRemoteModel.cpp
    src/model/RemoteFileSystemNode.h

    # 🔹 工具类 util
    src/util/IniSettings.hpp
    src/util/ApiUrl.h
    src/util/ApiUrl.cpp
    src/util/ApiRequest.h
    src/util/ApiRequest.cpp
    src/util/MemStore.h
    src/util/MemStore.cpp
    src/util/ClipboardManager.h
    src/util/ClipboardManager.cpp
    src/util/FileTranferListItem.h
    src/util/FileTranferListItem.cpp
    src/util/FileTranferListItem.ui
    src/util/BytesConvertorUtil.h
    src/util/BytesConvertorUtil.cpp
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
        Qt::WebSockets
        Qt::Charts
        Qt::Svg
        Qt::Concurrent
)

target_include_directories(FoxNAS
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/util
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MainTabWidgets
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MainTabWidgets/FileManagement
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
    set_source_files_properties(${APP_ICON_RESOURCE_WINDOWS} PROPERTIES LANGUAGE RC)

     install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/manifest/win32-require-admin-manifest.xml DESTINATION .)
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
