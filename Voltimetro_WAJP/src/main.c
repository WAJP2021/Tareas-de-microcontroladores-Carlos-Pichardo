#include <stdio.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/adc.h>

// Definiciones de constantes y variables globales
#define ADC_CHANNEL ADC1_CHANNEL_6
#define LED_GPIO 2
#define VOLTAGE_OBJECTIVE 18.0f
#define NUM_SAMPLES 100

float voltajeRMS;
float voltajes[NUM_SAMPLES];

// Prototipos de funciones
void initHardware(void);
void readAndProcessVoltages(void);

void app_main()
{
    initHardware();  // Inicialización del hardware

    while (1)
    {
        readAndProcessVoltages();  // Leer y procesar muestras de voltaje
        vTaskDelay(pdMS_TO_TICKS(1000));  // Esperar 1 segundo antes de la próxima iteración
    }
}

// Función para inicializar hardware
void initHardware(void)
{
    // Configuración del ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_12);

    // Configuración del GPIO para LED
    gpio_config_t io_config;
    io_config.mode = GPIO_MODE_OUTPUT;
    io_config.pin_bit_mask = (1ULL << LED_GPIO);
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    io_config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_config);
}

// Función para leer muestras de voltaje y calcular RMS
void readAndProcessVoltages(void)
{
    float sumSquaredVoltages = 0.0f;

    // Leer 100 muestras de voltaje
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        int rawValue = adc1_get_raw(ADC_CHANNEL);
        float voltage = rawValue * VOLTAGE_OBJECTIVE / 4095.0f;
        voltajes[i] = voltage;
        sumSquaredVoltages += voltage * voltage;
        vTaskDelay(pdMS_TO_TICKS(1));  // Esperar 1 ms entre cada lectura
    }

    // Calcular el voltaje RMS
    float meanSquaredVoltage = sumSquaredVoltages / NUM_SAMPLES;
    voltajeRMS = sqrtf(meanSquaredVoltage);

    // Imprimir el voltaje RMS
    printf("Voltaje RMS: %4.2fV\n", voltajeRMS);
}