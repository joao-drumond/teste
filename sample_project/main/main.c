#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h" // Adicionado para utilizar a função de registro de log

#define GPIO_OUTPUT_IO_0     2
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<GPIO_OUTPUT_IO_0)

#define GPIO_INPUT_IO_0     21
#define GPIO_INPUT_IO_1     22
#define GPIO_INPUT_IO_2     23
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1) | (1ULL<<GPIO_INPUT_IO_2))

#define ESP_INTR_FLAG_DEFAULT 0

static QueueHandle_t gpio_evt_queue = NULL;

// INTERRUPÇÃO
static void IRAM_ATTR gpio_isr_handler(void* arg) // Quando tiver IRAM é uma interrupção 
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL); // ISR = Interrupt service routine
}

// ENVIO DE TASK PRA FILA
static void gpio_task_auxiliar(void* arg) // É uma tarefa (task) -> Tem "cara" de função e tem loop infinito
{
    uint32_t io_num;
    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
           // printf("GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));

        ESP_LOGI(TAG_GPIO_INFO, "Botão acionado: GPIO[%"PRIu32"] - Estado: %d\n", io_num, gpio_get_level(io_num));
        
        }
    }
}

static const char* TAG_INFO = "System Info"; // O asterístico é pra declarar como ponteiro? Sim
static const char* TAG_GPIO_INFO = "GPIOs Info"; // O asterístico é pra declarar como ponteiro? Sim

void app_main(void){
    /*
    esp_log_level_set(TAG_INFO, ESP_LOG_INFO); // Essa função e o ESP_LOG_INFO estão dentro de uma biblioteca

    // Print chip information 
    esp_chip_info_t chip_info; // Declaração de um tipo de variável usando a estrutura esp_chip_info_t
    uint32_t flash_size; 
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
    */

    // --------------------------------------- AULA 2 -------------------------------------------- //

    // CONFIGURANDO A SAÍDA
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    // CONFIGURANDO AS ENTRADAS
    //interrupt of falling edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    // CONFIGURANDO E CRIANDO A FILA
    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t)); // 10 posicoes de 32 bits
    //start gpio task
    xTaskCreate(gpio_task_auxiliar, "gpio_task_auxiliar", 2048, NULL, 10, NULL);

    // CONFIGURANDO A INTERRUPÇÃO
    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_2, gpio_isr_handler, (void*) GPIO_INPUT_IO_2);

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}