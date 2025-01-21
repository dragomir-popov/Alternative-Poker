/**
 * 
 * Solution to course project # 10
 * Introduction to programming course
 * Faculty of Mathematics and Informatics of Sofia University
 * Winter semester 2023/2024
 * 
 * @author Dragomir Popov
 * @idnumber 3MI0600498
 * @compiler gcc
 * 
 * Main file with full functionallity
 * 
 */
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

void InitializeBalances(Player* players);
void TakeInitialBet(Player* players);
bool Raise(Player& player, int& currentBet, int raiseAmount, Player* players);
bool Call(Player& player, int currentBet, int playerBet);
void Fold(Player& player, int &activePlayers);
void BettingRound(Player* players, int currentBet);
void ValidateRaiseAmount(Player* players, const int i, int &currentBet);
int ValidateBettingChoice(const int currentBet, const int i);
void HandlePlayerChoice(const int choice, Player* players, int &currentBet, bool &bettingComplete, int &i, int* playerBets, bool* hasActed);
void PlayersMatchedBet(bool &bettingComplete, const int activePlayers, Player* players, const int currentBet, int *playerBets);
int CalculateHand(Player& player);
void InitializeRemainingIndices();
Card* DealCard();
void DealCards(Player*& players);
void FillDeck();
Player* CreatePlayerArray();
void FilterActivePlayers(Player* players);
void GameRound(Player*& players);
void GameStart(Player*& players, const char saveFile[]);
void LoadGame(Player*& players, const char* saveFile);
void SaveGame(Player* players, const char* saveFile);
bool FileEmptyOrNonExistent(const char* saveFile);
bool CheckForContinue(Player*& players, const char* saveFile);
void DisplayBalances(Player* players);
void PlayersInTheTie(Player* players, const int highestCount, int * highestPlayers);
void JoinTie(Player* players);
void ZeroBalanceInTie(Player* players);
void TieBreaker(int & highestCount, Player* players, int* highestPlayers, int &currentBet, int &highestScore);
bool SameSuit(Card* hand, int &score);
bool Has7S(Card* hand);
bool TwoSuitAnd7S(Card* hand, int &score);
void HighCardAnd7S(Card* hand, int &score);
bool TwoOfAKindAnd7S(Card* hand, int &score);
bool ThreeOfAKind(Card* hand, int &score);
bool TwoAces(Card* hand, int &score);
bool Two7s(Card* hand, int &score);
void HighCard(Card* hand, int &score);

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
int players_count = 0;
int activePlayers = 0; // active players are dealt cards and can bet

// Output each player's balance
void DisplayBalances(Player* players) {
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }
    for(int q = 0; q < players_count; ++q)
        {
            cout << "Player" << (q+1) << " balance: " << players[q].balance << "; ";
            if((q+1) % 3 == 0) { // Output 3 players per row max
                cout << endl;
            } 
        }
        cout << endl;
}

// Asks every inactive player to join the tie
void JoinTie(Player* players) {
    int addToPot = 0; 
    int halfPot = (pot/2);
    for(int i = 0; i < players_count; i++) {
        char answer;
        if((players[i].isActive == false) && (players[i].balance >= halfPot)) { // Only asks inactive players who can afford to pay half the pot
            cout << "Player" << (i+1) << " pay " << halfPot << " to join the tie? Y/N:" << endl;
            cin >> answer;

            if(answer == 'y' || answer == 'Y') {
                players[i].balance -= halfPot;
                addToPot += halfPot; // ensures every player pays the same amount and is added to the pot after everyone answers
                players[i].isActive = true;
            }
        }
    }
    pot += addToPot;
}

// Gives active players in the tie 50 points to bet with
void ZeroBalanceInTie(Player* players) {
    for(int i = 0; i < players_count; ++i) {
        if(players[i].isActive && players[i].balance < 1) {
            players[i].balance += 50;
        }
    }
    return;
}

// Deactivate players whose hands are not tied for the win
void PlayersInTheTie(Player* players, const int highestCount, int * highestPlayers) {
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

// Sets the balance of each player to 100 * CHIP_VALUE
void InitializeBalances(Player* players) {
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }
    for (int i = 0; i < players_count; ++i) {
        players[i].balance = 100 * CHIP_VALUE;
    }
}

// Gives the chip value from each player to the pot
void TakeInitialBet(Player* players) {
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }
    for (int i = 0; i < players_count; ++i) {
        if(players[i].isActive && players[i].balance >= CHIP_VALUE) {
            players[i].balance -= CHIP_VALUE;
            pot += CHIP_VALUE;
        }
        else {
            cout << "Player" << (i+1) << " cannot afford to pay minimal amount" << endl;
            players[i].isActive = false;
        }
    }
}

