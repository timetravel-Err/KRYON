find_package(OpenSSL REQUIRED)

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
)