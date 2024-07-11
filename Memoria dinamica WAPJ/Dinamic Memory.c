#include <stdio.h>
#include <string.h>

// Definición de la estructura para los datos del alumno
typedef struct {
    char nombre[50];
    int edad;
    int nota;
} Alumno;

// Declaración de funciones
void inicializarSPIFFS(void);
void grabarDato(void);
void leerDisco(void);
void salir(void);

// Tarea principal
void app_main() {
    // Inicialización de SPIFFS
    inicializarSPIFFS();

    // Creación de tarea para el menú
    xTaskCreate(menu_task, "menu_task", 4096, NULL, 1, NULL);
}


// Función para grabar datos de un alumno en SPIFFS
void grabarDato(void) {
    Alumno alumno;

    // Solicitar y guardar los datos del alumno
    printf("Ingrese el nombre del alumno: ");
    fgets(alumno.nombre, sizeof(alumno.nombre), stdin);
    alumno.nombre[strcspn(alumno.nombre, "\n")] = '\0'; // Eliminar el salto de línea del final

    printf("Ingrese la edad del alumno: ");
    scanf("%d", &alumno.edad);

    printf("Ingrese la nota del alumno: ");
    scanf("%d", &alumno.nota);

    // Abrir el archivo para escritura
    FILE *archivo = fopen("/spiffs/alumnos.dat", "a");
    if (archivo == NULL) {
        ESP_LOGE("ESP32_MENU", "Error al abrir el archivo");
        return;
    }

    // Escribir la estructura en el archivo
    fwrite(&alumno, sizeof(Alumno), 1, archivo);
    fclose(archivo);

    printf("Datos del alumno guardados correctamente.\n");
}

// Función para leer y mostrar los datos de todos los alumnos en SPIFFS
void leerDisco(void) {
    Alumno alumno;

    // Abrir el archivo para lectura
    FILE *archivo = fopen("/spiffs/alumnos.dat", "r");
    if (archivo == NULL) {
        ESP_LOGE("ESP32_MENU", "Error al abrir el archivo");
        return;
    }

    // Leer y mostrar los datos mientras se pueda leer un registro completo
    while (fread(&alumno, sizeof(Alumno), 1, archivo) == 1) {
        printf("Nombre: %s, Edad: %d, Nota: %d\n", alumno.nombre, alumno.edad, alumno.nota);
    }

    fclose(archivo);
}

// Función para salir del programa
void salir(void) {
    printf("\n\nSALIENDO DEL PROGRAMA.\n\n");
    vTaskDelete(NULL); // Eliminar la tarea principal y salir del programa
}

// Tarea para manejar el menú principal
void menu_task(void *pvParameter) {
    int opcion;

    do {
        printf("\n\t*********Menu:**********\n");
        printf("1- Grabar Dato\n");
        printf("2- Leer Disco\n");
        printf("3- Salir\n");
        printf("~~OPCION~~: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1:
                grabarDato();
                break;
            case 2:
                leerDisco();
                break;
            case 3:
                salir();
                break;
            default:
                printf("Opcion no valida.\n");
        }

        // Limpiar el buffer de entrada estándar
        while (getchar() != '\n');

    } while (opcion != 3);

    vTaskDelete(NULL); // Eliminar la tarea del menú al salir
}
