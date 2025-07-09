find_path(OpenHaptics_HD_INCLUDE_DIR
    NAMES HD/hd.h
    PATHS /usr/include
)

find_path(OpenHaptics_HL_INCLUDE_DIR
    NAMES HL/hl.h
    PATHS /usr/include
)

find_library(OpenHaptics_HD_LIBRARY
    NAMES HD
    PATHS /usr/lib
)

find_library(OpenHaptics_HL_LIBRARY
    NAMES HL
    PATHS /usr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenHaptics
    REQUIRED_VARS OpenHaptics_HD_INCLUDE_DIR OpenHaptics_HL_INCLUDE_DIR
                  OpenHaptics_HD_LIBRARY OpenHaptics_HL_LIBRARY
)

if(OpenHaptics_FOUND)
    set(OpenHaptics_INCLUDE_DIRS ${OpenHaptics_HD_INCLUDE_DIR} ${OpenHaptics_HL_INCLUDE_DIR})
    set(OpenHaptics_LIBRARIES ${OpenHaptics_HD_LIBRARY} ${OpenHaptics_HL_LIBRARY})

    if(NOT TARGET OpenHaptics::HD)
        add_library(OpenHaptics::HD UNKNOWN IMPORTED)
        set_target_properties(OpenHaptics::HD PROPERTIES
            IMPORTED_LOCATION "${OpenHaptics_HD_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OpenHaptics_HD_INCLUDE_DIR}"
        )
    endif()

    if(NOT TARGET OpenHaptics::HL)
        add_library(OpenHaptics::HL UNKNOWN IMPORTED)
        set_target_properties(OpenHaptics::HL PROPERTIES
            IMPORTED_LOCATION "${OpenHaptics_HL_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OpenHaptics_HL_INCLUDE_DIR}"
        )
    endif()
endif()
