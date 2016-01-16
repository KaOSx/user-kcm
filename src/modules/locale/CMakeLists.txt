include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB KCM_LOCALE_GLOB
     kcm_locale.desktop)

set(KCM_LOCALE_SRCS
    ${KCM_LOCALE_GLOB}
    EnabledLocalesModel.cpp
    LanguageListViewDelegate.cpp
    LocaleModule.cpp
    SelectLocalesDialog.cpp
    SupportedLocalesItem.cpp
    SupportedLocalesModel.cpp)

set(KCM_LOCALE_UI
    ui/LocaleModule.ui
    ui/SelectLocalesDialog.ui)
qt5_wrap_ui(KCM_LOCALE_SRCS ${KCM_LOCALE_UI})

cmake_policy(SET CMP0063 NEW)

add_library(kcm_locale MODULE ${KCM_LOCALE_SRCS})
target_compile_definitions(kcm_locale PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(kcm_locale
    kcm
    Qt5::DBus
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::ItemModels
    icudata
    icuuc)

install(TARGETS kcm_locale DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_locale.desktop DESTINATION ${SERVICES_INSTALL_DIR})

kauth_install_actions(org.kaosx.kcmm.locale kcm_locale.actions)

add_executable(kcm_locale_authhelper LocaleAuthHelper.cpp)
target_link_libraries(kcm_locale_authhelper Qt5::DBus KF5::Auth KF5::ConfigCore)
install(TARGETS kcm_locale_authhelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(kcm_locale_authhelper org.kaosx.kcm.locale root)
