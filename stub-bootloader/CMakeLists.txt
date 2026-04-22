include(TargetTransformer)
find_package(CMSIS5DeviceF3 REQUIRED)

add_executable(stub-bootloader source/main.cpp)
target_compile_definitions(stub-bootloader PRIVATE -DSTM32F303x8=1)
target_link_libraries(stub-bootloader PRIVATE CMSIS5::Device::F3)
target_link_options(stub-bootloader PRIVATE "-Wl,-T,${CMAKE_CURRENT_LIST_DIR}/bootloader.ld")

install(TARGETS stub-bootloader
  RUNTIME DESTINATION bin
)
