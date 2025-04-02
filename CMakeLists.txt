cmake_minimum_required(VERSION 3.29)
project(EmbeddedSoftwareLabor)

set(CMAKE_CXX_STANDARD 14)

include_directories(cdma_decoder)

add_executable(CDMA_decoder
        cdma_decoder/cdma_decoder.cpp
        cdma_decoder/cdma_decoder.h
        )

include_directories(cdma_decoder)

add_executable(CDMA_decoder_test
        cdma_decoder/cdma_decoder_test.cpp
        cdma_decoder/cdma_decoder.h
)