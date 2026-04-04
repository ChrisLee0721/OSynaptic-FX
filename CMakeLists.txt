cmake_minimum_required(VERSION 3.15)
project(osfx-c99 VERSION 1.0.0 LANGUAGES C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(OSFX_ENABLE_LTO "Enable link-time optimization" OFF)
option(OSFX_ENABLE_FILE_IO "Enable persistent storage" OFF)
option(OSFX_ENABLE_CLI "Enable command line interface" OFF)

option(OSFX_CFG_PLUGIN_TRANSPORT "Enable transport plugin" ON)
option(OSFX_CFG_PLUGIN_TEST_PLUGIN "Enable test_plugin plugin" ON)
option(OSFX_CFG_PLUGIN_PORT_FORWARDER "Enable port_forwarder plugin" ON)
option(OSFX_CFG_AUTOLOAD_TRANSPORT "Autoload transport plugin" ON)
option(OSFX_CFG_AUTOLOAD_TEST_PLUGIN "Autoload test_plugin plugin" ON)
option(OSFX_CFG_AUTOLOAD_PORT_FORWARDER "Autoload port_forwarder plugin" ON)

option(OSFX_CFG_PAYLOAD_DEVICE_ID "Enable DeviceId payload field" ON)
option(OSFX_CFG_PAYLOAD_DEVICE_STATUS "Enable DeviceStatus payload field" ON)
option(OSFX_CFG_PAYLOAD_TIMESTAMP "Enable Timestamp payload field" ON)
option(OSFX_CFG_PAYLOAD_SUB_TEMPLATE_ID "Enable SubTemplateId payload field" ON)
option(OSFX_CFG_PAYLOAD_SENSOR_ID "Enable SensorId payload field" ON)
option(OSFX_CFG_PAYLOAD_SENSOR_STATUS "Enable SensorStatus payload field" ON)
option(OSFX_CFG_PAYLOAD_PHYSICAL_ATTRIBUTE "Enable PhysicalAttribute payload field" OFF)
option(OSFX_CFG_PAYLOAD_NORMALIZED_VALUE "Enable NormalizedValue payload field" ON)
option(OSFX_CFG_PAYLOAD_GEOHASH_ID "Enable GeohashId payload field" OFF)
option(OSFX_CFG_PAYLOAD_SUPPLEMENTARY_MESSAGE "Enable SupplementaryMessage payload field" ON)
option(OSFX_CFG_PAYLOAD_RESOURCE_URL "Enable ResourceUrl payload field" OFF)

add_library(osfx_core STATIC
    src/osfx_solidity.c
    src/osfx_fusion_packet.c
    src/osfx_fusion_state.c
    src/osfx_standardization.c
    src/osfx_template_grammar.c
    src/osfx_secure_session.c
    src/osfx_id_allocator.c
    src/osfx_handshake_cmd.c
    src/osfx_handshake_dispatch.c
    src/osfx_payload_crypto.c
    src/osfx_protocol_matrix.c
    src/osfx_transporter_runtime.c
    src/osfx_service_runtime.c
    src/osfx_platform_runtime.c
    src/osfx_plugin_transport.c
    src/osfx_plugin_test.c
    src/osfx_plugin_port_forwarder.c
    src/osfx_cli_lite.c
    src/osfx_glue.c
    src/osfx_core_facade.c
    src/osfx_library_catalog.c
)

add_library(osfx::core ALIAS osfx_core)

target_include_directories(osfx_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(MSVC)
    target_compile_definitions(osfx_core PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if(OSFX_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT osfx_ipo_ok OUTPUT osfx_ipo_msg)
    if(osfx_ipo_ok)
        set_property(TARGET osfx_core PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "OSFX_ENABLE_LTO=ON but IPO/LTO is not supported: ${osfx_ipo_msg}")
    endif()
endif()

target_compile_definitions(osfx_core PUBLIC
    OSFX_ENABLE_FILE_IO=$<BOOL:${OSFX_ENABLE_FILE_IO}>
    OSFX_ENABLE_CLI=$<BOOL:${OSFX_ENABLE_CLI}>
    OSFX_CFG_PLUGIN_TRANSPORT=$<BOOL:${OSFX_CFG_PLUGIN_TRANSPORT}>
    OSFX_CFG_PLUGIN_TEST_PLUGIN=$<BOOL:${OSFX_CFG_PLUGIN_TEST_PLUGIN}>
    OSFX_CFG_PLUGIN_PORT_FORWARDER=$<BOOL:${OSFX_CFG_PLUGIN_PORT_FORWARDER}>
    OSFX_CFG_AUTOLOAD_TRANSPORT=$<BOOL:${OSFX_CFG_AUTOLOAD_TRANSPORT}>
    OSFX_CFG_AUTOLOAD_TEST_PLUGIN=$<BOOL:${OSFX_CFG_AUTOLOAD_TEST_PLUGIN}>
    OSFX_CFG_AUTOLOAD_PORT_FORWARDER=$<BOOL:${OSFX_CFG_AUTOLOAD_PORT_FORWARDER}>
    OSFX_CFG_PAYLOAD_DEVICE_ID=$<BOOL:${OSFX_CFG_PAYLOAD_DEVICE_ID}>
    OSFX_CFG_PAYLOAD_DEVICE_STATUS=$<BOOL:${OSFX_CFG_PAYLOAD_DEVICE_STATUS}>
    OSFX_CFG_PAYLOAD_TIMESTAMP=$<BOOL:${OSFX_CFG_PAYLOAD_TIMESTAMP}>
    OSFX_CFG_PAYLOAD_SUB_TEMPLATE_ID=$<BOOL:${OSFX_CFG_PAYLOAD_SUB_TEMPLATE_ID}>
    OSFX_CFG_PAYLOAD_SENSOR_ID=$<BOOL:${OSFX_CFG_PAYLOAD_SENSOR_ID}>
    OSFX_CFG_PAYLOAD_SENSOR_STATUS=$<BOOL:${OSFX_CFG_PAYLOAD_SENSOR_STATUS}>
    OSFX_CFG_PAYLOAD_PHYSICAL_ATTRIBUTE=$<BOOL:${OSFX_CFG_PAYLOAD_PHYSICAL_ATTRIBUTE}>
    OSFX_CFG_PAYLOAD_NORMALIZED_VALUE=$<BOOL:${OSFX_CFG_PAYLOAD_NORMALIZED_VALUE}>
    OSFX_CFG_PAYLOAD_GEOHASH_ID=$<BOOL:${OSFX_CFG_PAYLOAD_GEOHASH_ID}>
    OSFX_CFG_PAYLOAD_SUPPLEMENTARY_MESSAGE=$<BOOL:${OSFX_CFG_PAYLOAD_SUPPLEMENTARY_MESSAGE}>
    OSFX_CFG_PAYLOAD_RESOURCE_URL=$<BOOL:${OSFX_CFG_PAYLOAD_RESOURCE_URL}>
)

set_target_properties(osfx_core PROPERTIES
    OUTPUT_NAME osfx_core
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

install(TARGETS osfx_core
    EXPORT osfx_coreTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT osfx_coreTargets
    FILE osfx_coreTargets.cmake
    NAMESPACE osfx::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/osfx-c99
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/osfx-c99Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/osfx-c99Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/osfx-c99
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/osfx-c99ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/osfx-c99Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/osfx-c99ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/osfx-c99
)
