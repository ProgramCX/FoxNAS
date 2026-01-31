cmake_minimum_required(VERSION 3.19)
project(FoxNAS LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network WebSockets LinguistTools Charts Svg Concurrent Multimedia MultimediaWidgets)

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
    src/RegisterDialog.h
    src/RegisterDialog.cpp
    src/RegisterDialog.ui

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
    src/MediaPlayer/VideoPlayer.h src/MediaPlayer/VideoPlayer.cpp src/MediaPlayer/VideoPlayer.ui
    src/MediaPlayer/VideoWidget.h

    # 委托
    src/delegate/ButtonDelegate.h
    src/delegate/ButtonDelegate.cpp
    src/delegate/MultiButtonDelegate.h
    src/delegate/MultiButtonDelegate.cpp
    src/delegate/ComboBoxDelegate.h


    src/MainTabWidgets/DDNSManagement/DDNSSecretForm.h
    src/MainTabWidgets/DDNSManagement/DDNSSecretForm.cpp
    src/MainTabWidgets/DDNSManagement/DDNSSecretForm.ui
    src/MainTabWidgets/DDNSManagement/DDNSSecrectSelectDialog.h
    src/MainTabWidgets/DDNSManagement/DDNSSecrectSelectDialog.cpp src/MainTabWidgets/DDNSManagement/DDNSSecrectSelectDialog.ui
    src/MainTabWidgets/UserManagement/PasswordModifyDialog.h src/MainTabWidgets/UserManagement/PasswordModifyDialog.cpp src/MainTabWidgets/UserManagement/PasswordModifyDialog.ui
    src/MainTabWidgets/UserManagement/UserPermissionDialog.h src/MainTabWidgets/UserManagement/UserPermissionDialog.cpp src/MainTabWidgets/UserManagement/UserPermissionDialog.ui
    src/MainTabWidgets/UserManagement/FilePermissionManagmentDialog.h src/MainTabWidgets/UserManagement/FilePermissionManagmentDialog.cpp src/MainTabWidgets/UserManagement/FilePermissionManagmentDialog.ui
    src/MainTabWidgets/UserManagement/UserDirPermissionDialog.h src/MainTabWidgets/UserManagement/UserDirPermissionDialog.cpp src/MainTabWidgets/UserManagement/UserDirPermissionDialog.ui
    src/MainTabWidgets/UserManagement/DIrPermissionEditDialog.h src/MainTabWidgets/UserManagement/DIrPermissionEditDialog.cpp src/MainTabWidgets/UserManagement/DIrPermissionEditDialog.ui
    src/MainTabWidgets/UserManagement/DirSelectDialog.h src/MainTabWidgets/UserManagement/DirSelectDialog.cpp src/MainTabWidgets/UserManagement/DirSelectDialog.ui
    src/util/TaskExecutor.h
    src/MainTabWidgets/AboutDialog.h src/MainTabWidgets/AboutDialog.cpp src/MainTabWidgets/AboutDialog.ui
    src/MainTabWidgets/Settings/SelfUserSettingsForm.h src/MainTabWidgets/Settings/SelfUserSettingsForm.cpp src/MainTabWidgets/Settings/SelfUserSettingsForm.ui
    src/MainTabWidgets/Settings/GeneralSettingForm.h src/MainTabWidgets/Settings/GeneralSettingForm.cpp src/MainTabWidgets/Settings/GeneralSettingForm.ui
    src/MainTabWidgets/Settings/PasswordChangeDialog.h src/MainTabWidgets/Settings/PasswordChangeDialog.cpp src/MainTabWidgets/Settings/PasswordChangeDialog.ui

    # 语言管理器
    src/util/LanguageManager.h
    src/util/LanguageManager.cpp
)


# 多语言翻译文件
set(TS_FILES
    translations/FoxNAS_zh_CN.ts
    translations/FoxNAS_zh_TW.ts
    translations/FoxNAS_ja.ts
    translations/FoxNAS_fr.ts
    translations/FoxNAS_de.ts
    translations/FoxNAS_pt.ts
    translations/FoxNAS_ru.ts
    translations/FoxNAS_ar.ts
    translations/FoxNAS_en.ts
)

qt_add_translations(
    TARGETS FoxNAS
    TS_FILES ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE QM_FILES
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
        Qt::Multimedia
        Qt::MultimediaWidgets
)

target_include_directories(FoxNAS
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/util
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MainTabWidgets
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MainTabWidgets/FileManagement
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MainTabWidgets/MediaPlayer
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MainTabWidgets/DDNSManagement
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MainTabWidgets/UserManagement
        ${CMAKE_CURRENT_SOURCE_DIR}/src/delegate
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MainTabWidgets/Settings
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
