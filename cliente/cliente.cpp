#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <limits>

#define BUFFER_SIZE 1024

class TCPClient
{
private:
    int clientSocket;
    struct sockaddr_in serv_addr;

public:
    TCPClient(const char *serverIP, int serverPort)
    {
        // Crear socket del cliente
        if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Error al crear el socket del cliente");
            exit(EXIT_FAILURE);
        }

        // Configurar la estructura de dirección del servidor
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(serverPort);

        // Convertir la dirección IP de texto a binario
        if (inet_pton(AF_INET, serverIP, &serv_addr.sin_addr) <= 0)
        {
            perror("Dirección IP invalida o no soportada");
            exit(EXIT_FAILURE);
        }

        // Conectar al servidor
        if (connect(clientSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("Error en la conexión con el servidor");
            exit(EXIT_FAILURE);
        }
    }

    void printBoard(char board[6][7])
    {
        // Imprimir el tablero de juego
        std::cout << "_____________" << std::endl;
        for (int i = 0; i < 6; ++i)
        {
            for (int j = 0; j < 7; ++j)
            {
                std::cout << board[i][j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "1|2|3|4|5|6|7" << std::endl;
    }

    void play()
    {
        // Inicializar buffer y tablero de juego
        char buffer[BUFFER_SIZE] = {0};
        char board[6][7] = {{' ', ' ', ' ', ' ', ' ', ' ', ' '},
                            {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                            {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                            {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                            {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                            {' ', ' ', ' ', ' ', ' ', ' ', ' '}};

        // Recibir el primer mensaje del servidor
        recv(clientSocket, buffer, BUFFER_SIZE, 0);

        // Verificar si el cliente comienza el juego
        if (strcmp(buffer, "8") == 0)
        {
            std::cout << "Tú empiezas!\n";
            printBoard(board);
        }
        else
        {
            // El servidor ha comenzado el juego
            int col = atoi(buffer);
            for (int i = 5; i >= 0; --i)
            {
                if (board[i][col] == ' ')
                {
                    board[i][col] = 'S';
                    break;
                }
            }
            std::cout << "El servidor inicia el juego. Movimiento en columna: " << col + 1 << std::endl;
            printBoard(board);
        }

        while (true)
        {
            int col;
            bool validInput = false;

            // Validar la entrada del usuario
            while (!validInput)
            {
                std::cout << "Ingresa el número de columna (1-7): ";
                std::cin >> col;

                if (std::cin.fail() || col < 1 || col > 7)
                {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Entrada inválida. Por favor ingresa un número del 1 al 7.\n";
                }
                else
                {
                    validInput = true;
                }
            }

            col -= 1; // Convertir a índice basado en 0

            // Colocar la ficha del cliente en el tablero
            for (int i = 5; i >= 0; --i)
            {
                if (board[i][col] == ' ')
                {
                    board[i][col] = 'C';
                    break;
                }
            }

            // Enviar movimiento del cliente al servidor
            char message[3];
            snprintf(message, sizeof(message), "%d", col);
            send(clientSocket, message, strlen(message), 0);

            // Recibir respuesta del servidor
            recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (strcmp(buffer, "9") == 0)
            {
                // El cliente ha ganado
                printBoard(board);
                std::cout << "Ganaste!\n";
                break;
            }
            else if (strcmp(buffer, "7") == 0)
            {   
                // El servidor ha ganado
                printBoard(board);
                std::cout << "Perdiste!\n";
                break;
            }
            else
            {
                // El servidor ha realizado un movimiento
                int serverCol = atoi(buffer);
                for (int i = 5; i >= 0; --i)
                {
                    if (board[i][serverCol] == ' ')
                    {
                        board[i][serverCol] = 'S';
                        break;
                    }
                }
                std::cout << "Movimiento del servidor en columna: " << serverCol + 1 << std::endl;
                printBoard(board);
            }
        }
        close(clientSocket); // Cerrar el socket del cliente
    }
};

int main(int argc, char *argv[])
{
    // Verificar si se han proporcionado los argumentos necesarios
    if (argc != 3)
    {
        std::cerr << "Uso: " << argv[0] << " <dirección IP del servidor> <puerto>\n";
        return EXIT_FAILURE;
    }

    const char *serverIP = argv[1];
    int serverPort = std::atoi(argv[2]);

    // Crear una instancia de TCPClient y comenzar a jugar
    TCPClient client(serverIP, serverPort);
    client.play();

    return 0;
}
