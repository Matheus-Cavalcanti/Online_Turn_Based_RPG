// server.cpp
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <thread>
#include <cstring>      //Para memset
#include <unistd.h>     //Para close()
#include <sys/socket.h> //Para as funções de socket
#include <netinet/in.h> //Para a estrutura sockaddr_in
#include <arpa/inet.h>  //Para inet_ntoa
#include <atomic> // Para o bool atômico
#include <thread>   // Para std::this_thread::sleep_for
#include <chrono>   // Para std::chrono::seconds

#include "server/Player.hpp"
#include "server/Enemy.hpp"
#include "server/Battle.hpp"

#define PORT 8080
#define BUFFER_SIZE 1024

// --- Variáveis Globais para controle do Lobby ---
std::vector<Player*> g_player_lobby;
std::map<int, Player*> g_socket_map;
std::mutex g_lobby_mutex;

// Flag para sincronizar as threads. atomic é seguro para acesso sem mutex.
std::atomic<bool> g_battleInProgress(false);

// FUNÇÃO AUXILIAR: Envia uma pergunta ao cliente e retorna a resposta como string.
std::string requestClientInfo(int client_socket, const std::string& prompt) {
    send(client_socket, prompt.c_str(), prompt.length(), 0);
    char buffer[1024] = {0};
    int bytes_received = recv(client_socket, buffer, 1024, 0);

    if (bytes_received > 0) {
        // Remove quebras de linha da resposta do cliente para limpar a string
        buffer[strcspn(buffer, "\r\n")] = 0;
        return std::string(buffer);
    }
    return ""; // Retorna string vazia se o cliente desconectar
}

void handle_client(int client_socket, int party_size) {
    //std::cout << "Thread iniciada para o cliente " << client_socket << std::endl;

    // =======================================================
    // FASE 1: CRIAÇÃO DE PERSONAGEM
    // =======================================================
    
    std::string playerName = requestClientInfo(client_socket, "Bem-vindo ao RPG! Digite o nome do seu personagem:\n> ");
    if (playerName.empty()) {
        std::cout << "Cliente " << client_socket << " desconectou durante a criação do nome." << std::endl;
        close(client_socket);
        return;
    }

    std::string classPrompt = "\nOlá, " + playerName + "! Escolha sua classe:\n[1] Guerreiro\n[2] Cavaleiro\n> ";
    std::string classChoiceStr = requestClientInfo(client_socket, classPrompt);
    
    Class playerClass = Class::deprived; // Classe padrão caso a escolha seja inválida
    try {
        int choice = std::stoi(classChoiceStr);
        switch (choice) {
            case 1: playerClass = Class::warrior;
            break;

            case 2: playerClass = Class::knight;
            break;

            case 3: playerClass = Class::wanderer;
            break;

            case 4: playerClass = Class::thief;
            break;

            case 5: playerClass = Class::bandit;
            break;
        }
    } catch (const std::exception& e) {
        // Se o cliente digitar algo que não é um número, ele se torna um "Deprived".
    }

    Player* currentPlayer = new Player(playerName, playerClass);
    //std::string creationMsg = "\nPersonagem " + currentPlayer->getName() + " da classe " + currentPlayer->getClass() + " criado com sucesso!\n";
    //send(client_socket, creationMsg.c_str(), creationMsg.length(), 0);


    // =======================================================
    // FASE 2: LÓGICA DE LOBBY
    // =======================================================

    bool isPartyLeader = false;
    bool accepted = false;
    {
        std::lock_guard<std::mutex> lock(g_lobby_mutex);
        
        //Verificação do número de jogadores no lobby
        if (static_cast<int>(g_player_lobby.size()) < party_size) {
            //Se há espaço, o jogador é aceito.
            accepted = true;
            std::cout << "Jogador " << playerName << " aceito no lobby." << std::endl;
            
            g_player_lobby.push_back(currentPlayer);
            g_socket_map[client_socket] = currentPlayer;

            std::string creationMsg = "\nPersonagem " + currentPlayer->getName() + " criado com sucesso!\n";
            std::string waitMsg = "\nAguardando outros jogadores... (" + std::to_string(g_player_lobby.size()) + "/" + std::to_string(party_size) + ")\n";
            send(client_socket, (creationMsg + waitMsg).c_str(), (creationMsg + waitMsg).length(), 0);

            if (static_cast<int>(g_player_lobby.size()) == party_size) {
                isPartyLeader = true;
                g_battleInProgress = true;
            }
        }
    } //Mutex liberado

    if (!accepted) {
        //Se não foi aceito, o lobby estava cheio.
        std::cout << "Conexão rejeitada para " << playerName << ". A sala está cheia." << std::endl;
        const char* msg = "\nA sala está cheia. Tente novamente mais tarde.\n";
        send(client_socket, msg, strlen(msg), 0);
        delete currentPlayer; //Limpa a memória do personagem que não será usado.
        close(client_socket);
        return;
    }

    // O resto da lógica de espera e batalha permanece o mesmo
    if (isPartyLeader) {
        Battle battle(g_player_lobby, g_socket_map);
        battle.run();
        {
            std::lock_guard<std::mutex> lock(g_lobby_mutex);
            for(auto p : g_player_lobby) delete p;
            g_player_lobby.clear();
            g_socket_map.clear();
        }
        g_battleInProgress = false;
    } else {
        while (!g_battleInProgress) {
            if (g_player_lobby.empty()) break;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        while (g_battleInProgress) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    std::cout << "Encerrando conexão para o cliente " << client_socket << "." << std::endl;
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

    std::cout << "Quantos jogadores haverá?" << std::endl;
    int party_s = 0;
    std::cin >> party_s;
    std::cout << "Aguardando " << party_s << " jogadores se conectarem..." << std::endl;

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
        std::thread client_thread(handle_client, client_socket, party_s);
        client_thread.detach();
    }

    //Fecha o socket principal (esta linha nunca será alcançada no loop infinito)
    close(server_fd);
    return 0;
}