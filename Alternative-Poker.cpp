#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>
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
    bool isActive = true;
};

void InitializeBalances(Player* players, int players_count);
void TakeInitialBet(Player* players, int players_count);
bool Raise(Player& player, int& currentBet, int raiseAmount);
bool Call(Player& player, int currentBet, int playerBet);
void Fold(Player& player);
void BettingRound(Player* players, int players_count);
int CalculateHand(Player& player);
void InitializeRemainingIndices();
Card* DealCard();
void DealCards(Player * players, const int players_count);
void FillDeck();
Player* CreatePlayerArray(const int players_count);
void GameRound(Player* players, int players_count);
bool LoadGame(Player*& players, int& players_count, const char* filename);
void SaveGame(Player* players, int players_count, const char* filename);
bool FileEmptyOrNonExistent(const char* filename);
bool CheckForContinue(Player*& players, int& players_count, const char* filename);

enum CardValues {
    Ace = 11, King = 10, Queen = 10, Jack = 10, Ten = 10, Nine = 9, Eight = 8, Seven = 7
};

const char SUITS[4] = {'H', 'D', 'S', 'C'};
const char CARD_NAMES[8] = {'A', 'K', 'Q', 'J', 'T', '9', '8', '7'};
const int CARD_VALUES[8] = {Ace, King, Queen, Jack, Ten, Nine, Eight, Seven};

Card deck[32]; // Full deck of 32 cards
int remainingIndices[32]; // Tracks available cards
int remainingCount = 32;  // Tracks the number of remaining cards

int pot = 0;
const int CHIP_VALUE = 10;

void InitializeBalances(Player* players, int players_count) {
    for (int i = 0; i < players_count; ++i) {
        players[i].balance = 100 * CHIP_VALUE;
    }
}

void TakeInitialBet(Player* players, int players_count) {
    for (int i = 0; i < players_count; ++i) {
        players[i].balance -= CHIP_VALUE;
        pot += CHIP_VALUE;
    }
}

bool Raise(Player& player, int& currentBet, int raiseAmount) {
    if (raiseAmount < CHIP_VALUE || raiseAmount <= currentBet || raiseAmount > player.balance) {
        cout << "Invalid raise amount." << endl;
        return false;
    }
    player.balance -= raiseAmount;
    pot += raiseAmount;
    currentBet = raiseAmount;
    return true;
}

bool Call(Player& player, int currentBet, int playerBet) {
    int amountToCall = currentBet - playerBet;
    if (amountToCall > player.balance) {
        cout << "Not enough balance to call." << endl;
        return false;
    }
    player.balance -= amountToCall;
    pot += amountToCall;
    return true;
}

void Fold(Player& player) {
    player.isActive = false;
    cout << "Player folded." << endl;
}

void BettingRound(Player* players, int players_count) {
    if(players == nullptr) return;
    int currentBet = CHIP_VALUE;
    int activePlayers = players_count;
    bool bettingComplete = false;

    while (!bettingComplete) {
        bettingComplete = true;
        for (int i = 0; i < players_count; ++i) {
            if (!players[i].isActive) continue;

            cout << "Player " << (i + 1) << "'s turn. Current bet: " << currentBet << endl;
            cout << "Options: 1) Raise  2) Call  3) Fold" << endl;
            int choice;
            cin >> choice;

            switch (choice) {
                case 1: {
                    int raiseAmount;
                    cout << "Enter raise amount: ";
                    cin >> raiseAmount;
                    if (Raise(players[i], currentBet, raiseAmount)) {
                        bettingComplete = false;
                    }
                    break;
                }
                case 2:
                    if (!Call(players[i], currentBet, 0)) { // 0 for initial player bet
                        players[i].isActive = false;
                    }
                    break;
                case 3:
                    Fold(players[i]);
                    --activePlayers;
                    break;
                default:
                    cout << "Invalid choice. Try again." << endl;
                    break;
            }

            if (activePlayers == 1) {
                cout << "Betting round ends with one active player." << endl;
                return;
            }
        }
    }
}

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

Player* CreatePlayerArray(const int players_count) {
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

void SaveGame(Player* players, int players_count, const char* filename) {
    ofstream file(filename);
    if (!file) {
        cout << "Error: Unable to save game data." << endl;
        return;
    }
    file << players_count << endl;
    for (int i = 0; i < players_count; ++i) {
        file << players[i].balance << endl;
    }
    file.close();
}

bool LoadGame(Player*& players, int& players_count, const char* filename) {
    ifstream file(filename);
    if (!file || file.peek() == ifstream::traits_type::eof()) {
        return false; // No previous game data available
    }
    file >> players_count;
    players = new Player[players_count];
    for (int i = 0; i < players_count; ++i) {
        file >> players[i].balance;
    }
    file.close();
    return true;
}

bool CheckForContinue(Player*& players, int& players_count, const char* filename) {
    if(filename == nullptr)return false;
    if(!FileEmptyOrNonExistent){
        cout << "Previous game found. Do you want to continue? (Y/N): ";
        char response;
        cin >> response;

        if (response == 'y' || response == 'Y') {
            return true;
        }
    }
    return false;
}

bool FileEmptyOrNonExistent(const char* filename) {
    std::ifstream file(filename, std::ios::binary); // Open the file in binary mode
    if (!file.is_open()) {
        // File doesn't exist
        return true;
    }

    // Move the file pointer to the end and check its position
    file.seekg(0, std::ios::end);
    if (file.tellg() == 0) {
        // File is empty
        return true;
    }

    return false; // File exists and is not empty
}

Player* FilterActivePlayers(Player* players, int& players_count) {
    int activeCount = 0;
    for (int i = 0; i < players_count; ++i) {
        if (players[i].balance > 0) {
            ++activeCount;
        }
    }

    Player* activePlayers = new Player[activeCount];
    int index = 0;
    for (int i = 0; i < players_count; ++i) {
        if (players[i].balance > 0) {
            activePlayers[index++] = players[i];
        }
    }
    players_count = activeCount;
    delete[] players; // Release old array
    return activePlayers;
}

void GameRound(Player* players, const int players_count) {
    InitializeBalances(players, players_count);
    InitializeRemainingIndices();
    TakeInitialBet( players, players_count);
    DealCards(players, players_count);
    BettingRound(players, players_count);

    for (int i = 0; i < players_count; ++i) {
        if (players[i].isActive) {
            cout << "Player " << (i + 1) << " score: " << CalculateHand(players[i]) << endl;
        }
    }
}

int main() {
    const char saveFile[] = "game_data.txt";
    Player* players = nullptr;
    int players_count;

    // Check for previous game
    if (!CheckForContinue(players, players_count, saveFile)) {
        players_count = InputPlayers();
        players = CreatePlayerArray(players_count);
    }
    else{ //Read data from file

    }
    
    //Main loop
    char playAgain;
    do {
        players = FilterActivePlayers(players, players_count);

        if (players_count == 0) {
            cout << "No players with positive balance. Game over!" << endl;
            break;
        }
        GameRound(players, players_count);
        cout << "Game round completed." << endl;
        cout << "Do you want to play again? (y/n): ";
        cin >> playAgain;

        if (playAgain == 'n') {
            SaveGame(players, players_count, saveFile);
        }
    } while (playAgain == 'y');

    delete[] players;
    return 0;
}