// sets maxAmount to the lowest player's balance
int MaxRaiseAmount (Player* players, const int currentBet, const int raiseAmount) {
    int maxAmount = players[0].balance;
    for(int i = 0; i < players_count; ++i) {
        if(players[i].isActive) { 
            if(players[i].balance < maxAmount) {
                maxAmount = players[i].balance;
            }
        }
    }
    return maxAmount;
}

// Returns true if the player raises successfully
bool Raise(Player& player, int& currentBet, const int raiseAmount, Player* players) {
    if (raiseAmount < CHIP_VALUE || raiseAmount <= currentBet || raiseAmount > player.balance) {
        return false;
    }
    if(raiseAmount > MaxRaiseAmount(players, currentBet, raiseAmount)) {
        return false;
    }
    player.balance -= raiseAmount;
    pot += raiseAmount;
    currentBet = raiseAmount;
    return true;
}

// Returns true if the player can call on the current bet
bool Call(Player& player, const int currentBet, const int playerBet) {
    int amountToCall = currentBet - playerBet;

    if (currentBet <= playerBet) { // The player has already bet atleast an equal amount -> nothing to match
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

// Sets the player to inactive, that player does not pay the current bet
void Fold(Player& player, int &activePlayers) {
    player.isActive = false;
    --activePlayers; 
    cout << "Player folded." << endl;
}

// The players give their bets
void BettingRound(Player* players, int currentBet) {
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }
    int* playerBets = new int[players_count](); // Tracks individual bets
    bool* hasActed = new bool[players_count](); // Tracks if a player has acted this round
    bool bettingComplete = false;

    while (!bettingComplete) {
        bettingComplete = true;

        for (int i = 0; i < players_count; ++i) {
            if (activePlayers < 2) { // End betting round if fewer than 2 active players
                bettingComplete = true;
                break;
            }
            if (!players[i].isActive || hasActed[i]) continue; // Skip inactive or already acted players
            
            int choice = ValidateBettingChoice(currentBet, i); // Get a valid choice for betting: 1, 2 or 3
            HandlePlayerChoice(choice, players, currentBet, bettingComplete, i, playerBets, hasActed);

            if (activePlayers < 2) { // If only one player is left active, end the betting round immediately
                bettingComplete = true;
                break;
            }
        }
        // Ensure the loop only continues if all active players match the current bet
        PlayersMatchedBet(bettingComplete, activePlayers, players, currentBet, playerBets);
    }  
    delete[] playerBets; // Free allocated memory
    delete[] hasActed;
}

// returns a valid number between 1 and 3
int ValidateBettingChoice(const int currentBet, const int i) {
    char input;
    int choice;

    while (true) {
        cout << "Player " << (i + 1) << "'s turn. Current bet: " << currentBet << endl;
        cout << "Options: 1) Raise  2) Call  3) Fold" << endl;
        cin >> input;
        cin.ignore(1000, '\n'); // ignores the rest of the input after the first char

        if (input >= '1' && input <= '3') {
            choice = input - '0'; // Converts char to int
            return choice;
        } else {
            cout << "Invalid input. Please enter a number between 1 and 3." << endl;
        }
    }
    return choice;
}

void ValidateRaiseAmount(Player* players, const int i, int &currentBet) {
    int raiseAmount;
    bool validRaise = false;
    while (!validRaise) {
        cout << "Enter raise amount: ";
        // handle invalid input
        if (!(cin >> raiseAmount)) {
            cin.clear();
            cin.ignore(1000, '\n'); // Discards the rest of the input
            cout << "Invalid raise. Max amount is " << MaxRaiseAmount(players, currentBet, raiseAmount) << endl;
            cout << "Min amount is " << currentBet+1 << endl;
            continue;
        }
        validRaise = Raise(players[i], currentBet, raiseAmount, players);
        if (!validRaise) {
            cout << "Invalid raise. Max amount is " << MaxRaiseAmount(players, currentBet, raiseAmount) << endl;
            cout << "Min amount is " << currentBet+1 << endl;
        }
    }
}

// Depending on the player's choice call the appropriete function
void HandlePlayerChoice(const int choice, Player* players, int &currentBet, bool &bettingComplete, int &i, int* playerBets, bool* hasActed) {
    switch (choice) {
        case 1: { // Raise
            ValidateRaiseAmount(players, i, currentBet);
            playerBets[i] = currentBet; // Update player bet
            bettingComplete = false; // Restart round
            
            for (int j = 0; j < players_count; ++j) { // Reset all players' actions to false except the current player
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
}

// Ensure the betting only continues if all active players match the current bet
void PlayersMatchedBet(bool &bettingComplete, const int activePlayers, Player* players, const int currentBet, int *playerBets) {
    if (activePlayers > 1) { 
        for (int i = 0; i < players_count; ++i) {
            if (players[i].isActive && playerBets[i] < currentBet) {
                bettingComplete = false;
                break;
            }
        }
    }
}

// Check if only 1 player with points is left
bool GameIsOver(Player* players) {
    int remainingPlayers = 0;
    for (int i = 0; i < players_count; ++i) {
        if(players[i].balance > 0) {
            remainingPlayers++;
        }
    }
    if (remainingPlayers < 2) {
        cout << "We have a winner" << endl;
        return true;
    }
    return false;
}

// Check if the hand contains exactly 2 aces
bool TwoAces(Card* hand, int &score) {
    int aceCount = 0;
    for (int i = 0; i < 3; i++) {
        if (hand[i].name == 'A') {
            aceCount++;
        }
    }
    if (aceCount == 2) {
        score = 22;
        return true;
    }
    return false;
}

// Check if the hand contains exactly 2 sevens and an unrelated card
bool Two7s(Card* hand, int &score) {
    int count = 0;
    for (int i = 0; i < 3; i++) {
        if (hand[i].name == '7') {
            count++;
        }
    }
    if (count == 2) {
        score = 23;
        return true;
    }
    return false;
}

// Check if a hand is three of the same suit
bool SameSuit(Card* hand, int &score) {
    if (hand[0].suit == hand[1].suit && hand[1].suit == hand[2].suit) {
        score = hand[0].value + hand[1].value + hand[2].value;
        return true;
    }
    return false;
}

// Check if a hand has 7S and two cards with the same suit
bool TwoSuitAnd7S(Card* hand, int &score) {
    if ((hand[0].suit == hand[1].suit || hand[1].suit == hand[2].suit || hand[0].suit == hand[2].suit)) {
        int suitCardValue = 0;

        for (int i = 0; i < 3; ++i) {
            if (hand[i].suit == 'S') continue; // Ignores 7S
            suitCardValue += hand[i].value;
        }
        score = suitCardValue + 11; // Rule for 7S + two same-suit cards
        return true;
    }
    return false;
}

// Calculates the score for 7S and two unrelated cards and overwrites score
void HighCardAnd7S(Card* hand, int &score) {
    int highestValue = 0;
    for (int i = 0; i < 3; ++i) {
        if (hand[i].suit == 'S' && hand[i].name == '7') continue; // Ignore 7S
        highestValue = (hand[i].value > highestValue) ? hand[i].value : highestValue;
    }
    score = highestValue + 11;
    return;
}

// Check if a hand is 7S with two other of a kind
bool TwoOfAKindAnd7S(Card* hand, int &score) {
    if ((hand[0].name == hand[1].name || hand[1].name == hand[2].name || hand[0].name == hand[2].name)) {
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
        return true;
    }
    return false;
}

// Check if a hand has 3 cards of the same kind
bool ThreeOfAKind(Card* hand, int &score) {
    if (hand[0].name == hand[1].name && hand[1].name == hand[2].name) {
        if (hand[0].name == '7') {
            score = 34; // Special case: three 7s
        } else {
            score = 3 * hand[0].value;
        }
        return true;
    }
    return false;
}

// Finds the value of the highest card in hand
void HighCard(Card* hand, int &score) {
    int highestValue = 0;
        for (int i = 0; i < 3; ++i) {
            highestValue = (hand[i].value > highestValue) ? hand[i].value : highestValue;
        }
        score = highestValue;
}

// Check if a hand contains 7S
bool Has7S(Card* hand) {
    for (int i = 0; i < 3; ++i) {
        if (hand[i].name == '7' && hand[i].suit == 'S') {
            return true;
        }
    }
    return false;
}

// How many points is a single player's hand worth?
int CalculateHand(Player& player) {
    Card* hand = player.hand;
    int score = 0; // Score/Strength of the hand 
    bool has7S = Has7S(hand); // Check for 7S

    // Three identical cards
    if(ThreeOfAKind(hand, score)) {
        player.score = score;
        return score;
    }
    if(has7S) {
        if(TwoOfAKindAnd7S(hand, score)) { // 7S with two identical cards
            player.score = score;
            return score;
        }
        else if (TwoSuitAnd7S(hand, score)) { // 7S with two cards of the same suit
            player.score = score;
            return score;
        }
        else { // 7S with unrelated cards
            HighCardAnd7S(hand, score);
            player.score = score;
            return score;
        }
    }
    else if(TwoAces(hand, score)) { // TwoAces and an unrealated card
        player.score = score;
        return score;
    }
    else if(Two7s(hand, score)) { // Two sevens and an unrelated card
        player.score = score;
        return score;
    }
    else if (SameSuit(hand, score)) { // Three cards of the same suit
        player.score = score;
        return score;
    }
    else { //  No combinations
        HighCard(hand, score);
        player.score = score;
    }
    return score;
}

// Calculates how much every players hand is worth to determine if there's a draw
int* CalculateHighestHand(Player* players, int& highestScore, int& highestCount) {
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return nullptr;
    }
    highestScore = 0;
    highestCount = 0;
    // Dynamically allocates an array to store indices of players with the highest score
    int* highestPlayers = new int[players_count];

    // Iterates through the players to find the highest score and the corresponding players
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
    if(highestCount > 1) {
        cout << highestCount << " players are tied" << endl;
    }
    return highestPlayers;
}

// Refills the array of random indices with new random indices 
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

// Loop for dealing cards to every player
void DealCards(Player*& players) {
    if (players == nullptr) {
        cout << "Nullpointer error - players" << endl;
        return;
    }
    for (int p = 0; p < players_count; ++p) {
        if(players[p].isActive == true) { // deals only to active players
            cout << "Player " << (p + 1) << " cards:" << endl;
            for (int c = 0; c < 3; ++c) { // 3 cards per hand
                Card* card = DealCard(); // Deals a card
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

// Fills the players array with fresh players
Player* CreatePlayerArray() { 
    Player* players = new Player[players_count];
    return players;
}

// Returns the valid number of players
int InputPlayers() {
    char input;
    int count;

    while (true) {
        cout << "Number of players (2-9): ";
        cin >> input;
        cin.ignore(1000, '\n'); // ignores the rest of the input after the first char

        if (input >= '2' && input <= '9') {
            count = input - '0'; // Converts char to int
            return count;
        } else {
            cout << "Invalid input. Please enter a number between 2 and 9." << endl;
        }
    }
}

// Clears all the data from the safe file
void ClearFile(const char* filename) {
    std::ofstream file(filename, std::ofstream::trunc); // Opens the file in truncate mode which clears the text
    if (file.is_open()) {
        file.close();
    } else {
        std::cerr << "Failed to open the file.\n";
    }
}

// Writes the total number of players and their balances in the safe file
void SaveGame(Player* players, const char* saveFile) {
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
void LoadGame(Player*& players, const char* saveFile) {
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

// Asks the players if they wish to continue from the safe file
bool CheckForContinue(Player*& players, const char* saveFile) {
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
void FilterActivePlayers(Player* players) { 
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

// In case of a tie the betting continues
void TieBreaker(int & highestCount, Player* players, int* highestPlayers, int &currentBet, int &highestScore) {
    while(highestCount != 1) { 
        PlayersInTheTie(players, highestCount, highestPlayers); // Filter out the players not in the tie
        JoinTie(players);
        ZeroBalanceInTie(players); // If a player has 0 balance upon joining the tie, gives 50pts
        InitializeRemainingIndices(); // randomize and refill the deck
        DealCards(players);
        BettingRound(players, currentBet);
        highestPlayers = CalculateHighestHand(players, highestScore, highestCount);
    }
}

void GameRound(Player*& players) {
    if (players == nullptr) {
        cout << "Nullpointer error" << endl;
        return;
    }
    activePlayers = players_count;
    int currentBet = CHIP_VALUE; // Starting bet
    TakeInitialBet( players); 
    DealCards(players);
    BettingRound(players, currentBet);

    int highestScore, highestCount;
    int* highestPlayers = CalculateHighestHand(players, highestScore, highestCount);
    TieBreaker(highestCount, players, highestPlayers, currentBet, highestScore);
    
    players[highestPlayers[0]].balance += pot;
    pot = 0;
    DisplayBalances(players);
    
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

// If no save file is found or if its empty make a default player array with a new player count
void NewGame(Player** players) {
    players_count = InputPlayers();
    *players = CreatePlayerArray();
    InitializeBalances(*players);
    FillDeck();
}

// At the start of the program should the player continue or start a new game
void GameStart(Player*& players, const char saveFile[]) {
    if (!CheckForContinue(players, saveFile)) { // New Game
        NewGame(&players);
    }
    else{ // previous game found -> continue
        LoadGame(players, saveFile);
        FillDeck();
    }
}

int main() {
    const char saveFile[] = "game_data.txt";
    Player* players = nullptr;
    GameStart(players, saveFile);

    char playAgain;
    do {
        InitializeRemainingIndices(); // refills and shuffles the available cards to deal
        FilterActivePlayers(players); // deactivates players with non positive balance
        if (players_count == 0) {
            cout << "No players with positive balance. Game over!" << endl;
            break;
        }
        GameRound(players);
        activePlayers = 0; // Resets the active players
        cout << "Game round completed." << endl;
        if (GameIsOver(players)) {
            ClearFile(saveFile);
            break;
        }
        cout << "Do you want to play again? (y/n): ";
        cin >> playAgain;
        if ((playAgain != 'y') && (playAgain != 'Y')) { // Game will not continue and is saved in the file
            SaveGame(players, saveFile);
        }
    } while ((playAgain == 'y' || playAgain == 'Y') && !GameIsOver(players));

    delete[] players;
    return 0;
}