#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <printf.h>
#include <string.h>
#include <time.h>

#include "debug.h"

#include "iotlab_uid.h"
#include "mac_csma.h"
#include "phy.h"
#include "iotlab_i2c.h"

#include "iotlab_uid_num_hashtable.h"

// choose channel in [11-26]
#define CHANNEL 11
#define RADIO_POWER PHY_POWER_0dBm

#define ADDR_BROADCAST 0xFFFF

// App task handler
static void app_task(void *param);


/*
 * Radio config
 */
static void send_packet(uint8_t power_level)
{
    static char packet[PHY_MAX_TX_LENGTH - 4];  // 4 for mac layer
    static char pluspack[90]="12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789\0";
    
    // max pkt length <= max(cc2420, cc1101)
    snprintf(packet, sizeof(packet), "%u - 120 char payload packet - %s", power_level, pluspack);
    uint8_t length = 1 + strlen(packet);

    //printf("\nradio > ");
    if (!mac_csma_data_send(ADDR_BROADCAST, (uint8_t *)packet, length))
    {
        printf("FAILED: broadcast 100%% payload packet\n");
    }
}


/* Reception of a radio message */
void mac_csma_data_received(uint16_t src_addr,
        const uint8_t *data, uint8_t length, int8_t rssi, uint8_t lqi)
{
    struct node src_node = node_from_uid(src_addr);

    //printf("\nradio > ");
    printf("Got packet from %x (%s-%u). Len: %u Rssi: %d: '%s'\n",
            src_addr, src_node.type_str, src_node.num,
            length, rssi, (const char*)data);
}


static void hardware_init()
{
    // Openlab platform init
    platform_init();

    // Switch off the LEDs
    leds_off(LED_0 | LED_1 | LED_2);

    // Init csma Radio mac layer
    mac_csma_init(CHANNEL, RADIO_POWER);

    // Init control_node i2c
    iotlab_i2c_init();
}


int main()
{
    hardware_init();

    xTaskCreate(app_task, (const signed char * const) "broadcast_power_app_task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    platform_run();
    return 0;
}

static void app_task(void *param)
{
    vTaskDelay(configTICK_RATE_HZ * 18 * node_from_uid(iotlab_uid()).num);

    phy_power_t tx_power_list[16] = {
        PHY_POWER_m17dBm,
        PHY_POWER_m12dBm,
        PHY_POWER_m10dBm,
        PHY_POWER_m7dBm,
        PHY_POWER_m5dBm,
        PHY_POWER_m4dBm,
        PHY_POWER_m3dBm,
        PHY_POWER_m2dBm,
        PHY_POWER_m1dBm,
        PHY_POWER_0dBm,
        PHY_POWER_0_7dBm,
        PHY_POWER_1_3dBm,
        PHY_POWER_1_8dBm,
        PHY_POWER_2_3dBm,
        PHY_POWER_2_8dBm,
        PHY_POWER_3dBm
    };

    uint8_t i;
    for(i = 0; i<16; i++)
    {
        uint8_t j;
        for(j = 0; j<30; j++)
        {
            mac_csma_set_power(tx_power_list[i]);

            send_packet(tx_power_list[i]);

            // Delay by 1 second
            vTaskDelay(configTICK_RATE_HZ / 50);
        }
    }

    // 333 min delay
    vTaskDelay(20000 * configTICK_RATE_HZ);
}
