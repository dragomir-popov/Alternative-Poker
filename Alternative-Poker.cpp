#include <iostream>
#include <cmath>
#include <ctime>
using namespace std;

struct Card {
    char name;
    char suit;
    int value;
};

struct Player {
    Card hand[3];
    int balance = 0;
    int score = 0;
};

enum CardValues {
    Ace = 11, King = 10, Queen = 10, Jack = 10, Ten = 10, Nine = 9, Eight = 8, Seven = 7
};

const char SUITS[4] = {'H', 'D', 'S', 'C'};
const char CARD_NAMES[8] = {'A', 'K', 'Q', 'J', 'T', '9', '8', '7'};
const int CARD_VALUES[8] = {Ace, King, Queen, Jack, Ten, Nine, Eight, Seven};

Card deck[32]; // Full deck of 32 cards
int remainingIndices[32]; // Tracks available cards
int remainingCount = 32;  // Tracks the number of remaining cards

int CalculateHand(Player& player) {
    Card* hand = player.hand;
    int score = 0;

    // Check for `7S` (7 of Spades)
    bool has7S = false;
    for (int i = 0; i < 3; ++i) {
        if (hand[i].name == '7' && hand[i].suit == 'S') {
            has7S = true;
            break;
        }
    }

    // Case 1: Three identical cards
    if (hand[0].name == hand[1].name && hand[1].name == hand[2].name) {
        if (hand[0].name == '7') {
            score = 34; // Special case: three 7s
        } else {
            score = 3 * hand[0].value;
        }
    }
    // Case 2: `7S` with two identical cards
    else if (has7S && (hand[0].name == hand[1].name || hand[1].name == hand[2].name || hand[0].name == hand[2].name)) {
        char identicalCard = hand[0].name == hand[1].name ? hand[0].name : hand[2].name;
        int identicalCardValue = 0;

        // Find the value of the identical card
        for (int i = 0; i < 3; ++i) {
            if (hand[i].name == identicalCard) {
                identicalCardValue = hand[i].value;
                break;
            }
        }

        score = 2 * identicalCardValue + 11; // Rule for `7S` + two identical cards
    }
    // Case 3: All cards of the same suit (Flush)
    else if (hand[0].suit == hand[1].suit && hand[1].suit == hand[2].suit) {
        score = hand[0].value + hand[1].value + hand[2].value;
    }
    // Case 4: `7S` with two cards of the same suit
    else if (has7S && (hand[0].suit == hand[1].suit || hand[1].suit == hand[2].suit || hand[0].suit == hand[2].suit)) {
        int suitCardValue = 0;

        for (int i = 0; i < 3; ++i) {
            if (hand[i].suit == 'S') continue; // Ignore `7S`
            suitCardValue += hand[i].value;
        }

        score = suitCardValue + 11; // Rule for `7S` + two same-suit cards
    }
    // Case 5: `7S` with unrelated cards
    else if (has7S) {
        int highestValue = 0;
        for (int i = 0; i < 3; ++i) {
            if (hand[i].suit == 'S' && hand[i].name == '7') continue; // Ignore `7S`
            highestValue = std::max(highestValue, static_cast<int>(hand[i].value));
        }

        score = highestValue + 11;
    }
    // Case 6: No combinations
    else {
        int highestValue = 0;
        for (int i = 0; i < 3; ++i) {
            highestValue = std::max(highestValue, static_cast<int>(hand[i].value));
        }

        score = highestValue;
    }

    player.score = score; // Assign the calculated score to the player's balance
    return score;
}

void InitializeRemainingIndices() {
    srand(static_cast<unsigned>(time(0))); // ensures randomness every time the program is run
    // Fill the remainingIndices array with indices from 0 to 31
    for (int i = 0; i < 32; ++i) {
        remainingIndices[i] = i;
    }

    // Shuffle the remainingIndices array to randomize the deck
    for (int i = 0; i < 32; ++i) {
        int randIndex = rand() % 32; // Random index
        int temp = remainingIndices[i];
        remainingIndices[i] = remainingIndices[randIndex];
        remainingIndices[randIndex] = temp;
    }

    remainingCount = 32; // Reset remaining count
}

// Returns a random undealt card
Card* DealCard() {
    if (remainingCount == 0) {
        cout << "No cards remaining to deal!" << endl;
        return nullptr;
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

Player* FillPlayerArray(const int players_count) {
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
    for(int i = 0; i < players_count; i++)
    {
        cout << "Player " << (i+1) << " score: " << CalculateHand(players[i]) << endl;
    }
    
    delete[] players; // Free dynamically allocated memory
}

int main() {
    NewGame();
    return 0;
}