find_package(OpenSSL REQUIRED)

# Compute the output directory the same way ns-3 does
get_filename_component(scratch_absolute_directory ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)
string(REPLACE "${PROJECT_SOURCE_DIR}" "${CMAKE_OUTPUT_DIRECTORY}"
               scratch_directory
               "${scratch_absolute_directory}")

build_exec(
    EXECNAME kryon-simulator
    EXECNAME_PREFIX scratch_kryon_

    SOURCE_FILES
        kryon-simulator.cc

    LIBRARIES_TO_LINK
        ${ns3-libs}
        ${ns3-contrib-libs}
        OpenSSL::SSL
        OpenSSL::Crypto

    EXECUTABLE_DIRECTORY_PATH
        ${scratch_directory}/
)