// client.cpp
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1" //IP local para testes
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    //1. Criar o socket do cliente
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Erro ao criar socket");
        return 1;
    }

    //2. Conectar ao servidor
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    //Converte o endereço de IP de string para o formato binário
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Endereço de IP inválido");
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro de Conexão");
        return 1;
    }

    std::cout << "Conectado ao servidor!" << std::endl;

    //3. Loop de comunicação
    std::string line;
    char buffer[BUFFER_SIZE];

    while (true) {
        std::cout << "Digite uma mensagem (ou 'exit' para sair): ";
        std::getline(std::cin, line);

        if (line == "exit") {
            break;
        }

        //Adiciona o delimitador de nova linha
        line += "\n";

        //Envia a mensagem para o servidor
        send(sock, line.c_str(), line.length(), 0);

        //Zera o buffer e espera pela resposta
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        
        if (bytes_received > 0) {
            std::cout << "Resposta: " << buffer << std::endl;
        } else {
            std::cout << "Servidor desconectado." << std::endl;
            break;
        }
    }

    //Fecha o socket
    close(sock);
    return 0;
}