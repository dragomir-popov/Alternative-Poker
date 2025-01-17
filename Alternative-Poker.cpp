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
void DealCards(Player*& players, const int players_count);
void FillDeck();
Player* CreatePlayerArray(const int players_count);
void GameRound(Player*& players, const int players_count);
bool LoadGame(Player*& players, int& players_count, const char* saveFile);
void SaveGame(Player* players, int players_count, const char* saveFile);
bool FileEmptyOrNonExistent(const char* saveFile);
bool CheckForContinue(Player*& players, int& players_count, const char* saveFile);
void DisplayBalances(Player* players, const int players_count);

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

void DisplayBalances(Player* players, const int players_count) {
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }
    for(int q = 0; q < players_count; ++q) // Display balances
        {
            cout << "Player" << (q+1) << " balance: " << players[q].balance << "; ";
            if((q+1) % 3 == 0) { cout << endl;}
        }
        cout << endl;
}

void PlayersInTheTie(Player* players, const int players_count, const int highestCount, int * highestPlayers) {
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }
    for (int p = 0; p < players_count; ++p) {
            bool isInHighest = false;
            for (int index = 0; index < highestCount; ++index) {
                if (p == highestPlayers[index]) {
                    isInHighest = true;
                    break;
                }
            }
            if (!isInHighest) {
                players[p].isActive = false; // Deactivate players not in the highestPlayers array
            }
        }
}

void InitializeBalances(Player* players, int players_count) {
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }
    for (int i = 0; i < players_count; ++i) {
        players[i].balance = 100 * CHIP_VALUE;
    }
}

// Gives the chip value from each player to the pot
void TakeInitialBet(Player* players, const int players_count) {
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }
    for (int i = 0; i < players_count; ++i) {
        players[i].balance -= CHIP_VALUE;
        pot += CHIP_VALUE;
    }
}

bool Raise(Player& player, int& currentBet, const int raiseAmount) {
    if (raiseAmount < CHIP_VALUE || raiseAmount <= currentBet || raiseAmount > player.balance) {
        cout << "Invalid raise amount." << endl;
        return false;
    }
    player.balance -= raiseAmount;
    pot += raiseAmount;
    currentBet = raiseAmount;
    return true;
}

bool Call(Player& player, int currentBet, const int playerBet) {
    int amountToCall = currentBet - playerBet;

    if (currentBet <= playerBet) {
        cout << "You cannot call because the current bet is not greater than your previous bet." << endl;
        return false;
    }

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
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }

    int currentBet = CHIP_VALUE; // Началният залог
    int* playerBets = new int[players_count](); // Следим индивидуалните залози на играчите
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
                case 1: { // Raise
                    int raiseAmount;
                    bool validRaise = false;
                    while (!validRaise) {
                        cout << "Enter raise amount: ";
                        cin >> raiseAmount;
                        validRaise = Raise(players[i], currentBet, raiseAmount);
                        if (!validRaise) {
                            cout << "Invalid raise amount. Try again." << endl;
                        }
                    }
                    playerBets[i] = currentBet; // Обновяваме залога на играча
                    bettingComplete = false;
                    break;
                }
                case 2: { // Call
                    if (Call(players[i], currentBet, playerBets[i])) {
                        playerBets[i] = currentBet; // Обновяваме залога на играча
                        cout << "Player " << (i + 1) << " calls." << endl;
                    } else {
                        cout << "Player " << (i + 1) << " cannot call. Folding instead." << endl;
                        players[i].isActive = false;
                        --activePlayers;
                    }
                    break;
                }
                case 3: { // Fold
                    cout << "Player " << (i + 1) << " folds." << endl;
                    players[i].isActive = false;
                    --activePlayers;
                    break;
                }
                default: {
                    cout << "Invalid choice. Please try again." << endl;
                    --i; // Върнете текущия играч за повторно въвеждане
                    break;
                }
            }

            if (activePlayers <= 1) {
                bettingComplete = true;
                break;
            }
        }
    }

    delete[] playerBets; // Освобождаваме динамично заделената памет
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

