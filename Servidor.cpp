#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 7777
#define BUFFER_SIZE 1024

int main() {
    int serverSocket, newSocket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    
    // Crear socket del servidor
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }
    
    // Configurar opciones del socket
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Error al configurar el socket del servidor");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Enlazar el socket a la dirección y puerto
    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("Error al enlazar el socket del servidor");
        exit(EXIT_FAILURE);
    }
    
    // Escuchar conexiones entrantes
    if (listen(serverSocket, 3) < 0) {
        perror("Error al escuchar conexiones entrantes");
        exit(EXIT_FAILURE);
    }
    
    // Esperar por conexiones y manejarlas
    while (true) {
        if ((newSocket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("Error al aceptar la conexión");
            exit(EXIT_FAILURE);
        }
        
        // Manejar la conexión con un nuevo hilo o proceso
        // Implementa aquí la lógica para manejar el juego
        
        close(newSocket); // Cerrar el socket para esta conexión
    }
    
    return 0;
}
