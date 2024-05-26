#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

class TCPServer {
private:
    int serverSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

public:
    // Declaración de la función game
    int game(int newSocket);

    TCPServer(int port) {
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
        address.sin_port = htons(port);

        // Enlazar el socket a la dirección y puerto
        if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("Error al enlazar el socket del servidor");
            exit(EXIT_FAILURE);
        }

        // Escuchar conexiones entrantes
        if (listen(serverSocket, 3) < 0) {
            perror("Error al escuchar conexiones entrantes");
            exit(EXIT_FAILURE);
        }
    }

    void acceptConnections() {
        while (true) {
            int newSocket;
            struct sockaddr_in clientAddress;
            socklen_t clientAddrlen = sizeof(clientAddress);

            // Aceptar la conexión entrante
            if ((newSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddrlen)) < 0) {
                perror("Error al aceptar la conexión");
                exit(EXIT_FAILURE);
            }
            std::cout << "Conexión aceptada\n";

            // Manejar la conexión con un nuevo hilo o proceso

            // Lógica del juego
            game(newSocket);

            close(newSocket); // Cerrar el socket para esta conexión
        }
    }

    ~TCPServer() {
        close(serverSocket);
    }
};

int TCPServer::game(int newSocket) {
    // Definir el tablero
    char board[6][7] = {{' ', ' ', ' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' ', ' ', ' '}};

    // Elección aleatoria para definir quién inicia
    srand(time(0)); // Inicializar la semilla aleatoria
    bool serverStarts = rand() % 2;
    if (serverStarts) {
        std::cout << "El servidor inicia\n";
        // Lógica para que el servidor haga el primer movimiento
        int serverColumn = rand() % 7;

        for (int i = 5; i >= 0; --i) {
            if (board[i][serverColumn] == ' ') {
                board[i][serverColumn] = 'O'; // 'O' representa la ficha del servidor
                break;
            }
        }
        char message[3];
        snprintf(message, sizeof(message), "%d", serverColumn);
        send(newSocket, message, strlen(message), 0);
    } else {
        std::cout << "El cliente inicia\n";
    }

    // Bucle principal para interactuar con el cliente
    while (true) {
        char buffer[BUFFER_SIZE] = {0}; // Inicializar el buffer
        int valread = recv(newSocket, buffer, BUFFER_SIZE, 0); // Recibir mensaje del cliente
        if (valread > 0) {
            int clientColumn = atoi(buffer);
            std::cout << "Movimiento del cliente en columna: " << clientColumn + 1 << std::endl;

            // Colocar la ficha del cliente en la columna seleccionada solo si no está llena
            for (int i = 5; i >= 0; --i) {
                if (board[i][clientColumn] == ' ') {
                    board[i][clientColumn] = 'X'; // 'X' representa la ficha del cliente
                    break;
                }
            }

            // Procesar el movimiento del servidor
            int serverColumn;
            while (true) {
                serverColumn = rand() % 7;
                bool placed = false;
                for (int i = 5; i >= 0; --i) {
                    if (board[i][serverColumn] == ' ') {
                        board[i][serverColumn] = 'O'; // 'O' representa la ficha del servidor
                        placed = true;
                        break;
                    }
                }
                if (placed) break;
            }

            // Enviar el movimiento del servidor de vuelta al cliente
            char response[3];
            snprintf(response, sizeof(response), "%d", serverColumn);
            send(newSocket, response, strlen(response), 0);
            std::cout << "Movimiento del servidor en columna: " << serverColumn + 1 << std::endl;
        } else if (valread == 0) {
            // El cliente ha cerrado la conexión
            std::cout << "Cliente desconectado\n";
            break;
        } else {
            // Error al recibir el mensaje
            perror("Error al recibir el mensaje del cliente");
            break;
        }
    }

    // Lógica del juego
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto>\n";
        return EXIT_FAILURE;
    }

    int port = std::atoi(argv[1]);

    TCPServer server(port);
    server.acceptConnections();

    return 0;
}
