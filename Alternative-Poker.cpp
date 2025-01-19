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
void Fold(Player& player, int &activePlayers);
void BettingRound(Player* players, const int players_count, int currentBet);
int CalculateHand(Player& player);
void InitializeRemainingIndices();
Card* DealCard();
void DealCards(Player*& players, const int players_count);
void FillDeck();
Player* CreatePlayerArray(const int players_count);
void GameRound(Player*& players, const int players_count);
void LoadGame(Player*& players, int& players_count, const char* saveFile);
void SaveGame(Player* players, int players_count, const char* saveFile);
bool FileEmptyOrNonExistent(const char* saveFile);
bool CheckForContinue(Player*& players, int& players_count, const char* saveFile);
void DisplayBalances(Player* players, const int players_count);
void PlayersInTheTie(Player* players, const int players_count, const int highestCount, int * highestPlayers);
void JoinTie(Player* players, const int players_count);
void ZeroBalanceInTie(Player* players, const int players_count);

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
int activePlayers = 0;

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

void JoinTie(Player* players, const int players_count) {
    int addToPot = 0;
    int halfPot = (pot/2);
    for(int i = 0; i < players_count; i++) {
        char answer;
        if(players[i].isActive == false) {
            cout << "Player" << (i+1) << " pay " << halfPot << " to join the tie? Y/N:" << endl;
            cin >> answer;

            if(answer == 'y' || answer == 'Y') {
                players[i].balance -= halfPot;
                addToPot += halfPot;
                players[i].isActive = true;
            }
        }
    }
    pot += addToPot;
}

void ZeroBalanceInTie(Player* players, const int players_count) {
    for(int i = 0; i < players_count; ++i) {
        if(players[i].isActive && players[i].balance < 1) {
            players[i].balance += 50;
        }
    }
    return;
}

// Deactivate players whose hands are not tied for the win
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
            players[p].isActive = false; 
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

void Fold(Player& player, int &activePlayers) {
    player.isActive = false;
    --activePlayers;
    cout << "Player folded." << endl;
}

// The players give their bets
void BettingRound(Player* players, const int players_count, int currentBet) {
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }
    
    int* playerBets = new int[players_count](); // Track individual bets
    bool* hasActed = new bool[players_count](); // Track if a player has acted this round
    bool bettingComplete = false;

    while (!bettingComplete) {
        bettingComplete = true;

        for (int i = 0; i < players_count; ++i) {
            if (activePlayers < 2) { // End betting round if fewer than 2 active players
                bettingComplete = true;
                break;
            }
            if (!players[i].isActive || hasActed[i]) continue; // Skip inactive or already acted players

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
                    playerBets[i] = currentBet; // Update player bet
                    bettingComplete = false; // Restart round
                    // Reset all players' actions to false except the current player
                    for (int j = 0; j < players_count; ++j) {
                        hasActed[j] = (j == i);
                    }
                    break;
                }
                case 2: { // Call
                    if (Call(players[i], currentBet, playerBets[i])) {
                        playerBets[i] = currentBet;
                    } else {
                        cout << "Player " << (i + 1) << " cannot call. Folding instead." << endl;
                        Fold(players[i], activePlayers);
                    }
                    hasActed[i] = true; 
                    break;
                }
                case 3: { // Fold
                    Fold(players[i], activePlayers);
                    hasActed[i] = true; 
                    break;
                }
                default: {
                    cout << "Invalid choice. Please try again." << endl;
                    --i; // Retry this player
                    break;
                }
            }

            if (activePlayers < 2) { // If only one player is left active, end the betting round immediately
                bettingComplete = true;
                break;
            }
        }
        // Ensure the loop only continues if all active players match the current bet
        if (activePlayers > 1) { 
            for (int i = 0; i < players_count; ++i) {
                if (players[i].isActive && playerBets[i] < currentBet) {
                    bettingComplete = false;
                    break;
                }
            }
        }
    }
    delete[] playerBets; // Free allocated memory
    delete[] hasActed;
}

bool GameIsOver(Player* players, const int players_count) {
    int remainingPlayers = 0;
    for(int i = 0; i < players_count; ++i) {
        if(players[i].balance > 0) {
            remainingPlayers++;
        }
    }
    if (remainingPlayers < 2) {
        return true;
    }
    return false;
}