int* CalculateHighestHand(Player* players, const int players_count, int& highestScore, int& highestCount) {
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return nullptr;
    }
    highestScore = 0;
    highestCount = 0;

    // Dynamically allocate an array to store indices of players with the highest score
    int* highestPlayers = new int[players_count];

    // Iterate through the players to find the highest score and the corresponding players
    for (int i = 0; i < players_count; ++i) {
        if (players[i].isActive) {
            int handValue = CalculateHand(players[i]);
            if (handValue > highestScore) {
                // New highest score found, reset the list
                highestScore = handValue;
                highestCount = 1;
                highestPlayers[0] = i; // Start fresh with this player
            } else if (handValue == highestScore) { // Tie for the highest score  
                highestPlayers[highestCount] = i;
                ++highestCount;
            }
        }
    }
    cout << "Highest hand is " << highestScore << endl;
    if(highestCount > 1) {cout << highestCount << " players are tied" << endl;};
    
    return highestPlayers;
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
void DealCards(Player*& players, const int players_count) {
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }
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

// Fills the deck up to the 32 different cards
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

// Returns the valid number of players
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

void SaveGame(Player* players, int players_count, const char* saveFile) {
    if (players == nullptr || saveFile == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }
    ofstream file(saveFile);
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

bool LoadGame(Player*& players, int& players_count, const char* saveFile) {
    if (players == nullptr || saveFile == nullptr) {
        cout << "Nullpointer error" << endl;
        return false;
    }
    ifstream file(saveFile);
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

bool CheckForContinue(Player*& players, int& players_count, const char* saveFile) {
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

bool FileEmptyOrNonExistent(const char* saveFile) {
    if (saveFile == nullptr) {
        cout << "Nullpointer error" << endl;
        return true;
    }
    std::ifstream file(saveFile, std::ios::binary); // Open the file in binary mode
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
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return nullptr;
    }
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

void GameRound(Player*& players, const int players_count) {
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }
    TakeInitialBet( players, players_count); 
    DealCards(players, players_count);
    BettingRound(players, players_count);

    int highestScore, highestCount;
    int* highestPlayers = CalculateHighestHand(players, players_count, highestScore, highestCount);

    while(highestCount != 1) { // TieBreaker
        PlayersInTheTie(players, players_count, highestCount, highestPlayers); // Filter out the players not in the tie
        TakeInitialBet( players, players_count); 
        DealCards(players, players_count);
        BettingRound(players, players_count);
        highestPlayers = CalculateHighestHand(players, players_count, highestScore, highestCount);
    }

    players[highestPlayers[0]].balance += pot;
    pot = 0;
    DisplayBalances(players, players_count);
    
    for (int f = 0; f < players_count; ++f) {
        if(players[f].isActive == true) {
            cout << "Player" << (f+1) << "'s score = " << CalculateHand(players[f]) << endl;
        }
    }
    delete[] highestPlayers;
    for (int i = 0; i < players_count; ++i) {
        if (players[i].balance > 0) {
            players[i].isActive = true;
        }
    }
}

int main() {
    const char saveFile[] = "game_data.txt";
    Player* players = nullptr;
    int players_count = 0;

    // Check for previous game and load players
    if (!CheckForContinue(players, players_count, saveFile)) {
        players_count = InputPlayers();
        players = CreatePlayerArray(players_count);
        FillDeck();
    }
    else{ 
        LoadGame(players, players_count, saveFile);
        FillDeck();
    }
    InitializeBalances(players, players_count);
    InitializeRemainingIndices();
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

        if (playAgain == 'n' || playAgain == 'N') {
            SaveGame(players, players_count, saveFile);
        }
    } while (playAgain == 'y' || playAgain == 'Y');

    delete[] players;
    return 0;
}