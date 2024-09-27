#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <printf.h>
#include <string.h>
#include <time.h>

#include "iotlab_uid.h"
#include "mac_csma.h"
#include "phy.h"
#include "iotlab_i2c.h"
#include "phy_rf2xx/phy_rf2xx.h"
#include "rf2xx.h"

#include "iotlab_uid_num_hashtable.h"

// choose channel in [11-26]
#define CHANNEL 11
#define RADIO_POWER PHY_POWER_0dBm


/* Reception of a radio message */
void mac_csma_data_received(uint16_t src_addr,
        const uint8_t *data, uint8_t length, int8_t rssi, uint8_t lqi)
{
    struct node src_node = node_from_uid(src_addr);

    printf("\nradio > ");
    printf("Got packet from %x (%s-%u). Len: %u Rssi: %d: '%s'\n",
            src_addr, src_node.type_str, src_node.num,
            length, rssi, (const char*)data);
}


static void hardware_init()
{
    // Openlab platform init
    platform_init();

    // Set channel
    phy_set_channel(platform_phy, CHANNEL);

    // Init control_node i2c
    iotlab_i2c_init();
}


int main()
{
    hardware_init();

    // Init csma Radio mac layer
    mac_csma_init(CHANNEL, RADIO_POWER);

    phy_idle(platform_phy);
    // Set RSSI threshold
    rf2xx_set_rx_rssi_threshold(((phy_rf2xx_t*) platform_phy)->radio, RF2XX_PHY_RX_THRESHOLD__m101dBm);

    uint32_t node_num = node_from_uid(iotlab_uid()).num;
    printf("Node: %u UID: %u\n\n", node_num, iotlab_uid());

    platform_run();
    return 0;
}
