// server.cpp
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <cstring>      //Para memset
#include <unistd.h>     //Para close()
#include <sys/socket.h> //Para as funções de socket
#include <netinet/in.h> //Para a estrutura sockaddr_in
#include <arpa/inet.h>  //Para inet_ntoa

#define PORT 8080
#define BUFFER_SIZE 1024

//Função que será executada por cada thread para lidar com um cliente
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    std::cout << "Thread iniciada para o cliente " << client_socket << std::endl;

    while (true) {
        //Zera o buffer antes de receber novos dados
        memset(buffer, 0, BUFFER_SIZE);

        //recv() é bloqueante: a thread vai esperar aqui até receber dados
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            std::cout << "Cliente " << client_socket << " desconectado." << std::endl;
            break; //Sai do loop se o cliente desconectou ou houve erro
        }

        // Exibe a mensagem recebida no servidor
        std::cout << "Mensagem do cliente " << client_socket << ": " << buffer;

        //Prepara e envia uma resposta (eco)
        std::string response = "Servidor recebeu: ";
        response += buffer;
        send(client_socket, response.c_str(), response.length(), 0);
    }

    //Fecha o socket específico deste cliente
    close(client_socket);
}

int main() {
    //1. Criar o socket do servidor
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Erro ao criar socket");
        return 1;
    }

    //2. Ligar (bind) o socket a um IP e Porta
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Escuta em todas as interfaces de rede
    server_addr.sin_port = htons(PORT); // Converte a porta para o formato de rede

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro de Bind");
        return 1;
    }

    //3. Colocar o socket em modo de escuta (listen)
    if (listen(server_fd, 5) < 0) { // O 5 é o tamanho da fila de conexões pendentes
        perror("Erro de Listen");
        return 1;
    }

    std::cout << "Servidor escutando na porta " << PORT << std::endl;

    //4. Loop infinito para aceitar novas conexões
    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        //accept() é bloqueante: o programa principal espera aqui por um novo cliente
        int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        if (client_socket < 0) {
            perror("Erro de Accept");
            continue; // Continua para a próxima iteração
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "Novo cliente conectado: " << client_ip << " no socket " << client_socket << std::endl;

        //Cria uma nova thread para lidar com este cliente
        //.detach() faz com que a thread rode de forma independente
        std::thread client_thread(handle_client, client_socket);
        client_thread.detach();
    }

    //Fecha o socket principal (este código nunca será alcançado no loop infinito)
    close(server_fd);
    return 0;
}