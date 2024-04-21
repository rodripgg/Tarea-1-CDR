#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

class TCPClient {
private:
    int clientSocket;
    struct sockaddr_in serv_addr;

public:
    TCPClient(const char* serverIP, int serverPort) {
        if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "Error al crear el socket del cliente" << std::endl;
            exit(EXIT_FAILURE);
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(serverPort);

        if (inet_pton(AF_INET, serverIP, &serv_addr.sin_addr) <= 0) {
            std::cerr << "Dirección no válida/soportada" << std::endl;
            exit(EXIT_FAILURE);
        }

        if (connect(clientSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "Error al conectar al servidor" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void sendMessage(const char* message) {
        send(clientSocket, message, strlen(message), 0);
    }

    void receive(char* buffer, int bufferSize) {
        recv(clientSocket, buffer, bufferSize, 0);
    }

    ~TCPClient() {
        close(clientSocket);
    }
};

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <dirección IP del servidor> <puerto>" << std::endl;
        return EXIT_FAILURE;
    }

    TCPClient client(argv[1], atoi(argv[2]));

    // Implementa aquí la lógica del cliente para interactuar con el servidor

    return 0;
}
