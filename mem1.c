/*Paso 2 — Definir la estructura del arreglo dinámico*/
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *datos;
    size_t size;      /* elementos actualmente usados */
    size_t capacity;  /* elementos que caben sin pedir más memoria */
} ArregloDinamico;
/*Paso 3 — Función de inicialización (malloc)*/
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
/*Paso 4 — Función para agregar un elemento, creciendo con realloc cuando haga falta*/
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
/*Ejercicio de extension: Punto 1*/
void arreglo_insertar(ArregloDinamico *arr, size_t indice, int valor) {

    /* Verificar que el indice sea valido */
    if (indice > arr->size) {
        printf("Error: indice fuera de rango.\n");
        return;
    }

    /* Si el arreglo esta lleno, aumentar capacidad */
    if (arr->size == arr->capacity) {

        size_t nueva_capacidad = arr->capacity * 2;

        printf("[INFO] Arreglo lleno. Usando realloc para aumentar "
               "la capacidad de %zu a %zu elementos.\n",
               arr->capacity, nueva_capacidad);

        int *tmp = (int *) realloc(
            arr->datos,
            nueva_capacidad * sizeof(int)
        );

        if (tmp == NULL) {
            fprintf(stderr, "Error: realloc no pudo ampliar la memoria.\n");
            return;
        }

        arr->datos = tmp;
        arr->capacity = nueva_capacidad;

        printf("[INFO] Realloc exitoso. Nueva capacidad: %zu elementos.\n",
               arr->capacity);
    }

    /* Desplazar elementos hacia la derecha */
    for (size_t i = arr->size; i > indice; i--) {
        arr->datos[i] = arr->datos[i - 1];
    }

    /* Insertar el nuevo valor */
    arr->datos[indice] = valor;
    arr->size++;

    printf("[INFO] Valor %d insertado en el indice %zu.\n", valor, indice);
}
/*Ejercicio de extension: Punto 2*/
void arreglo_eliminar(ArregloDinamico *arr, size_t indice) {

    /* Verificar que el indice sea valido */
    if (indice >= arr->size) {
        printf("Error: indice fuera de rango.\n");
        return;
    }

    printf("[INFO] Eliminando el elemento %d del indice %zu.\n",
           arr->datos[indice], indice);

    /* Desplazar los elementos posteriores una posicion a la izquierda */
    for (size_t i = indice; i < arr->size - 1; i++) {
        arr->datos[i] = arr->datos[i + 1];
    }

    /* Reducir el numero de elementos */
    arr->size--;

    printf("[INFO] Elemento eliminado. Size actual: %zu, capacidad: %zu.\n",
           arr->size, arr->capacity);

    /*
     * Politica de reduccion:
     * Si size cae por debajo de 1/4 de capacity,
     * reducir la capacidad a la mitad.
     */
    if (arr->size < arr->capacity / 4) {

        size_t nueva_capacidad = arr->capacity / 2;

        /* Evitar que la capacidad llegue a cero */
        if (nueva_capacidad < 1) {
            nueva_capacidad = 1;
        }

        /*
         * La nueva capacidad nunca puede ser menor que size.
         */
        if (nueva_capacidad < arr->size) {
            nueva_capacidad = arr->size;
        }

        printf("[INFO] Size por debajo de 1/4 de la capacidad.\n");
        printf("[INFO] Intentando reducir capacidad de %zu a %zu elementos...\n",
               arr->capacity, nueva_capacidad);

        int *tmp = (int *) realloc(
            arr->datos,
            nueva_capacidad * sizeof(int)
        );

        if (tmp == NULL) {
            /*
             * Si realloc falla al reducir, el bloque original
             * sigue siendo valido.
             */
            fprintf(stderr,
                    "[INFO] realloc no pudo reducir la memoria.\n");
            return;
        }

        arr->datos = tmp;
        arr->capacity = nueva_capacidad;

        printf("[INFO] Reduccion exitosa. Nueva capacidad: %zu elementos.\n",
               arr->capacity);
    }
}
/*Ejercicio de extension: Punto 3*/
int arreglo_buscar(ArregloDinamico *arr, int valor) {

    /* Recorrer todos los elementos del arreglo */
    for (size_t i = 0; i < arr->size; i++) {

        /* Si encontramos el valor, retornar su indice */
        if (arr->datos[i] == valor) {
            return (int)i;
        }
    }

    /* Si no se encontro, retornar -1 */
    return -1;
}
/*Paso 5 — Función de liberación (free)*/
void arreglo_liberar(ArregloDinamico *arr) {
    free(arr->datos);
    arr->datos = NULL;   /* evita punteros colgantes (dangling pointers) */
    arr->size = 0;
    arr->capacity = 0;
    printf("[INFO] Memoria liberada correctamente.\n");
}
/*Paso 6 — Función main de prueba*/
int main(void) {
    ArregloDinamico arr;
    arreglo_iniciar(&arr, 2); /* capacidad inicial deliberadamente pequeña
                                  para forzar varios realloc y observarlos */

    for (int i = 1; i <= 20; i++) {
        arreglo_agregar(&arr, i * 10);
    }

    printf("\nAntes de insertar:\n");
	for (size_t i = 0; i < arr.size; i++) {
    	printf("%d ", arr.datos[i]);
	}
	printf("\n");

	/* Punto 1 - Insertar un elemento */
	arreglo_insertar(&arr, 5, 999);

	printf("\nDespues de insertar 999 en el indice 5:\n");
	for (size_t i = 0; i < arr.size; i++) {
	    printf("%d ", arr.datos[i]);
	}
		printf("\n");
	printf("\nContenido antes de eliminar:\n");

	for (size_t i = 0; i < arr.size; i++) {
    	printf("%d ", arr.datos[i]);
	}

	printf("\n");

	/* Punto 2 - Eliminar elemento */
	/*arreglo_eliminar(&arr, 5);

	printf("\nContenido despues de eliminar el indice 5:\n");

	for (size_t i = 0; i < arr.size; i++) {
    	printf("%d ", arr.datos[i]);
	}

	printf("\n");*/
	/*printf("\n--- PRUEBA DE ELIMINACION ---\n");

	printf("Estado inicial: size = %zu, capacity = %zu\n",
       		arr.size, arr.capacity);

	while (arr.size > 0) {

    	arreglo_eliminar(&arr, arr.size - 1);

    	printf("Estado despues de eliminar: size = %zu, capacity = %zu\n",
           	arr.size, arr.capacity);
	}*/
	/*Punto 3 - Buscar elemento
	printf("\n--- PRUEBA DE BUSQUEDA ---\n");

	int valor_buscado = 999;

	int indice = arreglo_buscar(&arr, valor_buscado);

	if (indice != -1) {
	    printf("El valor %d se encontro en el indice %d.\n",
        	   valor_buscado, indice);
	} else {
	    printf("El valor %d no existe en el arreglo.\n",
        	   valor_buscado);
	}*/

    arreglo_liberar(&arr);
    return 0;
}
