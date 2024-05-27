#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <limits>

#define BUFFER_SIZE 1024

class TCPServer
{
private:
    int serverSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

public:
    // Declaración de la función game
    int game(int newSocket);
    bool checkFourInARow(char board[6][7], char symbol);

    TCPServer(int port)
    {
        // Crear socket del servidor
        if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("Error al crear el socket del servidor");
            exit(EXIT_FAILURE);
        }

        // Configurar opciones del socket
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        {
            perror("Error al configurar el socket del servidor");
            exit(EXIT_FAILURE);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        // Enlazar el socket a la dirección y puerto
        if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            perror("Error al enlazar el socket del servidor");
            exit(EXIT_FAILURE);
        }

        // Escuchar conexiones entrantes
        if (listen(serverSocket, 3) < 0)
        {
            perror("Error al escuchar conexiones entrantes");
            exit(EXIT_FAILURE);
        }
    } // fin TCPServer

    // Maneja la comunicacion con un cliente
    void handleClient(int clientSocket)
    {
        char buffer[BUFFER_SIZE] = {0};
        int bytesRead;

        // Recibir y enviar mensajes
        while ((bytesRead = read(clientSocket, buffer, BUFFER_SIZE)) > 0)
        {
            std::cout << "Mensaje recibido: " << buffer << std::endl;
            send(clientSocket, buffer, bytesRead, 0);
            memset(buffer, 0, BUFFER_SIZE);
        }

        if (bytesRead == 0)
        {
            std::cout << "Cliente desconectado\n";
        }
        else if (bytesRead < 0)
        {
            perror("Error al leer del cliente");
        }

        // close(clientSocket);
    } // fin handleClient

    // Queda a la escucha
    void acceptConnections()
    {
        while (true)
        {
            int newSocket;
            struct sockaddr_in clientAddress;
            socklen_t clientAddrlen = sizeof(clientAddress);

            // Aceptar la conexion entrante
            if ((newSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddrlen)) < 0)
            {
                perror("Error al aceptar la conexion");
                exit(EXIT_FAILURE);
            }
            std::cout << "Conexion aceptada\n";

            // Manejar la conexion con un nuevo hilo o proceso (Diego)
            std::thread(&TCPServer::handleClient, this, newSocket).detach();

            // Lógica del juego
            game(newSocket);

            close(newSocket); // Cerrar el socket para esta conexión

            std::cout << "socket cerrado\n";
        }
    } // Fin acceptConnections()

    // cerrar conexión
    ~TCPServer()
    {
        close(serverSocket);
    }
};

bool TCPServer::checkFourInARow(char board[6][7], char symbol)
{
    std::cout << "Checking for " << symbol << std::endl;
    // Verificar horizontalmente
    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (board[i][j] == symbol && board[i][j + 1] == symbol && board[i][j + 2] == symbol && board[i][j + 3] == symbol)
            {
                return true;
            }
        }
    }

    // Verificar verticalmente
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 7; ++j)
        {
            if (board[i][j] == symbol && board[i + 1][j] == symbol && board[i + 2][j] == symbol && board[i + 3][j] == symbol)
            {
                return true;
            }
        }
    }

    // Verificar diagonales (de izquierda a derecha)
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (board[i][j] == symbol && board[i + 1][j + 1] == symbol && board[i + 2][j + 2] == symbol && board[i + 3][j + 3] == symbol)
            {
                return true;
            }
        }
    }

    // Verificar diagonales (de derecha a izquierda)
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 3; j < 7; ++j)
        {
            if (board[i][j] == symbol && board[i + 1][j - 1] == symbol && board[i + 2][j - 2] == symbol && board[i + 3][j - 3] == symbol)
            {
                return true;
            }
        }
    }

    return false;
}

