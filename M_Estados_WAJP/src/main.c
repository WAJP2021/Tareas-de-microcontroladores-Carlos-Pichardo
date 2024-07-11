// Librerías utilizadas
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <freertos/timers.h>
#include "esp_log.h"

// Definiciones de constantes y variables globales
static const char *TAG = "Main";

#define TRUE 1
#define FALSE 0

#define ESTADO_INIT         0
#define ESTADO_ABRIENDO     1
#define ESTADO_CERRANDO     2
#define ESTADO_CERRADO      3
#define ESTADO_ABIERTO      4
#define ESTADO_EMERGENCIA   5
#define ESTADO_ERROR        6
#define ESTADO_ESPERA       7

// Definición de pines
#define LSC_PIN 13
#define LSA_PIN 12
#define FC_PIN  14
#define CC_PIN  27
#define CA_PIN  26

#define LED_M_PIN 4
#define LED_E_PIN 16
#define MC_PIN    17
#define MA_PIN    5

// Variables globales
volatile int ESTADO_ACTUAL = ESTADO_INIT;
volatile int ESTADO_SIGUIENTE = ESTADO_INIT;
volatile int ESTADO_ANTERIOR = ESTADO_INIT;

// Estructuras de entradas y salidas
volatile struct INPUTS {
    unsigned int LSA: 1;
    unsigned int LSC: 1;
    unsigned int CA:  1;
    unsigned int CC:  1;
    unsigned int FC:  1;
} inputs;

volatile struct OUTPUTS {
    unsigned int MC: 1;
    unsigned int MA: 1;
    unsigned int LED_EMERGENCIA: 1;
    unsigned int LED_MOVIMIENTO: 1;
} outputs;

// Handle del timer
TimerHandle_t xTimers;
int timerID = 1;
int INTERVALO = 50;

// Prototipos de funciones
esp_err_t INTERRUPCION_50MS(void);
esp_err_t SET_TIMER(void);

int Func_ESTADO_INIT(void);
int Func_ESTADO_ABRIENDO(void);
int Func_ESTADO_CERRANDO(void);
int Func_ESTADO_CERRADO(void);
int Func_ESTADO_ABIERTO(void);
int Func_ESTADO_EMERGENCIA(void);
int Func_ESTADO_ERROR(void);
int Func_ESTADO_ESPERA(void);

/*****************************************************/

// Función de callback del timer
void vTimerCallback(TimerHandle_t pxTimer) {
    ESP_LOGI(TAG, "INTERRUPCIÓN HECHA.");
    INTERRUPCION_50MS();
}

// Función principal
void app_main() {
    ESP_LOGI(TAG, "Iniciando programa...");

    ESTADO_SIGUIENTE = Func_ESTADO_INIT();
    SET_TIMER();

    for (;;) {
        switch (ESTADO_SIGUIENTE) {
            case ESTADO_INIT:
                ESTADO_SIGUIENTE = Func_ESTADO_INIT();
                break;
            case ESTADO_ESPERA:
                ESTADO_SIGUIENTE = Func_ESTADO_ESPERA();
                break;
            case ESTADO_ABRIENDO:
                ESTADO_SIGUIENTE = Func_ESTADO_ABRIENDO();
                break;
            case ESTADO_CERRANDO:
                ESTADO_SIGUIENTE = Func_ESTADO_CERRANDO();
                break;
            case ESTADO_CERRADO:
                ESTADO_SIGUIENTE = Func_ESTADO_CERRADO();
                break;
            case ESTADO_ABIERTO:
                ESTADO_SIGUIENTE = Func_ESTADO_ABIERTO();
                break;
            case ESTADO_EMERGENCIA:
                ESTADO_SIGUIENTE = Func_ESTADO_EMERGENCIA();
                break;
            case ESTADO_ERROR:
                ESTADO_SIGUIENTE = Func_ESTADO_ERROR();
                break;
            default:
                break;
        }
    }
}

// Función de inicialización del estado inicial
int Func_ESTADO_INIT() {
    ESP_LOGI(TAG, "Inicializando estado INIT");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_INIT;

    // Configuración de pines GPIO
    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << LSA_PIN) | (1ULL << LSC_PIN) |
                           (1ULL << CA_PIN) | (1ULL << CC_PIN) |
                           (1ULL << FC_PIN);
    gpio_config(&io_conf);

    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << LED_M_PIN) | (1ULL << LED_E_PIN) |
                           (1ULL << MC_PIN) | (1ULL << MA_PIN);
    gpio_config(&io_conf);

    // Retornar el próximo estado
    return ESTADO_ESPERA;
}

// Función del estado ABRIENDO
int Func_ESTADO_ABRIENDO() {
    ESP_LOGI(TAG, "Estado ABRIENDO");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ABRIENDO;

    // Configurar salidas
    outputs.LED_MOVIMIENTO = TRUE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = TRUE;
    outputs.MC = FALSE;

    for (;;) {
        if (inputs.LSA == TRUE) {
            return ESTADO_ABIERTO;
        }
        if (inputs.LSA == TRUE && inputs.LSC == TRUE) {
            return ESTADO_ERROR;
        }
        if (inputs.FC == TRUE) {
            return ESTADO_EMERGENCIA;
        }
        if (inputs.CC == TRUE) {
            return ESTADO_CERRANDO;
        }
        // Falta la verificación del tiempo aquí
    }
}

// Función del estado CERRANDO
int Func_ESTADO_CERRANDO() {
    ESP_LOGI(TAG, "Estado CERRANDO");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_CERRANDO;

    // Configurar salidas
    outputs.LED_MOVIMIENTO = TRUE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = FALSE;
    outputs.MC = TRUE;

    for (;;) {
        if (inputs.LSC == TRUE) {
            return ESTADO_CERRADO;
        }
        if (inputs.LSA == TRUE && inputs.LSC == TRUE) {
            return ESTADO_ERROR;
        }
        if (inputs.FC == TRUE) {
            return ESTADO_EMERGENCIA;
        }
        if (inputs.CA == TRUE) {
            return ESTADO_ABRIENDO;
        }
        // Falta la verificación del tiempo aquí
    }
}

// Función del estado CERRADO
int Func_ESTADO_CERRADO() {
    ESP_LOGI(TAG, "Estado CERRADO");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_CERRADO;

    // Configurar salidas
    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    for (;;) {
        return ESTADO_ESPERA;
    }
}

// Función del estado ABIERTO
int Func_ESTADO_ABIERTO() {
    ESP_LOGI(TAG, "Estado ABIERTO");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ABIERTO;

    // Configurar salidas
    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    for (;;) {
        return ESTADO_ESPERA;
    }
}

// Función del estado EMERGENCIA
int Func_ESTADO_EMERGENCIA() {
    ESP_LOGI(TAG, "Estado EMERGENCIA");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_EMERGENCIA;

    // Configurar salidas
    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = TRUE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    for (;;) {
        vTaskDelay(1500 / portTICK_PERIOD_MS);
        if (inputs.FC == FALSE) {
            return ESTADO_ANTERIOR;
        }
    }
}

// Función del estado ERROR
int Func_ESTADO_ERROR() {
    ESP_LOGI(TAG, "Estado ERROR");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ERROR;

    // Configurar salidas
    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = TRUE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    for (;;) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        outputs.LED_EMERGENCIA = !outputs.LED_EMERGENCIA;
    }
}