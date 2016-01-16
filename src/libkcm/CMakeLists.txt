set(KDSINGLEAPPLICATIONGUARD
    kdsingleapplicationguard/kdsingleapplicationguard.cpp
    kdsingleapplicationguard/kdsharedmemorylocker.cpp
    kdsingleapplicationguard/kdtoolsglobal.cpp
    kdsingleapplicationguard/kdlockedsharedmemorypointer.cpp)

set_source_files_properties(
    ${KDSINGLEAPPLICATIONGUARD}
    PROPERTIES
    COMPILE_FLAGS "-Wno-unused-function")

set(LIBMSM_SRCS
    ActionDialog.cpp
    LanguageCommon.cpp

    ${KDSINGLEAPPLICATIONGUARD})

cmake_policy(SET CMP0063 NEW)

add_library (kcm STATIC ${LIBMSM_SRCS})
target_link_libraries (kcm KF5::ConfigWidgets
    KF5::CoreAddons)
target_include_directories (kcm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
