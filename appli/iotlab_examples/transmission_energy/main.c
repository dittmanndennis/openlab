#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <printf.h>
#include <string.h>
#include <time.h>
#include <packer.h>
#include "debug.h"

#include "phy_rf2xx/phy_rf2xx.h"
#include "rf2xx.h"
/*#ifdef IOTLAB_M3
#include "lps331ap.h"
#include "isl29020.h"
#endif*/
#include "iotlab_uid.h"
#include "phy.h"
#include "iotlab_i2c.h"

#include "iotlab_uid_num_hashtable.h"

// choose channel in [11-26]
#define CHANNEL 11
#define ADDR_BROADCAST 0xFFFF

// Sync vars
#define SYNC_SEQUENCE 22
#define CASE_DURATION 5 // in seconds

// App task handler
static void app_task(void *param);


static void send_100_packet(uint16_t dest_addr)
{
    static char packet[PHY_MAX_TX_LENGTH - 4];  // 4 for mac layer
    static char pluspack[95]="4321098765678901234567890123456789012345678901234567890123456789012345678901234567890123456789\0";

    snprintf(packet, sizeof(packet), "120 char payload packet - %s", pluspack);
    uint8_t length = 1 + strlen(packet);

    phy_packet_t tx_pkt;
    phy_prepare_packet(&tx_pkt);
    uint8_t* pkt_data = tx_pkt.data;

    // Set our address, then destination address
    pkt_data = packer_uint16_pack(pkt_data, platform_uid());
    pkt_data = packer_uint16_pack(pkt_data, dest_addr);

    // Copy payload
    memcpy(pkt_data, (uint8_t *)packet, length);
    tx_pkt.length = 4 + length;

    if(phy_tx_now(platform_phy, &tx_pkt, NULL) != PHY_SUCCESS)
    {
        printf("FAILED: unicast 100%% payload packet\n");
    }
}


/**
 * All LEDs on
 */
/*static void all_leds_on()
{
    printf("\nall leds on\n");
    leds_on(LED_0 | LED_1 | LED_2);
}*/


/**
 * Toggle all LEDs
 */
/*static void all_leds_toggle()
{
    printf("\ntoggle all leds\n");
    leds_toggle(LED_0 | LED_1 | LED_2);
}*/


/**
 * All LEDs off
 */
static void all_leds_off()
{
    printf("\nall leds off\n");
    leds_off(LED_0 | LED_1 | LED_2);
}


static void hardware_init()
{
    // Openlab platform init
    platform_init();

    // Set channel
    phy_set_channel(platform_phy, CHANNEL);

    // Switch off the LEDs
    //leds_off(LED_0 | LED_1 | LED_2);
    all_leds_off();
/*
#ifdef IOTLAB_M3
    // ISL29020 light sensor initialisation
    isl29020_prepare(ISL29020_LIGHT__AMBIENT, ISL29020_RESOLUTION__16bit,
            ISL29020_RANGE__16000lux);
    isl29020_sample_continuous();

    // LPS331AP pressure sensor initialisation
    lps331ap_powerdown();
    lps331ap_set_datarate(LPS331AP_P_12_5HZ_T_12_5HZ);
#endif
*/
    // Init control_node i2c
    iotlab_i2c_init();
}


