#pragma once
#include "Character.hpp"

// Classe Enemy, herda de Character
class Enemy : public Character {
private:
    std::string _name;
    int _baseDamage;

public:
    Enemy(const std::string& e_name, int health, int mana, int stam, int damage)
        //Chama o construtor da classe base (Character)
        : Character(health, mana, stam), _name(e_name), _baseDamage(damage) {}
 
    std::string getName() const { return _name; }
    int getBaseDamage() const { return _baseDamage; }
    
    //Sobrescreve o método displayStatus para mostrar informações do inimigo
    void displayStatus() const override {
        std::cout << "--- Status do Inimigo: " << _name << " ---" << std::endl;
        Character::displayStatus();
        std::cout << "---------------------------------" << std::endl;
    }
};