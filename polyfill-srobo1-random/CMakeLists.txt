add_library(srobo1-random STATIC
    source/random.cpp
)
add_library(F3Baremetal::srobo1-random ALIAS srobo1-random)
target_include_directories(srobo1-random PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/f3-baremetal/srobo1-random>
)

install(TARGETS srobo1-random
    EXPORT F3BaremetalTargets
    INCLUDES DESTINATION include/f3-baremetal
)

install(DIRECTORY include/
    DESTINATION include/f3-baremetal/srobo1-random
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)

