include_directories(${PROJECT_SOURCE_DIR}/appli/iotlab/lib
                    ${PROJECT_SOURCE_DIR}/appli/iotlab/periph)

if (PLATFORM STREQUAL "iotlab-a8-m3")
add_executable(receive_energy_a8_m3 main)
target_link_libraries(receive_energy_a8_m3
    platform
    rf2xx
    iotlab_uid_num_table
    iotlab_i2c
    mac_csma
    )

elseif (PLATFORM STREQUAL "iotlab-m3")
add_executable(receive_energy_m3 main)
target_link_libraries(receive_energy_m3
    platform
    rf2xx
    lps331ap
    isl29020
    iotlab_uid_num_table
    iotlab_i2c
    mac_csma
    )
endif (PLATFORM STREQUAL "iotlab-a8-m3")
