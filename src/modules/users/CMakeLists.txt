include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB KCM_USERS_GLOB
     kcm_users.desktop)

set(KCM_USERS_SRCS
    ${KCM_USERS_GLOB}
    AccountTypeDialog.cpp
    AddUserDialog.cpp
    ChangePasswordDialog.cpp
    FlatButton.cpp
    global.cpp
    ListWidget.cpp
    PreviewFileDialog.cpp
    UsersModule.cpp)

set(KCM_USERS_UI
    ui/AccountTypeDialog.ui
    ui/AddUserDialog.ui
    ui/ChangePasswordDialog.ui
    ui/PageUsers.ui)
qt5_wrap_ui(KCM_USERS_SRCS ${KCM_USERS_UI})

set(KCM_USERS_RESOURCES
    users.qrc)
QT5_ADD_RESOURCES (KCM_USERS_SRCS ${KCM_USERS_RESOURCES})

cmake_policy(SET CMP0063 NEW)

add_library(kcm_users MODULE ${KCM_USERS_SRCS})
target_compile_definitions(kcm_users PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(kcm_users
    Qt5::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons)

install(TARGETS kcm_users DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_users.desktop DESTINATION ${SERVICES_INSTALL_DIR})

kauth_install_actions(org.kaosx.kcm.users kcm_users.actions)

add_executable(kcm_users_authhelper UsersAuthHelper.cpp)
target_link_libraries(kcm_users_authhelper KF5::Auth KF5::ConfigCore)
install(TARGETS kcm_users_authhelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(kcm_users_authhelper org.kaosx.kcm.users root)
