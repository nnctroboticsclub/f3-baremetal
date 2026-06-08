cmake_minimum_required(VERSION 3.25)
cmake_policy(VERSION 3.25)

set(ROBO_TOOLCHAIN_FILE $ENV{ROBOPJ_TOOLCHAIN})
if (NOT ROBO_TOOLCHAIN_FILE)
  set(ROBO_TOOLCHAIN_FILE ClangArmToolchain)
endif()

if ($ENV{F3BARE_USE_STUB_BOOTLOADER} MATCHES "1|ON|TRUE|YES")
    set(USE_STUB_BOOTLOADER TRUE)
else()
    set(USE_STUB_BOOTLOADER FALSE)
endif()

if ($ENV{F3BARE_EMULATION} MATCHES "1|ON|TRUE|YES")
    set(F3BARE_EMULATION TRUE)
else()
    set(F3BARE_EMULATION FALSE)
endif()

if ($ENV{F3BARE_CAN_MONITOR} MATCHES "1|ON|TRUE|YES")
    set(F3BARE_CAN_MONITOR TRUE)
else()
    set(F3BARE_CAN_MONITOR FALSE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ROBO_TOOLCHAIN_FILE ${ROBO_TOOLCHAIN_FILE})

project(f3-baremetal VERSION 1.0.0)
enable_language(C CXX ASM)

message(STATUS "Using toolchain file: ${ROBO_TOOLCHAIN_FILE}")
message(STATUS "F3BARE_USE_STUB_BOOTLOADER: ${USE_STUB_BOOTLOADER}")
message(STATUS "F3BARE_EMULATION: ${F3BARE_EMULATION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Cortex M4
# if toolchain name contains "arm" add the options (case insensitive)
if (ROBO_TOOLCHAIN_FILE MATCHES "ClangArmToolchain")
  add_compile_options(-mfloat-abi=softfp -mfpu=fpv4-sp-d16 -mthumb -mcpu=cortex-m4)
  add_link_options(-mfloat-abi=softfp -mfpu=fpv4-sp-d16 -mthumb -mcpu=cortex-m4)
endif()

# Debug
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -gfull -fstandalone-debug -gdwarf-5 -gz")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gfull -fstandalone-debug -gdwarf-5 -gz")

# Size reduction
add_compile_options(-fno-rtti -fno-exceptions)
add_compile_options(-Os -ffunction-sections -fdata-sections)
add_link_options(-Wl,--gc-sections)

add_subdirectory(polyfill-srobo1-random)
add_subdirectory(f3-baremetal)

if (USE_STUB_BOOTLOADER)
  add_subdirectory(stub-bootloader)
  add_subdirectory(stub-bootloader-api)
endif()

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

if (F3BARE_CAN_MONITOR)
include(TargetTransformer)

  find_package(Nano REQUIRED)
  add_executable(CANMonitor CANMonitor/main.cpp)
  target_link_libraries(CANMonitor PUBLIC F3Baremetal::f3-baremetal Nano::Nano)
  target_compile_options(CANMonitor PUBLIC -fno-threadsafe-statics)
  target_compile_options(CANMonitor PUBLIC -fuse-cxa-atexit)
  target_link_options(CANMonitor PUBLIC -specs=nano.specs -specs=nosys.specs)
  target_link_options(CANMonitor PUBLIC -Wl,-T,${CMAKE_CURRENT_SOURCE_DIR}/CANMonitor/CANMonitor.ld)

  embedded_transform_target(CANMonitor)
endif()