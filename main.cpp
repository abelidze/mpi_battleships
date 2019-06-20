#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <ctime>
#include <limits>
#include <algorithm>
#include <cstdint>

// #include "include/table_printer.h"
#include "include/Client.hpp"
#include "include/Server.hpp"

#define FIELD_SIZE 10
#define SHIP_COUNT 10

using namespace bs;

struct Package
{
 uint8_t code;
 uint8_t first;
 uint8_t second;
 
 enum Types
 {
  GAMEEND = 0,
  TURN,
  HITTED,
  MISSED,
  STOP,  
 };
};

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

bool isValidCommand(const Package* msg)
{
  return msg->first >= 0 && msg->first < FIELD_SIZE && msg->second >= 0 && msg->second < FIELD_SIZE;
}

bool isCellEmpty(char map[][FIELD_SIZE], int x, int y)
{
  return map[y][x] == EMPTY;
}

bool isCellHasShip(char map[][FIELD_SIZE], int x, int y)
{
  return map[y][x] == SHIP;
}

void printMap(char map[][FIELD_SIZE])
{
  for (int i = -1; i < FIELD_SIZE; ++i) {
    for (int j = -1; j < FIELD_SIZE; ++j) {
      if (j == -1 && i == -1) {
        std::cout << "*";
      } else if (i == -1) {
        std::cout << (char) ('A' + j);
      } else if (j == -1) {
        std::cout << i;
      } else {
        std::cout << map[i][j];
      }
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

    // Clear buffered data
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Start game loop
    std::string command;
    Package payload;
    bool isPlaying = true;
    bool myTurn = mode == 's';
    while (isPlaying) {
      if (myTurn) {
        // Wait && Validate input
        payload.code = Package::TURN;
        while (true) {
          std::cout << std::endl << "Your turn (ex. B3): ";
          std::getline(std::cin, command);

          if (command.size() > 1) {
            std::transform(command.begin(), command.end(), command.begin(), ::tolower);
            payload.first = command[0] - 'a';
            payload.second = std::atoi(command.c_str() + 1);

            if (isValidCommand(&payload) && isCellEmpty(opMap, payload.first, payload.second)) {
              break;
            }
          }
        }

        // Send turn
        net->Send((char*) &payload, sizeof(Package));

        // Recv and update opponent's field
        auto data = (Package*) net->Recv().get();
        switch (data->code) {
          case Package::HITTED:
            opMap[data->second][data->first] = HIT;
            std::cout << "Opponent map:" << std::endl;
            printMap(opMap);
            break;

          case Package::MISSED:
            opMap[data->second][data->first] = MISS;
            std::cout << "Opponent map:" << std::endl;
            printMap(opMap);
            break;

          case Package::GAMEEND:
            std::cout << "You won!" << std::endl;
            isPlaying = false;
            break;

          case Package::STOP:
            std::cout << "Opponent breaks the game" << std::endl;
            isPlaying = false;
            break;
        }

      } else {
        // Recv turn && validate
        std::cout << std::endl << "Waiting for opponent's turn..." << std::endl;
        auto data = (Package*) net->Recv().get();
        if (!isValidCommand(data) || data->code != Package::TURN) {
          payload.code = Package::STOP;
          isPlaying = false;
        }

        // Check && update your field
        if (isPlaying) {
          if (isCellHasShip(myMap, data->first, data->second)) {
            myMap[data->second][data->first] = HIT;
            if (--lifes <= 0) {
              std::cout << "You lose!" << std::endl;
              payload.code = Package::GAMEEND;
              isPlaying = false;
            } else {
              payload.code = Package::HITTED;
            }

          } else if (isCellEmpty(myMap, data->first, data->second)) {
            myMap[data->second][data->first] = MISS;
            payload.code = Package::MISSED;

          } else {
            payload.code = Package::STOP;
            isPlaying = false;
          }
        }

        // Send response
        payload.first = data->first;
        payload.second = data->second;
        net->Send((char*) &payload, sizeof(Package));

        // Print map
        if (isPlaying) {
          std::cout << "My map:" << std::endl;
          printMap(myMap);
        }
      }
      myTurn = !myTurn;
    }

    // Testing
    // std::cout << "Enter code to send: ";
    // Package message;
    // int x = 0;
    // std::cin >> x;
    // message.code = (char) x;
    // net->Send((char*) &message, sizeof(Package));

    // auto data = net->Recv();
    // Package* payload = (Package*) data.get();
    // std::cout << "Code: " << (int) payload->code << std::endl;
    // std::cout << "First: " << (int) payload->first << std::endl;
    // std::cout << "Second: " << (int) payload->second << std::endl;

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