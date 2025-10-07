# Online_Turn_Based_RPG
Primeiro trabalho da disciplina Redes de Computadores

Grupo:
Pedro Gasparelo Leme- 14602421
Matheus Cavalcanti de Santana - 13217506
Gabriel Dezejácomo Maruschi - 14571525
Johnatas Luiz dos Santos - 13676388

Desenvolvido e testado no Fedora Linux 41
Compilado no g++ (GCC) 14.3.1 20250808 (Red Hat 14.3.1-3)

Para executar a aplicação:
1. Execute o comando make all (execute make clean e em seguida make all para recompilar caso necessário)
2. Execute o comando make run-server para executar o servidor e escolha o número de jogadores
3. Em outras instâncias do terminal execute make run-client para abrir os clientes
4. Jogue :)

OBS.: O jogo apenas permite a comunicação com servidor e cliente na mesma máquina (e rede). É possível jogar em redes e máquinas diferentes alterando o endereço IP e a porta em client.cpp (porém são necessárias certas configurações a serem feitas no roteador do servidor para garantir que os pacotes serão entregues a ele).
