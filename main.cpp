#include <iostream>
#include <memory>

// #include "include/table_printer.h"
#include "include/Client.hpp"
#include "include/Server.hpp"

using namespace bs;

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

  // Main loop
  while (true) {
    // Wait for opponent
    net->Wait();

    // generate fields 
    // start game loop
      // if your turn
        // loop
          // wait for input
          // validate input
        // send turn
        // recv response
          // match -> X
          // no match -> - O
          // game end -> break loop
        // update opponent's field == print
      // else
        // recv opponent's turn
        // check && update your field == print
        // if you are dead
          // send end game
        // else
          // send response

    // Testing
    std::cout << "Enter message to send: ";
    std::string message;
    std::cin >> message;
    net->Send(message);

    auto data = net->Recv();
    std::cout << "Received: " << data.get() << std::endl;

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