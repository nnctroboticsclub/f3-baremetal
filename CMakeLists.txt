cmake_minimum_required(VERSION 3.25)
cmake_policy(VERSION 3.25)

# if $env{ROBOPJ_TOOLCHAIN} is not set, use ClangArmToolchain
# if set, use the toolchain specified by $env{ROBOPJ_TOOLCHAIN}
set(ROBO_TOOLCHAIN_FILE $ENV{ROBOPJ_TOOLCHAIN})
if (NOT ROBO_TOOLCHAIN_FILE)
  set(ROBO_TOOLCHAIN_FILE ClangArmToolchain)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ROBO_TOOLCHAIN_FILE ${ROBO_TOOLCHAIN_FILE})

project(f3-baremetal VERSION 1.0.0)
enable_language(C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Cortex M4
add_compile_options(-mfloat-abi=softfp -mfpu=fpv4-sp-d16 -mthumb -mcpu=cortex-m4)
add_link_options(-mfloat-abi=softfp -mfpu=fpv4-sp-d16 -mthumb -mcpu=cortex-m4)

# Debug
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -gfull -fstandalone-debug -gdwarf-5 -gz")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gfull -fstandalone-debug -gdwarf-5 -gz")

# Size reduction
add_compile_options(-fno-rtti -fno-exceptions)
add_compile_options(-Os -ffunction-sections -fdata-sections)
add_link_options(-Wl,--gc-sections)

add_subdirectory(polyfill-srobo1-random)
add_subdirectory(f3-baremetal)
add_subdirectory(stub-bootloader)
add_subdirectory(stub-bootloader-api)

# Install export
install(EXPORT F3BaremetalTargets
  FILE F3BaremetalTargets.cmake
  NAMESPACE F3Baremetal::
  DESTINATION lib/cmake/F3Baremetal
)

# Create config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/F3BaremetalConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/F3BaremetalConfig.cmake
  INSTALL_DESTINATION lib/cmake/F3Baremetal
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/F3BaremetalConfig.cmake
  DESTINATION lib/cmake/F3Baremetal
)
