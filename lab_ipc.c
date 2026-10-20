#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <errno.h>

#define FIFO_FILE "canal_fifo"
#define BUFFER_SIZE 128

typedef struct {
    int a;
    int b;
} ParNumeros;

typedef struct {
    int a;
    int b;
    int resultado_mul;
    int resultado_sum;
    int fin;
    sem_t espacio_libre;
    sem_t hay_dato;
} DatosCompartidos;

int escribir_par(int fd, const ParNumeros *par) {
    const char *datos = (const char *)par;
    size_t total = 0;

    while (total < sizeof(*par)) {
        ssize_t escritos = write(
            fd,
            datos + total,
            sizeof(*par) - total
        );

        if (escritos < 0) {
            if (errno == EINTR) {
                continue;
            }

            return -1;
        }

        if (escritos == 0) {
            return -1;
        }

        total += (size_t)escritos;
    }

    return 0;
}

int leer_par(int fd, ParNumeros *par) {
    char *datos = (char *)par;
    size_t total = 0;

    while (total < sizeof(*par)) {
        ssize_t recibidos = read(
            fd,
            datos + total,
            sizeof(*par) - total
        );

        if (recibidos < 0) {
            if (errno == EINTR) {
                continue;
            }

            return -1;
        }

        if (recibidos == 0) {
            if (total == 0) {
                return 0;
            }

            errno = EPROTO;
            return -1;
        }

        total += (size_t)recibidos;
    }

    return 1;
}

