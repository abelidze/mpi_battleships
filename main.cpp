#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>

// #include "include/table_printer.h"
#include "include/Client.hpp"
#include "include/Server.hpp"

#define FIELD_SIZE 10
#define SHIP_COUNT 10

using namespace bs;

enum Cell
{
  EMPTY = 46,
  SHIP = 35,
  MISS = 79,
  HIT = 88
};

std::pair<int, int> commandToPosition(const std::string& cmd)
{
  return std::make_pair((int) cmd[0] - 'a', (int) cmd[1] - '0');
}

bool isEmptyAround(char map[][FIELD_SIZE], int x, int y)
{
  for (int i = -1; i < 2; ++i) {
    for (int j = -1; j < 2 && x + i >= 0 && x + i < FIELD_SIZE; ++j) {
      if (y + j < 0 || y + j >= FIELD_SIZE || map[y + j][x + i] != EMPTY) {
        return false;
      }
    }
  }
  return true;
}

bool isValidCommand(const std::string& cmd)
{
  return cmd.size() == 2 && cmd[0] >= 'a' && cmd[0] <= ('a' + FIELD_SIZE) && cmd[1] >= '0' && cmd[1] <= ('0' + FIELD_SIZE);
}

bool isCellEmpty(char map[][FIELD_SIZE], const std::string& cmd)
{
  auto pos = commandToPosition(cmd);
  return map[pos.second][pos.first] == EMPTY;
}

bool isCellHasShip(char map[][FIELD_SIZE], const std::string& cmd)
{
  auto pos = commandToPosition(cmd);
  return map[pos.second][pos.first] == SHIP;
}

void printMap(char map[][FIELD_SIZE])
{
  for (int i = 0; i < FIELD_SIZE; ++i) {
    for (int j = 0; j < FIELD_SIZE; ++j) {
      std::cout << map[i][j];
    }
    std::cout << std::endl;
  }
}

void clearMap(char map[][FIELD_SIZE])
{
  for (int i = 0; i < FIELD_SIZE; ++i) {
    for (int j = 0; j < FIELD_SIZE; ++j) {
      map[i][j] = EMPTY;
    }
  }
}

void generateMap(char map[][FIELD_SIZE], std::vector<int>& v)
{
  clearMap(map);
  std::random_shuffle(v.begin(), v.end());
  for (int i = 0, n = 0, x = 0, y = 0; n < SHIP_COUNT && i < v.size(); ++i) {
    x = v[i] % FIELD_SIZE;
    y = v[i] / FIELD_SIZE;
    if (isEmptyAround(map, x, y)) {
      map[y][x] = SHIP;
      ++n;
    }
  }
}

int main(int argc, char* argv[])
{
  // Init MPI
  MPI_Init(&argc, &argv);

  // You must specify file for game session or it will use the default one
  std::string sessionFile = "session.txt";
  if (argc > 1) {
    sessionFile = argv[1];
  }

  // Select mode
  char mode = 0;
  do {
    std::cout << "Select client/server mode (c / s): ";
    std::cin >> mode;
  } while (mode != 'c' && mode != 's');

  // Init communication interface
  std::shared_ptr<NetworkNode> net;
  switch (mode) {
    case 'c':
      net = std::make_shared<Client>(sessionFile);
      break;

    case 's':
      net = std::make_shared<Server>(sessionFile);
      break;
  }

  // Prepare map and random
  std::srand(std::time(NULL));
  char myMap[FIELD_SIZE][FIELD_SIZE];
  char opMap[FIELD_SIZE][FIELD_SIZE];
  std::vector<int> v(FIELD_SIZE * FIELD_SIZE);
  for (int i = 0; i < FIELD_SIZE * FIELD_SIZE; ++i) {
    v[i] = i;
  }

  // Main loop
  while (true) {
    // Wait for opponent
    net->Wait();

    // Generate fields
    clearMap(opMap);
    generateMap(myMap, v);
    std::cout << "My map:" << std::endl;
    printMap(myMap);
    int lifes = SHIP_COUNT;

    // Start game loop
    std::string command;
    bool isPlaying = true;
    bool myTurn = mode == 's';
    while (isPlaying) {
      if (myTurn) {
        // Wait && Validate input
        do {
          std::cout << std::endl << "Your turn (ex. B3): ";
          std::getline(std::cin, command);
          std::transform(command.begin(), command.end(), command.begin(), ::tolower);
        } while (!isValidCommand(command) || !isCellEmpty(opMap, command));

        // Send turn
        net->Send(command);

        // Recv and update opponent's field
        auto data = net->Recv();
        auto pos = commandToPosition(command);
        switch (data.get()[0]) {
          case 'x':
            opMap[pos.second][pos.first] = HIT;
            break;

          case 'o':
            opMap[pos.second][pos.first] = MISS;
            break;

          case 'w':
            std::cout << "You won!" << std::endl;
            isPlaying = false;
            break;

          case 'q':
            std::cout << "Opponent breaks the game" << std::endl;
            isPlaying = false;
            break;
        }

        // Print map
        std::cout << "Opponent map:" << std::endl;
        printMap(opMap);

      } else {
        // Recv turn && validate
        auto data = net->Recv();
        command = std::string(data.get());
        if (!isValidCommand(command)) {
          net->Send("q");
          isPlaying = false;
          break;
        }

        // Check && update your field
        auto pos = commandToPosition(command);
        if (isCellHasShip(myMap, command)) {
          if (--lifes <= 0) {
            std::cout << "You lose!" << std::endl;
            net->Send("w");
            isPlaying = false;
            break;
          }
          myMap[pos.second][pos.first] = HIT;
          net->Send("x");

        } else if (isCellEmpty(myMap, command)) {
          myMap[pos.second][pos.first] = MISS;
          net->Send("o");

        } else {
          net->Send("q");
          isPlaying = false;
          break;
        }

        // Print map
        std::cout << "My map:" << std::endl;
        printMap(myMap);
      }
      myTurn = !myTurn;
    }

    // Testing
    // std::cout << "Enter message to send: ";
    // std::string message;
    // std::cin >> message;
    // net->Send(message);

    // auto data = net->Recv();
    // std::cout << "Received: " << data.get() << std::endl;

    net->Disconnect();

    // Check for another match
    char answer = 0;
    std::cout << "Play again? (y / n): ";
    std::cin >> answer;
    if (answer != 'y') {
      break;
    }
  }
  return 0;
}