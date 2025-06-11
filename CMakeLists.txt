add_subdirectory(polyfills)

# *******************
# F3-HAL Library
# *******************
if(${EMULATION_ENABLED})
    add_library(f3-hal OBJECT startup/dummy.c)
else()
    add_library(f3-hal OBJECT startup/startup_stm32f303x8.s)
endif()

target_include_directories(f3-hal PUBLIC include)
target_link_libraries(f3-hal PUBLIC stm32cube-cmsis syoch-robotics-common)


# *******************
# F3-Baremetal Library
# *******************
add_library(f3-baremetal-libs STATIC source/ram_vector.cpp)
target_link_libraries(f3-baremetal-libs PUBLIC f3-hal stm32cube)

if(${EMULATION_ENABLED})
    target_compile_definitions(f3-baremetal-libs PRIVATE
        -D__EMULATION__
    )
else()
    target_sources(f3-baremetal-libs PRIVATE
        source/startup/startup.cpp
    )
endif()