#include <iostream>
#include <fstream>

#include "include/Server.hpp"

using namespace bs;

Server::Server(std::string sessionFile) : NetworkNode(), sessionFile(sessionFile)
{
  MPI_Open_port(MPI_INFO_NULL, this->port);
  std::ofstream file(this->sessionFile);
  file << this->port;
  file.close();
}

Server::~Server()
{
  std::remove(this->sessionFile.c_str());
  MPI_Close_port(this->port);
}

void Server::Wait()
{
  std::cout << "Waiting for incomming connection..." << std::endl;
  MPI_Comm_accept(this->port, MPI_INFO_NULL, 0, MPI_COMM_SELF, &this->intercomm);
  std::cout << "Client connected." << std::endl;
}