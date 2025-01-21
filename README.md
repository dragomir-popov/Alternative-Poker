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
- Three identical cards form a combination are worth as many points as the leading card. For example, three eights give 24 points (3x8=24), while three queens give 30 points (3x10), and so on. An exception is the sevens – 3 sevens give 34 points, which is the strongest hand in the game.
- If the cards are of the same suit, their points are added together.
- Most cards do not form a combination when there are only 2 of a kind, except for aces and sevens. For instance, two aces and a third unmatched card will give 22 points. Two sevens and a third unmatched card will give 23 points.
- The 7S (Seven of Spades) can combine with any cards. If there are 2 cards of the same rank and the 7S, the hand is valued at 2*(the points of those cards) + 11 (the bonus provided by the 7S). If there are 2 cards of the same suit and the 7S, the hand is valued at (the points of one card) + (the points of the other) + 11. If there are two cards of different suits and ranks with the 7S, the hand is valued at (the points of the higher card) + 11.
- If no combination can be made among the cards, the value of the highest card is taken.
