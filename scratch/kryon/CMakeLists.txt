find_package(OpenSSL REQUIRED)

# Compute the output directory the same way ns-3 does
get_filename_component(
    scratch_absolute_directory
    ${CMAKE_CURRENT_SOURCE_DIR}
    ABSOLUTE
)

string(
    REPLACE
    "${PROJECT_SOURCE_DIR}"
    "${CMAKE_OUTPUT_DIRECTORY}"
    scratch_directory
    "${scratch_absolute_directory}"
)

# ----------------------------------------------------------
# KRYON Simulator
# ----------------------------------------------------------

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

# ----------------------------------------------------------
# ECC Test
# ----------------------------------------------------------

build_exec(
    EXECNAME test-ecc
    EXECNAME_PREFIX scratch_kryon_

    SOURCE_FILES
        test-ecc.cc

    LIBRARIES_TO_LINK
        ${ns3-libs}
        ${ns3-contrib-libs}
        OpenSSL::SSL
        OpenSSL::Crypto

    EXECUTABLE_DIRECTORY_PATH
        ${scratch_directory}/
)

# ----------------------------------------------------------
# RAP Test
# ----------------------------------------------------------

build_exec(
    EXECNAME test-rap
    EXECNAME_PREFIX scratch_kryon_

    SOURCE_FILES
        test-rap.cc

    LIBRARIES_TO_LINK
        ${ns3-libs}
        ${ns3-contrib-libs}
        OpenSSL::SSL
        OpenSSL::Crypto

    EXECUTABLE_DIRECTORY_PATH
        ${scratch_directory}/
)

# ----------------------------------------------------------
# Session Manager Test
# ----------------------------------------------------------

build_exec(
    EXECNAME test-session-manager
    EXECNAME_PREFIX scratch_kryon_

    SOURCE_FILES
        test-session-manager.cc

    LIBRARIES_TO_LINK
        ${ns3-libs}
        ${ns3-contrib-libs}
        OpenSSL::SSL
        OpenSSL::Crypto

    EXECUTABLE_DIRECTORY_PATH
        ${scratch_directory}/
)
# ----------------------------------------------------------
# Session Reuse Test
# ----------------------------------------------------------

build_exec(
    EXECNAME test-session-reuse
    EXECNAME_PREFIX scratch_kryon_

    SOURCE_FILES
        test-session-reuse.cc

    LIBRARIES_TO_LINK
        ${ns3-libs}
        ${ns3-contrib-libs}
        OpenSSL::SSL
        OpenSSL::Crypto

    EXECUTABLE_DIRECTORY_PATH
        ${scratch_directory}/
)
# ----------------------------------------------------------
# Session Expiry + Re-authentication Test
# ----------------------------------------------------------

build_exec(
    EXECNAME test-session-expiry-reauth
    EXECNAME_PREFIX scratch_kryon_

    SOURCE_FILES
        test-session-expiry-reauth.cc

    LIBRARIES_TO_LINK
        ${ns3-libs}
        ${ns3-contrib-libs}
        OpenSSL::SSL
        OpenSSL::Crypto

    EXECUTABLE_DIRECTORY_PATH
        ${scratch_directory}/
)

# ----------------------------------------------------------
# Test Secure Channel 
# ----------------------------------------------------------

build_exec(
    EXECNAME test-secure-channel
    EXECNAME_PREFIX scratch_kryon_

    SOURCE_FILES
        test-secure-channel.cc

    LIBRARIES_TO_LINK
        ${ns3-libs}
        ${ns3-contrib-libs}
        OpenSSL::SSL
        OpenSSL::Crypto

    EXECUTABLE_DIRECTORY_PATH
        ${scratch_directory}/
)
# ----------------------------------------------------------
# Test Secure Channel Replay Protection
# ----------------------------------------------------------

build_exec(
    EXECNAME test-secure-channel-replay
    EXECNAME_PREFIX scratch_kryon_

    SOURCE_FILES
        test-secure-channel-replay.cc

    LIBRARIES_TO_LINK
        ${ns3-libs}
        ${ns3-contrib-libs}
        OpenSSL::SSL
        OpenSSL::Crypto

    EXECUTABLE_DIRECTORY_PATH
        ${scratch_directory}/
)
# ----------------------------------------------------------
# Test Secure Channel Out-of-Order Protection
# ----------------------------------------------------------

build_exec(
    EXECNAME test-secure-channel-out-of-order
    EXECNAME_PREFIX scratch_kryon_

    SOURCE_FILES
        test-secure-channel-out-of-order.cc

    LIBRARIES_TO_LINK
        ${ns3-libs}
        ${ns3-contrib-libs}
        OpenSSL::SSL
        OpenSSL::Crypto

    EXECUTABLE_DIRECTORY_PATH
        ${scratch_directory}/
)
# ----------------------------------------------------------
# RAP + Secure Channel Integration Test
# ----------------------------------------------------------

build_exec(
    EXECNAME test-rap-secure-channel-integration
    EXECNAME_PREFIX scratch_kryon_

    SOURCE_FILES
        test-rap-secure-channel-integration.cc

    LIBRARIES_TO_LINK
        ${ns3-libs}
        ${ns3-contrib-libs}
        OpenSSL::SSL
        OpenSSL::Crypto

    EXECUTABLE_DIRECTORY_PATH
        ${scratch_directory}/
)