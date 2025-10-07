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
#define BUFFER_SIZE 2024

int main() {
    // 1. Criação do socket e conexão (continua igual)
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Erro ao criar socket");
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Endereço de IP inválido");
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro de Conexão");
        return 1;
    }

    std::cout << "Conectado ao servidor! Aguarde as instruções." << std::endl;

    // 2. Novo loop de comunicação "reativo"
    char buffer[BUFFER_SIZE];
    while (true) {
        // Zera o buffer e espera por uma mensagem do servidor
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            std::cout << "\nServidor desconectado." << std::endl;
            break;
        }
        
        // Imprime a mensagem recebida do servidor na tela
        std::cout << buffer;
        std::string server_msg(buffer);

        // Verifica se a mensagem do servidor é um prompt que pede uma resposta.
        // Usamos a presença de "> " como nosso indicador.
        if (server_msg.find("> ") != std::string::npos) {
            // Só agora pedimos input ao usuário
            std::string user_input;
            std::getline(std::cin, user_input);

            // E enviamos a resposta para o servidor
            send(sock, user_input.c_str(), user_input.length(), 0);
        }
    }

    close(sock);
    return 0;
}