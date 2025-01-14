#include <iostream>
#include <cmath>
using namespace std;


struct Card {
    char name;
    char suit;
    short value;
};

struct Player {
    Card hand[3];
    int balance = 0;
};

enum CardValues {
    Ace = 11, King = 10, Queen = 10, Jack = 10, Ten = 10, Nine = 9, Eight = 8, Seven = 7
};

const char SUITS[4] = {'H', 'D', 'S', 'C'};
const char CARD_NAMES[8] = {'A', 'K', 'Q', 'J', 'T', '9', '8', '7'};
const short CARD_VALUES[8] = {Ace, King, Queen, Jack, Ten, Nine, Eight, Seven};

Card deck[32]; // Full deck of 32 cards
int remainingIndices[32]; // Tracks available cards
int remainingCount = 32;  // Tracks the number of remaining cards

void InitializeRemainingIndices() {
    for (int i = 0; i < 32; ++i) {
        remainingIndices[i] = i;
    }
    remainingCount = 32;
}

// Returns a random undealt card
Card* DealCard() {
    if (remainingCount == 0) {
        cout << "No cards remaining to deal!" << endl;
        return nullptr; // Handle edge case if called incorrectly
    }

    int randIndex = rand() % remainingCount; // Randomly pick an index within the remaining range
    int cardIndex = remainingIndices[randIndex];

    // Replace the used index with the last available index
    remainingIndices[randIndex] = remainingIndices[remainingCount - 1];
    --remainingCount;

    return &deck[cardIndex];
}

// Loop for dealing cards
void DealCards(Player * players, const int players_count) {
    for (int p = 0; p < players_count; ++p) {
        cout << "Player " << (p + 1) << " cards:" << endl;
        for (int c = 0; c < 3; ++c) {
            Card* card = DealCard();
            if (card) {
                players[p].hand[c] = *card;
                cout << card->name << card->suit << " ";
            }
        }
        cout << endl;
    }
}

void FillDeck() {
    int index = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 4; ++j) {
            deck[index] = {CARD_NAMES[i], SUITS[j], CARD_VALUES[i]};
            ++index;
        }
    }
}

Player* FillPlayerArray(int players_count) {
    Player* players = new Player[players_count];
    return players;
}

int InputPlayers() {
    int players_count;
    do {
        cout << "Number of players (2-9): ";
        cin >> players_count;
        if (players_count < 2 || players_count > 9) {
            cout << "Invalid input. Please enter a number between 2 and 9." << endl;
        }
    } while (players_count < 2 || players_count > 9);
    return players_count;
}

void NewGame() {
    int players_count = InputPlayers();
    Player* players = FillPlayerArray(players_count);
    FillDeck();
    InitializeRemainingIndices(); // Initialize remaining indices
    DealCards(players, players_count);
    

    delete[] players; // Free dynamically allocated memory
}

int main() {
    NewGame();
    return 0;
}