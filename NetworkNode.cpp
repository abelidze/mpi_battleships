#include "include/NetworkNode.hpp"

using namespace bs;

NetworkNode::NetworkNode() : intercomm(MPI_COMM_WORLD) { }
NetworkNode::NetworkNode(MPI_Comm comm) : intercomm(comm) { }

NetworkNode::~NetworkNode()
{
  this->Disconnect();
  MPI_Finalize();
}

void NetworkNode::Wait()
{
  //
}

void NetworkNode::Disconnect()
{
  if (this->intercomm != MPI_COMM_WORLD) {
    MPI_Comm_free(&this->intercomm);
    this->SetIntercomm(MPI_COMM_WORLD);
  }
}

void NetworkNode::Send(const std::string& data)
{
  this->Send(data.c_str(), data.size() + 1);
}

void NetworkNode::Send(const char* data, size_t size)
{
  MPI_Send(data, size, MPI_CHAR, 0, 0, this->intercomm);
}

std::shared_ptr<char> NetworkNode::Recv()
{
  int size = 0;
  MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, this->intercomm, &this->status);
  MPI_Get_count(&this->status, MPI_CHAR, &size);

  std::shared_ptr<char> buffer(new char[size]);
  MPI_Recv(buffer.get(), size, MPI_CHAR, 0, 0, intercomm, MPI_STATUS_IGNORE);
  return buffer;
}

void NetworkNode::SetIntercomm(MPI_Comm comm)
{
  this->intercomm = comm;
}