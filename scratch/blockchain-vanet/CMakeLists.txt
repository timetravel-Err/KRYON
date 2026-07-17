# CMakeLists.txt for the blockchain-vanet scratch subdirectory.
#
# ns-3's build system (ns-3.36+) will `add_subdirectory()` any scratch/<name>/
# folder that contains its own CMakeLists.txt instead of auto-globbing it,
# which is what lets us link OpenSSL here (needed for ECDSA/SHA-256/HMAC).
#
# If your ns-3 build doesn't pick this up automatically, see the fallback
# manual-compile command in README.md.

find_package(OpenSSL REQUIRED)

file(GLOB BLOCKCHAIN_VANET_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

build_exec(
  EXECNAME blockchain-vanet
  SOURCE_FILES "${BLOCKCHAIN_VANET_SOURCES}"
  LIBRARIES_TO_LINK "${ns3-libs};${ns3-contrib-libs};OpenSSL::Crypto"
  EXECUTABLE_DIRECTORY_PATH ${CMAKE_OUTPUT_DIRECTORY}/scratch/blockchain-vanet/
)
