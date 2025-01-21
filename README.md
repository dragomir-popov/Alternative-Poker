# Alternative-Poker
Simplified poker console app written in C++ for 2-9 players.
game_data.txt is the save file. It's cleared if the game reaches a winner.
If game_data.txt is not empty the game will ask if the players wish to continue form that save.
The information in the file is about the number of players and their balances.
At the end of each round the players are asked if they wish to continue or not. In case they do not wish to do so, the game is saved.

In a new game the players enter how many they are. They start with balance of 10*CHIP_VALUE (=1000).
They initially give 10 points from their balance to the pot and start the betting round.
Players whose balance reaches 0 (except when joining the tie) or who cannot afford the initial pot are set to inactive for the rest of the game.

In the betting round the players are presented with 3 choices: to raise, call or fold.
A player can raise the current bet only if they don't exceed the balance of the player with the lowest balance and if they bet more than the current bet.
A player who calls gives the current bet from their balance to the pot.
A player who fold does not pay the current bet but forfeits their chance to win the pot and their points back.

In case of a tie the players enter the betting round again. If they are left with 0 balance before entering the betting round, they are given 50 points to play with and the tie starts from the initial bet.
The players lost that hand are asked to pay half of the pot to join that tie.
The pot accumulates the bets until a decisive winner is determined.

The game uses the cards from 7 to Ace and their points are:
Ace = 11, The face cards are worth 10, and the numbers are equal to their number.
The names are 2 characters long - the first if for the kind and the second is for the suit. Example: Ace of Hears -> AH, Ten of Clubs -> TC, Seven of Spades -> 7S.
Each player is dealt 3 cards and the strength of that hand is determined by the following rules:
