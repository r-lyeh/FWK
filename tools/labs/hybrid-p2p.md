# Fair hybrid P2P model (rlyeh, public domain)

## Abstract

In gamedev, the problem with p2p is that decision-making falls to individuals, and they can unbalance the balance in their very own favor. For this reason, the client/server model is usually preferred, with the servers (and $maintenance$) being handled by the company that hosts the game.

## Theory 

Following hybrid p2p system would be one in which a game is made up of many clusters of clients that play different games simultaneously, but in which the peers that act as referees actually belong to a different game, in such a way that a peer cannot Referee their own game. Basically if a peer is in a game-A with A-peers, it can only arbitrate a game-B with B-peers, all this while peer is also playing their game with other A peers.

In addition, in order for peer A to arbitrate and initiate a game-B of B-peers, all nodes of B must send the source code of the server (with the rules of the game) to peer A for further validation. Once the source code is validated (sources must match for all the involved nodes), server-A will process the traffic of game-B, but note that server does not care about the rules of B, neither what the traffic of B looks like, and in fact, server-A does not know or care what peers in game-B are doing... what peer A does know as a player, however, is everything about game-A, whose refereers are in fact random external players to A set.

## Appendix and notes

```
game A                                                   game B
[ player12 player16* player45 server33* player08 ]       [ player49 player09 player33* server16* ]
```

- 16 is a player in A, and a refeerer in B. 33 is a player in B, and a refeerer in A. 
- 33 agreed to arbitrate game A after checking data & code consensus for players { 12, 16, 45, 08 }.
- 16 agreed to arbitrate game B after checking data & code consensus for players { 49, 09, 33 }.
- traffic could be encrypted. public key could be a deterministic hash of all code+datas involved.
- peers could use an anonymous karma score system: positive for peers completing game sessions, negative for those peers cheating and/or with communication issues.

