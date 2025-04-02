cmake_minimum_required(VERSION 3.29)
project(EmbeddedSoftwareLabor)

set(CMAKE_CXX_STANDARD 14)

include_directories(cdma_decoder)

add_executable(CDMA_Decoder
        cdma_decoder/cdma_decoder.cpp
        cdma_decoder/cdma_decoder.h
        cdma_decoder/cdma_decoder_test.cpp)

include_directories(Laufzeitoptimierung)

add_executable(Laufzeitoptimierung
    Laufzeitoptimierung/Laufzeitoptimierung.cpp
        Laufzeitoptimierung/Laufzeitoptimierung.h)