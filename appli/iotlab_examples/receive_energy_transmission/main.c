#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <printf.h>
#include <string.h>
#include <time.h>
#include <packer.h>

#include "phy_rf2xx/phy_rf2xx.h"
#include "rf2xx.h"
#ifdef IOTLAB_M3
#include "lps331ap.h"
#include "isl29020.h"
#endif
#include "iotlab_uid.h"
#include "mac_csma.h"
#include "phy.h"
#include "iotlab_i2c.h"

#include "iotlab_uid_num_hashtable.h"

// choose channel in [11-26]
#define CHANNEL 11
#define RADIO_POWER PHY_POWER_3dBm

#define ADDR_BROADCAST 0xFFFF

// Sync vars
#define CASE_DURATION 4 // in seconds

// App task handler
static void app_task(void *param);


#ifdef IOTLAB_M3
/**
 * Sensors
 *
static int16_t temperature_sensor()
{
    int16_t temperature;
    uint8_t error_code = lps331ap_read_temp(&temperature);

    if(error_code)
    {
        printf("ERROR Temperature Sensor | Error Code: %u", error_code);
    }

    return temperature;
}

static float light_sensor()
{
    float light = isl29020_read_sample();

    return light;
}

static uint32_t pressure_sensor()
{
    uint32_t pressure;
    uint8_t error_code = lps331ap_read_pres(&pressure);

    if(error_code)
    {
        printf("ERROR Pressure Sensor | Error Code: %u", error_code);
    }

    return pressure;
}*/
#endif


/*
 * Radio config
 *
static void send_sensor_packet()
{
    static char packet[PHY_MAX_TX_LENGTH - 4];  // 4 for mac layer
    // max pkt length <= max(cc2420, cc1101)
#ifdef IOTLAB_M3
    snprintf(packet, sizeof(packet), "T:%f L:%f P:%f", (42.5 + temperature_sensor() / 480.0), light_sensor(), (pressure_sensor() / 4096.0));
#else
    snprintf(packet, sizeof(packet), "T:20.0 L:20000 P:1013");
#endif
    uint16_t length = 1 + strlen(packet);

    printf("\nradio > ");
    if (!mac_csma_data_send(ADDR_BROADCAST, (uint8_t *)packet, length))
        printf("FAILED: sending sensor packet\n");
}

static void send_25_packet()
{
    static char packet[PHY_MAX_TX_LENGTH - 4];  // 4 for mac layer
    static char pluspack[6]="56789\0";

    snprintf(packet, sizeof(packet), "30 char payload packet - %s", pluspack);
    uint16_t length = 1 + strlen(packet);

    printf("\nradio > ");
    if (!mac_csma_data_send(ADDR_BROADCAST, (uint8_t *)packet, length))
        printf("FAILED: sending 25%% payload packet\n");
}

static void send_50_packet()
{
    static char packet[PHY_MAX_TX_LENGTH - 4];  // 4 for mac layer
    static char pluspack[36]="56789012345678901234567890123456789\0";

    snprintf(packet, sizeof(packet), "60 char payload packet - %s", pluspack);
    uint16_t length = 1 + strlen(packet);

    printf("\nradio > ");
    if (!mac_csma_data_send(ADDR_BROADCAST, (uint8_t *)packet, length))
        printf("FAILED: sending 50%% payload packet\n");
}

static void send_75_packet()
{
    static char packet[PHY_MAX_TX_LENGTH - 4];  // 4 for mac layer
    static char pluspack[66]="56789012345678901234567890123456789012345678901234567890123456789\0";

    snprintf(packet, sizeof(packet), "90 char payload packet - %s", pluspack);
    uint16_t length = 1 + strlen(packet);

    printf("\nradio > ");
    if (!mac_csma_data_send(ADDR_BROADCAST, (uint8_t *)packet, length))
        printf("FAILED: sending 75%% payload packet\n");
}

static void broadcast_100_packet(phy_power_t tx_power)
{
    static char packet[PHY_MAX_TX_LENGTH - 4];  // 4 for mac layer
    static char pluspack[95]="78901234567890123456789012345678901234567890123456789012345678901234567890123456789\0";

    snprintf(packet, sizeof(packet), "BROADCAST: 120 char payload packet - %s", pluspack);
    uint16_t length = 1 + strlen(packet);

    printf("\nradio > ");
    if (!mac_csma_data_send(tx_power, ADDR_BROADCAST, (uint8_t *)packet, length))
        printf("FAILED: sending 100%% payload packet\n");
}

static void unicast_100_packet(phy_power_t tx_power, uint16_t dest_addr)
{
    static char packet[PHY_MAX_TX_LENGTH - 4];  // 4 for mac layer
    static char pluspack[95]="5678901234567890123456789012345678901234567890123456789012345678901234567890123456789\0";

    snprintf(packet, sizeof(packet), "UNICAST: 120 char payload packet - %s", pluspack);
    uint16_t length = 1 + strlen(packet);

    printf("\nradio > ");
    if (!mac_csma_data_send(tx_power, dest_addr, (uint8_t *)packet, length))
        printf("FAILED: sending 100%% payload packet\n");
}*/

static void send_100_packet(phy_power_t tx_power, uint16_t dest_addr)
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

    // Set power
    phy_set_power(platform_phy, tx_power);

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
 * All LEDs off
 */
static void all_leds_off()
{
    printf("\nall leds off\n");
    leds_off(LED_0 | LED_1 | LED_2);
}


/**
 * Toggle all LEDs
 */
/*static void all_leds_toggle()
{
    printf("\ntoggle all leds\n");
    leds_toggle(LED_0 | LED_1 | LED_2);
}*/


static void hardware_init()
{
    // Openlab platform init
    platform_init();

    // Set channel
    phy_set_channel(platform_phy, CHANNEL);

    // Switch off the LEDs
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

    // Init csma Radio mac layer
    mac_csma_init(CHANNEL, RADIO_POWER);
*/
    // Init control_node i2c
    iotlab_i2c_init();
}


int main()
{
    hardware_init();
    
    xTaskCreate(app_task, (const signed char * const) "receive_energy_transmission_app_task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    platform_run();
    return 0;
}

static void app_task(void *param)
{
    vTaskDelay(configTICK_RATE_HZ * 10);

    // Sync time series
    uint8_t i;
    for(i = 0; i<250; i++)
    {

        printf("radio > Broadcast\n\n");
        send_100_packet(RADIO_POWER, ADDR_BROADCAST);
        while (((phy_rf2xx_t*) platform_phy)->state != PHY_STATE_IDLE)
        {
            asm volatile("nop");
        }
    }

    vTaskDelay(configTICK_RATE_HZ * 2);
    portTickType t_start = xTaskGetTickCount();

    for(i = 0; i<30; i++)
    {
        printf("radio > Broadcast\n\n");
        send_100_packet(RADIO_POWER, ADDR_BROADCAST);
        // Wait till current case is done
        vTaskDelayUntil(&t_start, CASE_DURATION * configTICK_RATE_HZ);
        // Start next case
        t_start = xTaskGetTickCount();
    }

    printf("> Done");
    // 20 min delay
    vTaskDelay(240 * CASE_DURATION * configTICK_RATE_HZ);
}
