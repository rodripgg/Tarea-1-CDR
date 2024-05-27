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
        std::cout << "Conectado al servidor" << std::endl;
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

void printBoard(char board[6][7]) {
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 7; ++j) {
            std::cout << board[i][j] << " ";
        }
        std::cout << std::endl;
    }
}



int main(int argc, char const *argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <dirección IP del servidor> <puerto>" << std::endl;
        return EXIT_FAILURE;
    }

    TCPClient client(argv[1], atoi(argv[2]));


    // Definir el tablero
    char board[6][7] = {{' ', ' ', ' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' ', ' ', ' '}};
                        
    // Bucle principal del juego
    while (true) {
        // Recibir el movimiento del servidor
        char buffer[BUFFER_SIZE] = {0};
        client.receive(buffer, BUFFER_SIZE);
        int serverColumn = atoi(buffer);   


        if (serverColumn == 8) {
            std::cout << "Tu comienzas la partida" << std::endl;
        } else if (serverColumn == 9) {
            std::cout << "¡Ganaste!" << std::endl;
            break;
        } else if (serverColumn == 7) {
            std::cout << "¡Perdiste!" << std::endl;
            break;
        }else{
            std::cout << "Movimiento del servidor en columna: " << serverColumn + 1 << std::endl;
                    // Colocar la ficha del servidor en la columna seleccionada solo si no está llena
            for (int i = 5; i >= 0; --i) {
                if (board[i][serverColumn] == ' ') {
                    board[i][serverColumn] = 'O'; // 'O' representa la ficha del servidor
                    break;
                }
            }// Mostrar el tablero actualizado
            printBoard(board);  
        }

int column;
bool validInput = false;

// Solicitar al usuario que ingrese su movimiento
do {
    std::cout << "Ingrese el número de columna para colocar su ficha (1-7): ";
    if (std::cin >> column) {
        // Verificar si la entrada es un número y está en el rango válido
        if (column >= 1 && column <= 7) {
            validInput = true;
        } else {
            std::cout << "Número de columna inválido. Debe estar entre 1 y 7." << std::endl;
        }
    } else {
        // Limpiar el buffer de entrada
        std::cin.clear();
        while (std::cin.get() != '\n') continue; // Descartar caracteres adicionales en el buffer
        std::cout << "Entrada inválida. Debe ingresar un número." << std::endl;

    }
} while (!validInput);

column--; // Ajustar el índice de la columna



        // Colocar la ficha del jugador en la columna seleccionada solo si no está llena
        for (int i = 5; i >= 0; --i) {
            if (board[i][column] == ' ') {
                board[i][column] = 'X'; // 'X' representa la ficha del jugador
                break;
            }
        }

        // Enviar el movimiento al servidor
        char message[3];
        snprintf(message, sizeof(message), "%d", column);
        client.sendMessage(message);
    }

    return 0;
}
