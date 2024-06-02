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
    static int gameCounter;
    int serverSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

public:
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

        // Configurar socket para reutilizar dirección y puerto
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        {
            perror("Error al configurar el socket del servidor");
            exit(EXIT_FAILURE);
        }

        // Configurar estructura de dirección del servidor
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        // Enlazar socket con la dirección y puerto especificados
        if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            perror("Error al enlazar el socket del servidor");
            exit(EXIT_FAILURE);
        }

        // Configurar socket para escuchar conexiones entrantes
        if (listen(serverSocket, 3) < 0)
        {
            perror("Error al escuchar conexiones entrantes");
            exit(EXIT_FAILURE);
        }
    }

    void handleClient(int clientSocket)
    {
        // Manejar la conexión del cliente
        game(clientSocket);
        close(clientSocket);
    }

    void acceptConnections()
    {
        // Aceptar conexiones de clientes en un bucle infinito
        while (true)
        {
            int newSocket;
            struct sockaddr_in clientAddress;
            socklen_t clientAddrlen = sizeof(clientAddress);

            // Aceptar una nueva conexión
            if ((newSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddrlen)) < 0)
            {
                perror("Error al aceptar la conexion");
                exit(EXIT_FAILURE);
            }
            
            std::cout << "Juego #" << ++gameCounter << " iniciado\n"; // Indicar el número del juego
            // Crear un hilo para manejar la conexión del cliente
            std::thread(&TCPServer::handleClient, this, newSocket).detach();
        }
    }

    ~TCPServer()
    {
        // Cerrar el socket del servidor
        close(serverSocket);
    }
};

int TCPServer::gameCounter = 0;

// busca si hay un ganador, devuelve TRUE o FALSE
bool TCPServer::checkFourInARow(char board[6][7], char symbol)
{
    // Comprobar si hay cuatro fichas consecutivas en una fila
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

    // Comprobar si hay cuatro fichas consecutivas en una columna
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 7; ++j)
        {
            if (board[i][j] == symbol && board[i + 1][j] == symbol && board[i + 2][j] == symbol && board[i + 3][j] == symbol)
            {
                return true;
            }
        }
    }

    // Comprobar si hay cuatro fichas consecutivas en una diagonal ascendente
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

    // Comprobar si hay cuatro fichas consecutivas en una diagonal descendente
    for (int i = 3; i < 6; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (board[i][j] == symbol && board[i - 1][j + 1] == symbol && board[i - 2][j + 2] == symbol && board[i - 3][j + 3] == symbol)
            {
                return true;
            }
        }
    }

    return false;
}

int TCPServer::game(int newSocket)
{   
    // Incrementar el contador de juegos
    int currentGame = gameCounter;
    // Inicializar el tablero de juego vacio
    char board[6][7] = {{' ', ' ', ' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' ', ' ', ' '}};

    // Determinar quien inicia el juego
    srand(time(0));
    bool serverStarts = rand() % 2;

    // Si el servidor comienza, realiza su movimiento y envía la columna al cliente
    if (serverStarts)
    {
        std::cout << "Juego #" << currentGame << ": El servidor inicia\n";
        int serverColumn1;
        // Elegir una columna aleatoria para el primer movimiento del servidor
        while (true)
        {
            serverColumn1 = rand() % 7;
            bool placed = false;
            // Colocar la ficha del servidor en la columna elegida si está vacía
            for (int i = 5; i >= 0; --i)
            {
                if (board[i][serverColumn1] == ' ')
                {
                    board[i][serverColumn1] = 'S';
                    placed = true;
                    break;
                }
            }
            if (placed)
                break;
        }

        // Enviar la columna al cliente
        char response[3];
        snprintf(response, sizeof(response), "%d", serverColumn1);
        send(newSocket, response, strlen(response), 0);
        std::cout << "Juego #" << currentGame << ": Movimiento del servidor en columna: " << serverColumn1 + 1 << std::endl;    
        if (checkFourInARow(board, 'S'))
        {
            std::cout << "Juego #" << currentGame << ": Servidor ha ganado\n";
            send(newSocket, "7", 1, 0);
            return 0;
        }
    }
    else
    {
        // Si el cliente comienza, enviar el código "8" al cliente para indicarlo
        std::cout << "Juego #" << currentGame << ": El cliente inicia\n";
        send(newSocket, "8", 1, 0);
    }

    // Bucle principal del juego
    while (true)
    {
        char buffer[BUFFER_SIZE] = {0};
        // Recibir la columna seleccionada por el cliente
        int valread = recv(newSocket, buffer, BUFFER_SIZE, 0);
        if (valread > 0)
        {
            int clientColumn = atoi(buffer);
            std::cout << "Juego #" << currentGame << ": Movimiento del cliente en columna: " << clientColumn + 1 << std::endl;       
            // Colocar la ficha del cliente en la columna seleccionada
            for (int i = 5; i >= 0; --i)
            {
                if (board[i][clientColumn] == ' ')
                {
                    board[i][clientColumn] = 'C';
                    break;
                }
            }

            // Verificar si el cliente ganó después de su movimiento
            if (checkFourInARow(board, 'C'))
            {   
                std::cout << "Juego #" << currentGame << ": Cliente ha ganado" << std::endl;
                send(newSocket, "9", 1, 0);
                return 0;
            }

            // Realizar el movimiento del servidor
            int serverColumn;
            while (true)
            {
                serverColumn = rand() % 7;
                bool placed = false;
                for (int i = 5; i >= 0; --i)
                {
                    if (board[i][serverColumn] == ' ')
                    {
                        board[i][serverColumn] = 'S';
                        placed = true;
                        break;
                    }
                }
                if (placed)
                    break;
            }

            // Verificar si el servidor ganó después de su movimiento
            if (checkFourInARow(board, 'S'))
            {
            std::cout << "Juego #" << currentGame << ": Servidor ha ganado\n";
                send(newSocket, "7", 1, 0);
                return 0;
            }

            // Enviar la columna seleccionada por el servidor al cliente
            char response[3];
            snprintf(response, sizeof(response), "%d", serverColumn);
            send(newSocket, response, strlen(response), 0);
            std::cout << "Juego #" << currentGame << ": Movimiento del servidor en columna: " << serverColumn + 1 << std::endl;  
        }
        else if (valread == 0)
        {
            // Si no se recibe ninguna información del cliente, el juego termina
            std::cout << "Juego #" << currentGame<< "Terminado" << std::endl;
            break;
        }
        else
        {
            // Si hay un error al recibir el mensaje del cliente, se muestra un mensaje de error
            perror("Error al recibir el mensaje del cliente");
            break;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    // Verificar si se proporciona el puerto como argumento de línea de comandos
    if (argc != 2)
    {
        std::cerr << "Uso: " << argv[0] << " <puerto>\n";
        return EXIT_FAILURE;
    }

    // Obtener el puerto del argumento de línea de comandos
    int port = std::atoi(argv[1]);
    // Crear una instancia de TCPServer
    TCPServer server(port);
    std::cout << "Esperando conexiones...\n";

    // Esperar conexiones entrantes
    server.acceptConnections();

    return 0;
}

