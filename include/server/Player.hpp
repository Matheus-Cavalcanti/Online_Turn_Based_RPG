#pragma once
#include "Character.hpp"

//Estrutura para os atributos do jogador, similar a Dark Souls
struct Attributes {
    int vitality;
    int attunement;
    int endurance;
    int strength;
    int dexterity;
    int resistance;
    int intelligence;
    int faith;
};

enum Class {
    warrior,
    knight,
    wanderer,
    thief,
    bandit,
    hunter,
    sorcerer,
    pyromancer,
    cleric,
    deprived
};

//Classe Player, herda de Character
class Player : public Character {
private:
    std::string _name;
    Class _class;
    int _level;
    int _xp;
    Attributes _attributes;

    //Função auxiliar estática para obter os atributos antes de construir o objeto
    static Attributes getAttributesForClass(Class p_class) {
        switch (p_class) {
            case warrior: return {11, 8, 12, 13, 13, 11, 9, 9};
            case knight:  return {14, 10, 10, 11, 11, 10, 9, 11};
            case wanderer:return {10, 11, 10, 10, 14, 12, 11, 8};
            // ... outras classes
            default:      return {11, 11, 11, 11, 11, 11, 11, 11}; // Deprived
        }
    }

public:
    Player(const std::string& p_name, const Class p_class)
        // 1. Primeiro, inicializa a classe base. Chamamos a função auxiliar
        //    para obter os valores necessários (vitalidade, etc).
        : Character(
            getAttributesForClass(p_class).vitality * 10,   // HP
            50,                                             // MP (exemplo)
            getAttributesForClass(p_class).endurance * 10   // Stamina
          ),
        // 2. Agora, inicializa os membros da classe Player.
          _name(p_name),
          _class(p_class),
          _level(1),
          _xp(0),
          _attributes(getAttributesForClass(p_class)) // Chamamos a função de novo aqui
    {
        // O corpo do construtor continua o mesmo
        _initiative = (_attributes.dexterity * 2) / _attributes.endurance;
    }

    //Getters e setters dos atributos
    std::string getName() const { return _name; }
    std::string getClass() const { 
        switch (_class)
        {
        case warrior:
            return "warrior";
            break;
        case knight:
            return "knight";
        case wanderer:
            return "wanderer";
        //Todo: fazer demais classes
        default:
            return "deprived";
            break;
        }
    }
    //Por equanto sem demais getters e setters de atributos,level e xp para simplificar

    //Sobrescreve o método displayStatus para mostrar informações do jogador
    void displayStatus() const override {
        std::cout << "--- Status de " << _name << " ---" << std::endl;
        std::cout << "Level: " << _level << " (" << _xp << " XP)" << std::endl;
        Character::displayStatus(); // Chama a versão da classe base
        std::cout << "-------------------------" << std::endl;
    }
};
