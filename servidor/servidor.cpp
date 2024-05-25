#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>

#define BUFFER_SIZE 1024

class TCPServer {
private:
    int serverSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

public:
    //
    TCPServer(int port) 
    {
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
    }//fin TCPServer

    //Maneja la comunicacion con un cliente
    void handleClient(int clientSocket) 
    {
        char buffer[BUFFER_SIZE] = {0};
        int bytesRead;
        
        // Recibir y enviar mensajes
        while ((bytesRead = read(clientSocket, buffer, BUFFER_SIZE)) > 0) {
            std::cout << "Mensaje recibido: " << buffer << std::endl;
            send(clientSocket, buffer, bytesRead, 0);
            memset(buffer, 0, BUFFER_SIZE);
        }

        if (bytesRead == 0) {
            std::cout << "Cliente desconectado\n";
        } else if (bytesRead < 0) {
            perror("Error al leer del cliente");
        }

        close(clientSocket);
    }//fin handleClient

    //Queda a la escucha
    void acceptConnections() 
    {
        while (true) {
            int newSocket;
            struct sockaddr_in clientAddress;
            socklen_t clientAddrlen = sizeof(clientAddress);

            // Aceptar la conexion entrante
            if ((newSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddrlen)) < 0) {
                perror("Error al aceptar la conexion");
                exit(EXIT_FAILURE);
            }
            std::cout << "Conexion aceptada\n";

            // Manejar la conexion con un nuevo hilo o proceso (Diego)
            std::thread(&TCPServer::handleClient, this, newSocket).detach();


            // Implementar aquí la lógica para manejar el juego (Parra)
            
            // recibir mensaje y enviar mensaje de vuelta con el movimiento del servidor
            

            close(newSocket); // Cerrar el socket para esta conexión
            
            std::cout << "socket cerrado\n";
        }
    }//Fin acceptConnections()

    //cerrar conexión
    ~TCPServer() {
        close(serverSocket);
    }
};


//Esto ejecuta el servidor
int main(int argc, char *argv[]) //   ./ servidor <puerto>
{
    //Si no hay la cantidad de argumentos correcto retornar error
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto>\n";
        return EXIT_FAILURE;
    }
    
    int port = std::atoi(argv[1]); //Convertir <puerto> en integer

    TCPServer server(port); //Abrir conexion
    std::cout << "Conexión abierta\n";

    server.acceptConnections();
    return 0;
}