int main()
{
    hardware_init();
    
    xTaskCreate(app_task, (const signed char * const) "transmission_energy_app_task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    platform_run();
    return 0;
}

static char* state_to_string(phy_rf2xx_state_t state)
{
    switch(state)
    {
        case PHY_STATE_SLEEP:
            return "PHY_STATE_SLEEP\0";
        case PHY_STATE_IDLE:
            return "PHY_STATE_IDLE\0";
        case PHY_STATE_RX_WAIT:
            return "PHY_STATE_RX_WAIT\0";
        case PHY_STATE_RX:
            return "PHY_STATE_RX\0";
        case PHY_STATE_TX_WAIT:
            return "PHY_STATE_TX_WAIT\0";
        case PHY_STATE_TX:
            return "PHY_STATE_TX\0";
        case PHY_STATE_JAMMING:
            return "PHY_STATE_JAMMING\0";
        default:
            log_error("ILLEGAL STATE. Could not match given state to existing state.");
            return "UNKNOWN_STATE\0";
    }
}

static void app_task(void *param)
{   
    // Start in sleep mode
    phy_sleep(platform_phy);
    
    vTaskDelay(configTICK_RATE_HZ * 10);
    //uint32_t node_num = node_from_uid(iotlab_uid()).num;
    //printf("Node: %u UID: %x\n", node_num, iotlab_uid());

    // Sync time series
    uint16_t sync = SYNC_SEQUENCE;
    uint8_t i;
    portTickType t_start = xTaskGetTickCount();
    for(i = 0; i<8; i++)
    {
        if(sync & 0x80)
        {
            printf("radio > Jam\n\n");
            phy_jam(platform_phy, CHANNEL, PHY_POWER_3dBm);
            // Wait till current case is done
            vTaskDelayUntil(&t_start, CASE_DURATION * configTICK_RATE_HZ);
            // Start next case
            t_start = xTaskGetTickCount();
            // Stop jaming and go back to sleep
            phy_reset(platform_phy);
        }
        else
        {
            // Sleep mode is already enabled
            printf("> Sleep\n\n");
            // Wait till current case is done
            vTaskDelayUntil(&t_start, CASE_DURATION * configTICK_RATE_HZ);
            // Start next case
            t_start = xTaskGetTickCount();
        }
        sync <<= 1;
    }

    // RX_ON
    phy_idle(platform_phy);
    printf("> RX_ON\n\n");
    rf2xx_set_state(((phy_rf2xx_t*) platform_phy)->radio, RF2XX_TRX_STATE__RX_ON);
    // Wait till current case is done
    vTaskDelayUntil(&t_start, CASE_DURATION * configTICK_RATE_HZ);
    // Start next case
    t_start = xTaskGetTickCount();

    // PLL_ON
    phy_idle(platform_phy);
    printf("> PLL_ON\n\n");
    rf2xx_set_state(((phy_rf2xx_t*) platform_phy)->radio, RF2XX_TRX_STATE__PLL_ON);
    // Wait till current case is done
    vTaskDelayUntil(&t_start, CASE_DURATION * configTICK_RATE_HZ);
    // Start next case
    t_start = xTaskGetTickCount();

    // Idle
    printf("> Idle\n\n");
    phy_idle(platform_phy);
    // Wait till current case is done
    vTaskDelayUntil(&t_start, CASE_DURATION * configTICK_RATE_HZ);
    // Start next case
    t_start = xTaskGetTickCount();

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

    for(i = 0; i<16;)
    {
        // Set power
        phy_set_power(platform_phy, tx_power_list[i]);
        // Number of readings for statistical significance
        int j;
        for(j = 0; j<30; j++)
        {
            // 2: 9982, 3: b868, 100: b080, 101: 0x9181, 102: 0xa881
            printf("radio > Unicast - %u\n", i);
            send_100_packet(0xb080);
            // Wait till current case is done
            vTaskDelayUntil(&t_start, CASE_DURATION * configTICK_RATE_HZ);
            // Start next case
            t_start = xTaskGetTickCount();
            if(((phy_rf2xx_t*) platform_phy)->state != PHY_STATE_IDLE)
            {
                log_error("BROADCAST SKIPPED. Wrong state %s", state_to_string(((phy_rf2xx_t*) platform_phy)->state));
                continue;
            }

            printf("radio > Broadcast - %u\n", i);
            send_100_packet(ADDR_BROADCAST);
            // Wait till current case is done
            vTaskDelayUntil(&t_start, CASE_DURATION * configTICK_RATE_HZ);
            // Start next case
            t_start = xTaskGetTickCount();
            if(((phy_rf2xx_t*) platform_phy)->state != PHY_STATE_IDLE)
            {
                log_error("UNICAST SKIPPED. Wrong state %s", state_to_string(((phy_rf2xx_t*) platform_phy)->state));
                continue;
            }
        }

        // Next power level
        i++;
    }

    printf("> Sleep\n\n");
    phy_sleep(platform_phy);
    // 20 min delay
    vTaskDelay(240 * CASE_DURATION * configTICK_RATE_HZ);
}
