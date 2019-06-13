#pragma once

#include <string>
#include <memory>
#include <mpi.h>

namespace bs {

class NetworkNode
{
public:
  NetworkNode();
  NetworkNode(MPI_Comm comm);
  virtual ~NetworkNode();

  virtual void Wait();
  virtual void Send(const std::string& data);
  virtual void Send(const char* data, size_t size);
  virtual std::shared_ptr<char> Recv();
  virtual void Disconnect();

protected:
  MPI_Comm intercomm;
  MPI_Status status;
  char port[MPI_MAX_PORT_NAME];

  virtual void SetIntercomm(MPI_Comm comm);
};

} // namespace bs