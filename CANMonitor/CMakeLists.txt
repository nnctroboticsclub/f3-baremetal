include(TargetTransformer)

find_package(Nano REQUIRED)

add_executable(CANMonitor main.cpp)
target_link_libraries(CANMonitor PUBLIC f3-baremetal Nano::Nano)
target_compile_options(CANMonitor PUBLIC -fno-threadsafe-statics)
target_compile_options(CANMonitor PUBLIC -fuse-cxa-atexit)
target_link_options(CANMonitor PUBLIC -specs=nano.specs -specs=nosys.specs)
target_link_options(CANMonitor PUBLIC -Wl,-T,${CMAKE_CURRENT_SOURCE_DIR}/CANMonitor.ld)

embedded_transform_target(CANMonitor)