int main() {
    int shm_id;
    DatosCompartidos *mem;
    pid_t pid_prod, pid_cons1, pid_cons2;

    // Crear memoria compartida
    shm_id = shmget(IPC_PRIVATE, sizeof(DatosCompartidos), IPC_CREAT | 0600);
    if (shm_id < 0) {
        perror("Error creando memoria compartida");
        exit(1);
    }

    mem = (DatosCompartidos *) shmat(shm_id, NULL, 0);
    if (mem == (DatosCompartidos *) -1) {
        perror("Error asociando memoria compartida");
        exit(1);
    }
    
    if (sem_init(&mem->espacio_libre, 1, 1) == -1) {
    	perror("Error al inicializar espacio_libre");
    	shmdt(mem);
    	exit(1);
    }
    
    if (sem_init(&mem->hay_dato, 1, 0) == -1) {
    	perror("Error al inicializar hay_dato");
    	sem_destroy(&mem->espacio_libre);
    	shmdt(mem);
    	exit(1);
    }
    
    mem->fin = 0;

    // Crear FIFO si no existe
    if (mkfifo(FIFO_FILE, 0666) == -1 && errno != EEXIST) {
    	perror("Error creando FIFO");
    	shmdt(mem);
    	shmctl(shm_id, IPC_RMID, NULL);
    	exit(1);
    }

    pid_prod = fork();

    if (pid_prod == 0) {
        // Proceso Productor
        int fd = open(FIFO_FILE, O_WRONLY);
        if (fd < 0) {
            perror("Error abriendo FIFO para escritura");
            exit(1);
        }

        for (int i = 1; i <= 5; i++) {
            ParNumeros par;
            
            par.a = i;
            par.b = i + 2;
            
            if (escribir_par(fd, &par) == -1) {
            	perror("[Productor] Error escribiendo en FIFO");
            	close(fd);
            	exit(1);
            }
            
            printf(
            	"[Productor] Enviado: a=%d, b=%d\n",
            	par.a,
            	par.b
            );
            
            sleep(1);
        }

        close(fd);
        exit(0);
    } else {
        // Crear dos consumidores
        pid_cons1 = fork();
        if (pid_cons1 == 0) {
        	// Reservar memoria dinámica para las multiplicaciones
            	int *resultados_mul = malloc(5 * sizeof(int));

    		if (resultados_mul == NULL) {
        		perror("[Consumidor 1] Error reservando memoria");
        		shmdt(mem);
        		exit(1);
    		}

    		int cantidad_mul = 0;
          // Primer consumidor
          int fd = open(FIFO_FILE, O_RDONLY);
          
          if (fd < 0) {
          	perror("[Consumidor 1] Error abriendo FIFO");
          	exit(1);
          }
          ParNumeros par;
          int estado;
          
          for (int i = 0; i < 5; i++) {
          	estado = leer_par(fd, &par);
          	
          	if (estado != 1) {
          		if (estado == 0) {
          			fprintf(stderr,
          				"[Consumidor 1] Fin inesperado de la FIFO\n");
          		} else {
          			perror("[Consumidor 1] Error leyendo FIFO");
          		}
          		
          		close(fd);
          		exit(1);
          	}
          	
          	// Esperar hasta que la memoria compartida esté disponible
          	while (sem_wait(&mem->espacio_libre) == -1) {
          		if (errno != EINTR) {
          			perror("[Consumidor 1] Error en sem_wait");
          			close(fd);
          			exit(1);
          		}
          	}
          	// Publicar el par en la memoria compartida
          	mem->a = par.a;
          	mem->b = par.b;
          	
          	// Calcular la multiplicación
          	mem->resultado_mul = par.a * par.b;
          	resultados_mul[cantidad_mul] = mem->resultado_mul;
          	cantidad_mul++;
          	
          	printf(
          		"[Consumidor 1] Recibido: a=%d, b=%d\n",
          		par.a, par.b
          	);
          	
          	printf(
          		"[Consumidor 1] Multiplicación: %d x %d = %d\n",
          		par.a, par.b, mem->resultado_mul
          	);
          	
          	// Avisar al Consumidor 2 de que hay datos disponibles
          	if (sem_post(&mem->hay_dato) == -1) {
          		perror("[Consumidor 1] Error en sem_post");
          		close(fd);
          		exit(1);
          	}
          }
          
          close(fd);
          
          // Esperar a que el Consumidor 2 procese el último par
          while (sem_wait(&mem->espacio_libre) == -1) {
          	if (errno != EINTR) {
          		perror("[Consumidor 1] Error esperando el último par");
          		exit(1);
          	}
          }
          
          // Indicar que ya no quedan más datos
          mem->fin = 1;
          
          // Despertar al Consumidor 2 para que pueda terminar
          if (sem_post(&mem->hay_dato) == -1) {
          	perror("[Consumidor 1] Error notificando el final");
          	exit(1);
          }
          
          printf("[Consumidor 1] Resultados almacenados:\n");
          for (int i = 0; i < cantidad_mul; i++) {
          	printf("%d ", resultados_mul[i]);
          }
          
          printf("\n");
          
          //liberar arreglo dinamico
          free(resultados_mul);
          
          //Desasociar la memoria compartida en este proceso
          shmdt(mem);
          
          exit(0);
      }
       else {
            pid_cons2 = fork();
            if (pid_cons2 == 0) {
            	//Reserva memoria dinamica para las sumas
            	int *resultados_sum = malloc(5 * sizeof(int));
            	
            	if (resultados_sum == NULL) {
            		perror("[Consumidor 2] Error reservando memoria");
            		shmdt(mem);
            		exit(1);
            	}
            	
            	int cantidad_sum = 0;
            	
            // Segundo consumidor: calcula suma desde la memoria compartida
            
            for (int i = 0; i < 5; i++) {
            	// Esperar hasta que el Consumidor 1 publique un dato
            	while (sem_wait(&mem->hay_dato) == -1) {
            		if (errno != EINTR) {
            			perror("[Consumidor 2] Error esperando datos");
            			shmdt(mem);
            			exit(1);
            		}
            	}
            	// Verificar si el Consumidor 1 anunció el final
            	if (mem->fin == 1) {
            		//liberar el espacio antes de terminar
            		if (sem_post(&mem->espacio_libre) == -1) {
            			perror("[Consumidor 2] Error liberando espacio al finalizar");
            			shmdt(mem);
            			free(resultados_sum);
            			exit(1);
            		}
            		
            		break;
            	}
            	
            	// Leer los valores publicados en la memoria compartida
            	int a = mem->a;
            	int b = mem->b;
            	
            	// Calcular la suma
            	mem->resultado_sum = a + b;
            	resultados_sum[cantidad_sum] = mem->resultado_sum;
            	cantidad_sum++;
            	printf(
            		"[Consumidor 2] Recibido: a=%d, b=%d\n",
            		a, b
        	);

        	printf(
            		"[Consumidor 2] Suma: %d + %d = %d\n",
            		a, b, mem->resultado_sum
        	);

        	// Avisar al Consumidor 1 que la memoria está libre
        	if (sem_post(&mem->espacio_libre) == -1) {
            		perror("[Consumidor 2] Error liberando espacio");
            		shmdt(mem);
            		exit(1);
        	}
    	}
	printf("[Consumidor 2] Resultados almacenados:\n");
	
	for (int i = 0; i < cantidad_sum; i++) {
		printf("%d ", resultados_sum[i]);
	}
	
	printf("\n");
	
	//liberar arreglo dinamico
	free(resultados_sum);
	
	// Desasociar la memoria compartida
    	shmdt(mem);
    	
    	exit(0);
        }
    }

    // el proceso padre espera a que todos los hijos terminen
    if (waitpid(pid_prod, NULL, 0) == -1) {
        perror("[Padre] Error esperando al Productor");
    }

    if (waitpid(pid_cons1, NULL, 0) == -1) {
        perror("[Padre] Error esperando al Consumidor 1");
    }

    if (waitpid(pid_cons2, NULL, 0) == -1) {
        perror("[Padre] Error esperando al Consumidor 2");
    }

    // Destruir los semáforos cuando ya no los utilizan los hijos
    if (sem_destroy(&mem->espacio_libre) == -1) {
        perror("[Padre] Error destruyendo espacio_libre");
    }

    if (sem_destroy(&mem->hay_dato) == -1) {
        perror("[Padre] Error destruyendo hay_dato");
    }

    // Desasociar la memoria compartida
    if (shmdt(mem) == -1) {
        perror("[Padre] Error desasociando memoria compartida");
    }

    // Eliminar el segmento de memoria compartida
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("[Padre] Error eliminando memoria compartida");
    }

    // Eliminar la FIFO del sistema de archivos
    if (unlink(FIFO_FILE) == -1) {
        perror("[Padre] Error eliminando FIFO");
    }

    printf("[Padre] Todos los procesos terminaron y se liberaron los recursos.\n");

    return 0;
    }
}
