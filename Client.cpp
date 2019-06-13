#include <iostream>
#include <fstream>

#include "include/Client.hpp"

using namespace bs;

Client::Client(std::string sessionFile) : NetworkNode(), sessionFile(sessionFile)
{
  std::ifstream file(this->sessionFile);
  file >> this->port;
  file.close();
}

void Client::Wait()
{
  std::cout << "Trying to connect to server..." << std::endl;
  MPI_Comm_connect(this->port, MPI_INFO_NULL, 0, MPI_COMM_SELF, &this->intercomm);
  std::cout << "Connected to server." << std::endl;
}