int TCPServer::game(int newSocket)
{
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
    // Si el servidor inicia, enviar el número de columna aleatorio al cliente
    if (serverStarts)
    {
        std::cout << "El servidor inicia\n";
        // Lógica para que el servidor haga el primer movimiento
        int serverColumn1 = rand() % 7;
        while (true)
        {
            serverColumn1 = rand() % 7;
            bool placed = false;
            for (int i = 5; i >= 0; --i)
            {
                if (board[i][serverColumn1] == ' ')
                {
                    board[i][serverColumn1] = 'O'; // 'O' representa la ficha del servidor
                    placed = true;
                    break;
                }
            }
            if (placed)
                break;
        }

        // Enviar el movimiento del servidor de vuelta al cliente
        char response[3];
        snprintf(response, sizeof(response), "%d", serverColumn1);
        send(newSocket, response, strlen(response), 0);
        std::cout << "Movimiento del servidor en columna: " << serverColumn1 + 1 << std::endl;
    }
    else
    {
        std::cout << "El cliente inicia\n";
        // Enviar un mensaje con un 8 para que el cliente inicie
        send(newSocket, "8", 1, 0);
    }

    // Bucle principal para interactuar con el cliente
    while (true)
    {
        char buffer[BUFFER_SIZE] = {0};                        // Inicializar el buffer
        int valread = recv(newSocket, buffer, BUFFER_SIZE, 0); // Recibir mensaje del cliente
        if (valread > 0)
        {
            int clientColumn = atoi(buffer);
            std::cout << "Movimiento del cliente en columna: " << clientColumn + 1 << std::endl;

            // Colocar la ficha del cliente en la columna seleccionada solo si no está llena
            for (int i = 5; i >= 0; --i)
            {
                if (board[i][clientColumn] == ' ')
                {
                    board[i][clientColumn] = 'X'; // 'X' representa la ficha del cliente
                    break;
                }
            }

            // comprobar si x gana
            bool Xwin = checkFourInARow(board, 'X');
            if (Xwin)
            {
                std::cout << "Cliente ha ganado" << std::endl;
                send(newSocket, "9", 1, 0);
                return 0; // Salir del juego
            }

            // Procesar el movimiento del servidor
            int serverColumn;
            while (true)
            {
                serverColumn = rand() % 7;
                bool placed = false;
                for (int i = 5; i >= 0; --i)
                {
                    if (board[i][serverColumn] == ' ')
                    {
                        board[i][serverColumn] = 'O'; // 'O' representa la ficha del servidor
                        placed = true;
                        break;
                    }
                }
                if (placed)
                    break;
            }

            // comprobar si o gana
            bool Owin = checkFourInARow(board, 'O');
            if (Owin)
            {
                std::cout << "Servidor ha ganado" << std::endl;
                send(newSocket, "7", 1, 0);
                return 0; // Salir del juego
            }

            // Enviar el movimiento del servidor de vuelta al cliente
            char response[3];
            snprintf(response, sizeof(response), "%d", serverColumn);
            send(newSocket, response, strlen(response), 0);
            std::cout << "Movimiento del servidor en columna: " << serverColumn + 1 << std::endl;
        }
        else if (valread == 0)
        {
            // El cliente ha cerrado la conexión
            std::cout << "Cliente desconectado\n";
            break;
        }
        else
        {
            // Error al recibir el mensaje
            perror("Error al recibir el mensaje del cliente");
            break;
        }
    }
    // Lógica del juego
    return 0;
}

// Esto ejecuta el servidor
int main(int argc, char *argv[]) //   ./ servidor <puerto>
{
    // Si no hay la cantidad de argumentos correcto retornar error
    if (argc != 2)
    {
        std::cerr << "Uso: " << argv[0] << " <puerto>\n";
        return EXIT_FAILURE;
    }

    int port = std::atoi(argv[1]); // Convertir <puerto> en integer

    TCPServer server(port); // Abrir conexion
    std::cout << "Conexión abierta\n";

    server.acceptConnections();

    return 0;
}
