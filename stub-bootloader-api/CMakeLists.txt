find_package(CMSIS5 CONFIG)

add_library(stub-bootloader-api INTERFACE)
add_library(F3Baremetal::stub-bootloader-api ALIAS stub-bootloader-api)
target_link_libraries(stub-bootloader-api INTERFACE CMSIS5::CoreM)
target_include_directories(stub-bootloader-api INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/f3-baremetal/stub-bootloader-api>
)

install(TARGETS stub-bootloader-api
    EXPORT F3BaremetalTargets
    INCLUDES DESTINATION include/f3-baremetal/stub-bootloader-api
)
install(DIRECTORY include/
    DESTINATION include/f3-baremetal/stub-bootloader-api
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)