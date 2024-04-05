#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h" // Adicionado para utilizar a função de registro de log

static const char* TAG_INFO = "System Info"; // O asterístico é pra declarar como ponteiro? Sim

void app_main(void){
     
    esp_log_level_set(TAG_INFO, ESP_LOG_INFO); // ESP_LOG_INFO não foi declarado. Como ele é interpretdo? Está dentro de uma biblioteca

    /* Print chip information */
    esp_chip_info_t chip_info; // Declaração de um tipo de variável usando a estrutura esp_chip_info_t
    uint32_t flash_size; // Confirmar com Túlio de onde vem essa estrutura
    esp_chip_info(&chip_info);

    ESP_LOGI(TAG_INFO, "This is %s chip with %d CPU core(s), %s%s%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
            (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
            (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    
    ESP_LOGI(TAG_INFO, "silicon revision v%d.%d, ", major_rev, minor_rev);
}