int CalculateHand(Player& player) {
    Card* hand = player.hand;
    int score = 0;

    // Check for 7S
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
    // Case 2: 7S with two identical cards
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

        score = 2 * identicalCardValue + 11; // Rule for 7S + two identical cards
    }
    // Case 3: All cards of the same suit
    else if (hand[0].suit == hand[1].suit && hand[1].suit == hand[2].suit) {
        score = hand[0].value + hand[1].value + hand[2].value;
    }
    // Case 4: 7S with two cards of the same suit
    else if (has7S && (hand[0].suit == hand[1].suit || hand[1].suit == hand[2].suit || hand[0].suit == hand[2].suit)) {
        int suitCardValue = 0;

        for (int i = 0; i < 3; ++i) {
            if (hand[i].suit == 'S') continue; // Ignores 7S
            suitCardValue += hand[i].value;
        }

        score = suitCardValue + 11; // Rule for 7S + two same-suit cards
    }
    // Case 5: 7S with unrelated cards
    else if (has7S) {
        int highestValue = 0;
        for (int i = 0; i < 3; ++i) {
            if (hand[i].suit == 'S' && hand[i].name == '7') continue; // Ignore 7S
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

    player.score = score;
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
    srand(static_cast<unsigned>(time(0))); // ensures randomness every time the program is ran
    // Fills the remainingIndices array with indices from 0 to 31
    for (int i = 0; i < 32; ++i) {
        remainingIndices[i] = i;
    }

    // Shuffles the remainingIndices array to randomize the deck
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
    if (remainingCount == 0) { // no more cards to deal
        cout << "No more cards left" << endl;
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
        cout << "Nullpointer error - players" << endl;
        return;
    }
    for (int p = 0; p < players_count; ++p) {
        if(players[p].isActive == true) {
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

Player* CreatePlayerArray(const int players_count) { // Fills the players array with fresh players
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

void ClearFile(const char* filename) {
    std::ofstream file(filename, std::ofstream::trunc); // Opens the file in truncate mode which clears the text
    if (file.is_open()) {
        file.close();
    } else {
        std::cerr << "Failed to open the file.\n";
    }
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

// Reads the number of players and their balances from the saveFile
void LoadGame(Player*& players, int& players_count, const char* saveFile) {
    if (saveFile == nullptr) {
        cout << "Nullpointer error" << endl;

        return;
    }
    ifstream file(saveFile);
    if (!file || file.peek() == ifstream::traits_type::eof()) {
        cout << "File is empty" << endl;
        return; // No previous game data available
    }
    file >> players_count;
    players = new Player[players_count];
    for (int i = 0; i < players_count; ++i) {
        file >> players[i].balance;
    }
    file.close();
    return;
}

bool CheckForContinue(Player*& players, int& players_count, const char* saveFile) {
    if(!(FileEmptyOrNonExistent(saveFile))){
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

// Returns only the players still in the game
void FilterActivePlayers(Player* players, int& players_count) { // should be void
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }
    for (int i = 0; i < players_count; ++i) {
        if (players[i].balance > 0) { 
            ++activePlayers; // How many players are still in the game
            continue;
        }
        else {
            players[i].isActive = false;
        }
    }
    return; 
}

void GameRound(Player*& players, const int players_count) {
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }
    activePlayers = players_count;
    int currentBet = CHIP_VALUE; // Starting bet
    TakeInitialBet( players, players_count); 
    DealCards(players, players_count);
    BettingRound(players, players_count, currentBet);

    int highestScore, highestCount;
    int* highestPlayers = CalculateHighestHand(players, players_count, highestScore, highestCount);

    // TieBreaker
    while(highestCount != 1) { 
        PlayersInTheTie(players, players_count, highestCount, highestPlayers); // Filter out the players not in the tie
        JoinTie(players, players_count);
        ZeroBalanceInTie(players, players_count); // If a player has 0 balance upon joining the tie, gives 50pts
        InitializeRemainingIndices(); // randomize and refill the deck
        DealCards(players, players_count);
        BettingRound(players, players_count, currentBet);
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

void NewGame(Player** players, int &players_count) {
    players_count = InputPlayers();
    *players = CreatePlayerArray(players_count);
    InitializeBalances(*players, players_count);
    FillDeck();
}

int main() {
    const char saveFile[] = "game_data.txt";
    Player* players = nullptr;
    int players_count = 0;

    if (!CheckForContinue(players, players_count, saveFile)) { // New Game (make a new function)
        NewGame(&players, players_count);
    }
    else{ // previous game found -> continue
        LoadGame(players, players_count, saveFile);
        FillDeck();
    }
    
    char playAgain;
    do {
        InitializeRemainingIndices();
        FilterActivePlayers(players, players_count);
        if (players_count == 0) {
            cout << "No players with positive balance. Game over!" << endl;
            break;
        }
        GameRound(players, players_count);
        activePlayers = 0; // Resets the active players
        cout << "Game round completed." << endl;
        if (GameIsOver(players, players_count)) {
            ClearFile(saveFile);
            cout << "We have a winner" << endl;
            break;
        }
        cout << "Do you want to play again? (y/n): ";
        cin >> playAgain;

        if (playAgain == 'n' || playAgain == 'N') {
            SaveGame(players, players_count, saveFile);
        }
    } while ((playAgain == 'y' || playAgain == 'Y') && !GameIsOver(players, players_count));

    delete[] players;
    return 0;
}