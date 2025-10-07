#pragma once

#include <vector>
#include <map>
#include <string>
#include <algorithm> // Para std::sort e std::find_if
#include "Player.hpp"
#include "Enemy.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

enum class BattleState { ONGOING, VICTORY, DEFEAT };

class Battle {
private:
    std::vector<Player*>& _players;      // Referência ao grupo de jogadores
    std::vector<Enemy> _enemies;         // Inimigos pertencentes a esta batalha
    std::map<int, Player*>& _playerSockets; // Mapeia socket_fd -> Player*
    
    std::vector<Character*> _turnOrder;  // Lista de todos os combatentes em ordem de turno
    BattleState _state;

    // Envia uma mensagem para todos os jogadores na batalha
    void broadcast(const std::string& msg) {
        for (auto const& [socket, player] : _playerSockets) {
            send(socket, msg.c_str(), msg.length(), 0);
        }
    }

    static bool compInit(const Character* a, const Character* b) {
        return a->getInit() < b->getInit();
    }

    // Determina a ordem de turno (iniciativa simples baseada no índice)
    void determineTurnOrder() {
        _turnOrder.clear();
        for (auto player : _players) _turnOrder.push_back(player);
        for (auto& enemy : _enemies) _turnOrder.push_back(&enemy);

        std::sort(_turnOrder.begin(), _turnOrder.end(), compInit);
    }

    void processPlayerTurn(Player* activePlayer) {
        // Encontra o socket do jogador ativo
        int playerSocket = -1;
        for (auto const& [socket, player] : _playerSockets) {
            if (player == activePlayer) {
                playerSocket = socket;
                break;
            }
        }
        if (playerSocket == -1) return; // Jogador não encontrado/desconectado

        broadcast("\nÉ o turno de " + activePlayer->getName() + ".\n");
        
        // Apresenta alvos
        std::string targetsMsg = "Escolha um alvo:\n";
        for (size_t i = 0; i < _enemies.size(); ++i) {
            if (_enemies[i].getHp() > 0) {
                targetsMsg += "[" + std::to_string(i + 1) + "] " + _enemies[i].getName() + " (HP: " + std::to_string(_enemies[i].getHp()) + ")\n";
            }
        }
        targetsMsg += "Comando: atacar <numero_do_alvo>\n> ";
        send(playerSocket, targetsMsg.c_str(), targetsMsg.length(), 0);

        // Recebe e processa o comando
        char buffer[1024];
        memset(buffer, 0, 1024);
        recv(playerSocket, buffer, 1024, 0);
        buffer[strcspn(buffer, "\r\n")] = 0;
        std::string command(buffer);

        // Lógica de comando simples (ex: "atacar 1")
        if (command.rfind("atacar ", 0) == 0) {
            size_t targetIndex = std::stoi(command.substr(7)) - 1;
            if (targetIndex < _enemies.size() && _enemies[targetIndex].getHp() > 0) {
                int damage = 10; // Dano fixo
                _enemies[targetIndex].takeDamage(damage);
                broadcast(activePlayer->getName() + " ataca " + _enemies[targetIndex].getName() + " e causa " + std::to_string(damage) + " de dano!\n");
                if (_enemies[targetIndex].getHp() == 0) {
                    broadcast(_enemies[targetIndex].getName() + " foi derrotado!\n");
                }
            } else {
                send(playerSocket, "Alvo inválido.\n", 18, 0);
                processPlayerTurn(activePlayer); // Pede para o jogador tentar de novo
            }
        } else {
             send(playerSocket, "Comando inválido.\n", 19, 0);
             processPlayerTurn(activePlayer); // Tenta de novo
        }
    }

    void processEnemyTurn(Enemy* activeEnemy) {
        broadcast("\nÉ o turno de " + activeEnemy->getName() + ".\n");
        
        // IA Simples: ataca um jogador vivo aleatório
        std::vector<Player*> livingPlayers;
        for(auto p : _players) {
            if(p->getHp() > 0) livingPlayers.push_back(p);
        }

        if(!livingPlayers.empty()) {
            Player* target = livingPlayers[rand() % livingPlayers.size()];
            int damage = activeEnemy->getBaseDamage();
            target->takeDamage(damage);
            broadcast(activeEnemy->getName() + " ataca " + target->getName() + " e causa " + std::to_string(damage) + " de dano!\n");
        }
    }

    void updateBattleState() {
        bool allEnemiesDead = std::all_of(_enemies.begin(), _enemies.end(), [](const Enemy& e){ return e.getHp() == 0; });
        bool allPlayersDead = std::all_of(_players.begin(), _players.end(), [](const Player* p){ return p->getHp() == 0; });

        if (allEnemiesDead) _state = BattleState::VICTORY;
        else if (allPlayersDead) _state = BattleState::DEFEAT;
    }

public:
    Battle(std::vector<Player*>& players, std::map<int, Player*>& playerSockets)
        : _players(players), _playerSockets(playerSockets), _state(BattleState::ONGOING) {
        // Cria inimigos para esta batalha
        _enemies.emplace_back("Goblin Arqueiro", 30, 0, 0, 7); // Usando a versão corrigida do construtor
        _enemies.emplace_back("Ogro da Caverna", 80, 0, 0, 15);
    }

    void run() {
        broadcast("A batalha começou!\n");

        while (_state == BattleState::ONGOING) {
            determineTurnOrder();

            for (Character* combatant : _turnOrder) {
                if (combatant->getHp() <= 0) continue; // Pula turnos de quem já morreu

                // Verifica se é um jogador ou inimigo usando dynamic_cast
                if (Player* player = dynamic_cast<Player*>(combatant)) {
                    processPlayerTurn(player);
                } else if (Enemy* enemy = dynamic_cast<Enemy*>(combatant)) {
                    processEnemyTurn(enemy);
                }
                
                updateBattleState();
                if (_state != BattleState::ONGOING) break;
            }
        }

        if (_state == BattleState::VICTORY) {
            broadcast("\nVITÓRIA! O grupo derrotou todos os inimigos!\n");
        } else {
            broadcast("\nDERROTA! O grupo foi aniquilado...\n");
        }
    }
};