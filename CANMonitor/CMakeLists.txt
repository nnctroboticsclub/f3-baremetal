cmake_minimum_required(VERSION 3.25)
cmake_policy(VERSION 3.25)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ROBO_TOOLCHAIN_FILE ClangArmToolchain)

project(F3CANMonitor)
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

include(TargetTransformer)

find_package(Nano REQUIRED)
find_package(F3Baremetal REQUIRED)

add_executable(CANMonitor main.cpp)
target_link_libraries(CANMonitor PUBLIC F3Baremetal::f3-baremetal Nano::Nano)
target_compile_options(CANMonitor PUBLIC -fno-threadsafe-statics)
target_compile_options(CANMonitor PUBLIC -fuse-cxa-atexit)
target_link_options(CANMonitor PUBLIC -specs=nano.specs -specs=nosys.specs)
target_link_options(CANMonitor PUBLIC -Wl,-T,${CMAKE_CURRENT_SOURCE_DIR}/CANMonitor.ld)

embedded_transform_target(CANMonitor)

install(TARGETS CANMonitor DESTINATION bin)