cmake_minimum_required(VERSION 3.29)
project(EmbeddedSoftwareLabor)

set(CMAKE_CXX_STANDARD 14)

include_directories(cdma_decoder)

add_executable(cdma_decoder
        cdma_decoder/cdma_decoder.cpp
)

target_link_libraries(cdma_decoder)