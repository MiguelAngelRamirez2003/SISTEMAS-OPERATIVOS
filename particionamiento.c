/*Miguel Angel Ramirez Villani*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PARTICIONES 10
#define LIBRE 0
#define OCUPADA 1

/* Representa una particion de memoria */
typedef struct {
    int id;            /* Identificador de la particion (0..n-1)          */
    int tamano;        /* Tamano en KB de la particion                    */
    int estado;        /* LIBRE u OCUPADA                                 */
    int id_proceso;    /* Identificador del proceso alojado (-1 si LIBRE) */
} Particion;

int puntero_next_fit = 0;

void inicializar_particiones(Particion particiones[], int n) {
    int tamanos[MAX_PARTICIONES] = {100, 500, 200, 300, 600, 150, 400, 250, 350, 120};
    int i;
    for (i = 0; i < n; i++) {
        particiones[i].id = i;
        particiones[i].tamano = tamanos[i];
        particiones[i].estado = LIBRE;
        particiones[i].id_proceso = -1;
    }
}

void mostrar_particiones(Particion particiones[], int n) {
    int i;
    printf("\n----------------------------------------------------------\n");
    printf(" ID | Tamano(KB) | Estado  | Proceso\n");
    printf("----------------------------------------------------------\n");
    for (i = 0; i < n; i++) {
        printf(" %-3d| %-11d| %-8s| %s\n",
               particiones[i].id,
               particiones[i].tamano,
               particiones[i].estado == LIBRE ? "LIBRE" : "OCUPADA",
               particiones[i].estado == OCUPADA ? "" : "-");
        if (particiones[i].estado == OCUPADA) {
            printf("     -> Proceso P%d\n", particiones[i].id_proceso);
        }
    }
    printf("----------------------------------------------------------\n");
}

/*first fit*/
int first_fit(Particion particiones[], int n, int tam_proceso) {
	int i;
	for (i=0; i < n; i++) {
		if (particiones[i].estado == LIBRE && particiones[i].tamano >= tam_proceso){
			return i;
		}
	}
	return -1;
}
/*best fit*/
int best_fit(Particion particiones[], int n, int tam_proceso) {
	int i;
	int mejor_indice = -1;
	int mejor_tamano = 999999;
	
	for (i = 0; i < n; i++) {
		if (particiones[i].estado == LIBRE && particiones[i].tamano >= tam_proceso && particiones[i].tamano < mejor_tamano) {
			mejor_indice = i;
			mejor_tamano = particiones[i].tamano;
		}
	}
	return mejor_indice;
}
/*worst fit*/
int worst_fit(Particion particiones[], int n, int tam_proceso) {
	int i;
	int mejor_indice = -1;
	int mejor_tamano = 0;
	
	for (i = 0; i < n; i++) {
		if (particiones[i].estado == LIBRE && particiones[i].tamano >= tam_proceso && particiones[i].tamano > mejor_tamano) {
			mejor_indice = i;
			mejor_tamano = particiones[i].tamano;
		}
	}
	return mejor_indice;
}
/*next_fit*/
int next_fit(Particion particiones[], int n, int tam_proceso) {
	int i;
	int indice;

	for (i = 0; i < n; i++) {
		indice = (puntero_next_fit + i) %n;
		if (particiones[indice].estado == LIBRE && particiones[indice].tamano >= tam_proceso) {
			puntero_next_fit = (indice +1) % n;
			return indice;
		}
	}
	return -1;
}

void asignar_particion(Particion particiones[], int indice, int id_proceso) {
    particiones[indice].estado = OCUPADA;
    particiones[indice].id_proceso = id_proceso;
}

/*liberar memoria*/
int liberar_particion(Particion particiones[], int n, int id_proceso) {
	int i;
	
	for (i = 0; i < n; i++) {
		if (particiones[i].estado == OCUPADA && particiones[i].id_proceso == id_proceso){
			particiones[i].estado = LIBRE;
			particiones[i].id_proceso = -1;
			return 1;
		}
	}
	return 0;
}

int main() {
    Particion particiones[MAX_PARTICIONES];
    int n = MAX_PARTICIONES;
    int opcion, algoritmo, tam_proceso, id_proceso, indice;
    static int contador_procesos = 1;

    inicializar_particiones(particiones, n);

    do {
        printf("\n============ SIMULADOR DE PARTICIONAMIENTO DE MEMORIA ============\n");
        printf("1. Ver mapa de memoria\n");
        printf("2. Asignar un proceso\n");
        printf("3. Liberar un proceso\n");
        printf("0. Salir\n");
        printf("Seleccione una opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1:
                mostrar_particiones(particiones, n);
                break;

            case 2:
                printf("Tamano del proceso (KB): ");
                scanf("%d", &tam_proceso);

                printf("Algoritmo a usar:\n");
                printf(" 1. First Fit\n 2. Best Fit\n 3. Worst Fit\n 4. Next Fit\n");
                printf("Opcion: ");
                scanf("%d", &algoritmo);

                switch (algoritmo) {
                    case 1: indice = first_fit(particiones, n, tam_proceso); break;
                    case 2: indice = best_fit(particiones, n, tam_proceso);  break;
                    case 3: indice = worst_fit(particiones, n, tam_proceso); break;
                    case 4: indice = next_fit(particiones, n, tam_proceso);  break;
                    default:
                        printf("Algoritmo invalido.\n");
                        indice = -1;
                }

                if (indice == -1) {
                    printf("No hay particion disponible para un proceso de %d KB.\n", tam_proceso);
                } else {
                    id_proceso = contador_procesos++;
                    asignar_particion(particiones, indice, id_proceso);
                    printf("Proceso P%d asignado a la particion %d (%d KB).\n",
                           id_proceso, indice, particiones[indice].tamano);
                }
                break;

            case 3:
                printf("Id del proceso a liberar: ");
                scanf("%d", &id_proceso);
                if (liberar_particion(particiones, n, id_proceso)) {
                    printf("Proceso P%d liberado correctamente.\n", id_proceso);
                } else {
                    printf("No se encontro el proceso P%d.\n", id_proceso);
                }
                break;

            case 0:
                printf("Saliendo del simulador...\n");
                break;

            default:
                printf("Opcion invalida.\n");
        }
    } while (opcion != 0);

    return 0;
}
