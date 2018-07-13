add_library(tasks OBJECT
    src/BackgroundSubstraction.cpp
    src/FlatfieldCorrection.cpp
    src/RoiCounter.cpp
    src/Binning.cpp
    src/Flip.cpp
    src/SoftRoi.cpp
    src/Bpm.cpp
    src/Mask.cpp
    src/Roi2Spectrum.cpp
    src/Rotation.cpp
    src/PeakFinder.cpp)

target_include_directories(tasks PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include/processlib>")

target_link_libraries(tasks core)
