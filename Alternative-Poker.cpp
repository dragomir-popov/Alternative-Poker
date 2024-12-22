#include<iostream>
using namespace std;

struct Card{
    char name;
    char suit;
    unsigned short value;
};
enum CardValues{
    Ace = 11, King = 10, Queen = 10, Jack = 10, Ten = 10, Nine = 9, Eight = 8, Seven = 7
};

int InputPlayers();
void NewGame();
const char SUITS[4] = {'H', 'D', 'S', 'C'}; 
const char CARD_NAMES[8] = {'A', 'K', 'Q', 'J', 'T', '9', '8', '7'}; 
const unsigned short CARD_VALUES[8] = {Ace, King, Queen, Jack, Ten, Nine, Eight, Seven};


Card deck[32]; //Масив за цялото тесте

// Инициализиране на пълно тесте
void InitializeDeck() {
    int index = 0;
    for (int i = 0; i < 8; ++i) {       // length of SUITS   
        for (int j = 0; j < 4; ++j) {     // length of CARD_VALUES
            deck[index] = {CARD_NAMES[i], SUITS[j], CARD_VALUES[i]};
            ++index;
            
        }
    }
}
void PrintDeck() {
    for (int i = 0; i < 32; ++i) {
        std::cout << deck[i].name << deck[i].suit << " (value: " << deck[i].value << ")\n";
    }
}

int main()
{
    NewGame();
    PrintDeck();
}

void NewGame(){
    InitializeDeck();
    //InputPlayers();
}

int InputPlayers(){
    int * players_count = new int;
    cout << "Number of players: (2-9)" << endl;
    cin >> *players_count;
    if(*players_count < 2 || *players_count > 9){
        cout << "Number of players should be between 2 and 9!" << endl;

    }
    return *players_count;
}
