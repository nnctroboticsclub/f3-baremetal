find_package(CMSIS5DeviceF3 REQUIRED)

add_library(f3-baremetal STATIC
    source/ram_vector.cpp
    source/startup.cpp
    source/startup_stm32f303x8.s
)
add_library(F3Baremetal::f3-baremetal ALIAS f3-baremetal)
target_compile_definitions(f3-baremetal PUBLIC
    STM32F303x8
)
target_link_libraries(f3-baremetal PUBLIC
    CMSIS5::Device::F3
)
target_include_directories(f3-baremetal PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/f3-baremetal>
)

install(TARGETS f3-baremetal
    EXPORT F3BaremetalTargets
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include/f3-baremetal
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)
