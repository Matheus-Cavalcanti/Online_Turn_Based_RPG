#pragma once

#include <string>
#include <iostream>

struct MaxStatus {
    int hp;
    int mp;
    int stamina;
};

struct CurrentStatus {
    int hp;
    int mp;
    int stamina;
};

enum LifeStatus {
    alive,
    dead
};

//Classe base para todas as entidades do jogo
class Character {
protected:
    MaxStatus _maxStatus;
    CurrentStatus _currentStatus;
    LifeStatus _lifeStatus;
    int _initiative;

public:
    //Construtores
    Character();

    Character(int health, int mana, int stam) {
        _maxStatus.hp = health;
        _maxStatus.mp = mana;
        _maxStatus.stamina = stam;
        _currentStatus.hp = health;
        _currentStatus.mp = mana;
        _currentStatus.stamina = stam;
        _lifeStatus = alive;
        _initiative = 0;
    }

    //Destrutor virtual
    virtual ~Character() {}

    //Getters e setters dos atributos
    int getMaxHp() const { return _maxStatus.hp; }
    int getMaxMp() const { return _maxStatus.mp; }
    int getMaxStamina() const { return _maxStatus.stamina; }
    int getHp() const { return _currentStatus.hp; }
    int getMp() const { return _currentStatus.mp; }
    int getStamina() const { return _currentStatus.stamina; }
    int getLifeStatus() const { return _lifeStatus; }
    int getInit() const { return _initiative; }
    
    void setMaxHp(int health) { _maxStatus.hp = health; }
    void setMaxMp(int mana) { _maxStatus.mp = mana; }
    void setMaxStamina(int stam) { _maxStatus.stamina = stam; }
    void setHp(int health) { _currentStatus.hp = health; }
    void setMp(int mana) { _currentStatus.mp = mana; }
    void setStamina(int stam) { _currentStatus.stamina = stam; }

    void die() { _lifeStatus = dead; }

    void takeDamage(int damage) { 
        _currentStatus.hp -= damage;
        if (_currentStatus.hp < 0) {
            _currentStatus.hp = 0;
            this->die();
        }
    }

    //Método virtual para exibir o status. Será sobrescrito pelas classes filhas.
    virtual void displayStatus() const {
        std::cout << "HP: " << _currentStatus.hp << "/" << _maxStatus.hp << std::endl;
        std::cout << "MP: " << _currentStatus.mp << "/" << _maxStatus.mp << std::endl;
        std::cout << "Stamina: " << _currentStatus.stamina << "/" << _maxStatus.stamina << std::endl;
    }
};
