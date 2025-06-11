add_library(srobo1-random STATIC
    source/random.cpp
)

target_include_directories(srobo1-random PUBLIC
    include
)