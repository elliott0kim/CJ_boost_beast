# Automatically generated by scripts/boost/generate-ports.ps1

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO boostorg/bind
    REF boost-${VERSION}
    SHA512 c02facb201f11b770e4160220fc272660c89ae0f3b10550dd1056678984f09b0d40f7b8023b32c8960ce930947f2175ff7a4a5986c56db6481762c89be799078
    HEAD_REF master
)

set(FEATURE_OPTIONS "")
boost_configure_and_install(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS ${FEATURE_OPTIONS}
)