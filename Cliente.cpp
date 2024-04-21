#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 7777
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <dirección IP del servidor> <puerto>" << std::endl;
        return EXIT_FAILURE;
    }

    int clientSocket;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error al crear el socket del cliente" << std::endl;
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        std::cerr << "Dirección no válida/soportada" << std::endl;
        return EXIT_FAILURE;
    }

    if (connect(clientSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error al conectar al servidor" << std::endl;
        return EXIT_FAILURE;
    }

    // Implementa aquí la lógica del cliente para interactuar con el servidor

    close(clientSocket);
    return 0;
}
