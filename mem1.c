#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *datos;
    size_t size;      /* elementos actualmente usados */
    size_t capacity;  /* elementos que caben sin pedir más memoria */
} ArregloDinamico;

void arreglo_iniciar(ArregloDinamico *arr, size_t capacidad_inicial) {
    arr->datos = (int *) malloc(capacidad_inicial * sizeof(int));
    if (arr->datos == NULL) {
        fprintf(stderr, "Error: malloc no pudo reservar memoria.\n");
        exit(EXIT_FAILURE);
    }
    arr->size = 0;
    arr->capacity = capacidad_inicial;
    printf("[INFO] Arreglo iniciado con capacidad para %zu elementos.\n", capacidad_inicial);
}

void arreglo_agregar(ArregloDinamico *arr, int valor) {
    if (arr->size == arr->capacity) {
        size_t nueva_capacidad = arr->capacity * 2;
        printf("[INFO] Capacidad agotada (%zu/%zu). Solicitando realloc a %zu elementos...\n",
               arr->size, arr->capacity, nueva_capacidad);

        int *tmp = (int *) realloc(arr->datos, nueva_capacidad * sizeof(int));
        if (tmp == NULL) {
            fprintf(stderr, "Error: realloc no pudo ampliar la memoria.\n");
            free(arr->datos);
            exit(EXIT_FAILURE);
        }

        arr->datos = tmp;
        arr->capacity = nueva_capacidad;
        printf("[INFO] Realloc exitoso. Nueva capacidad: %zu elementos.\n", arr->capacity);
    }

    arr->datos[arr->size] = valor;
    arr->size++;
}

void arreglo_liberar(ArregloDinamico *arr) {
    free(arr->datos);
    arr->datos = NULL;   /* evita punteros colgantes (dangling pointers) */
    arr->size = 0;
    arr->capacity = 0;
    printf("[INFO] Memoria liberada correctamente.\n");
}

int main(void) {
    ArregloDinamico arr;
    arreglo_iniciar(&arr, 2); /* capacidad inicial deliberadamente pequeña
                                  para forzar varios realloc y observarlos */

    for (int i = 1; i <= 20; i++) {
        arreglo_agregar(&arr, i * 10);
    }

    printf("\nContenido final del arreglo (%zu elementos, capacidad %zu):\n",
           arr.size, arr.capacity);
    for (size_t i = 0; i < arr.size; i++) {
        printf("%d ", arr.datos[i]);
    }
    printf("\n");

    arreglo_liberar(&arr);
    return 0;
}
