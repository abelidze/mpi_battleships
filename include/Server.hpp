#pragma once

#include <string>

#include "NetworkNode.hpp"

namespace bs {

class Server: public NetworkNode
{
public:
  Server(std::string sessionFile);
  ~Server() override;

  void Wait() override;

protected:
  std::string sessionFile;
};

} // namespace bs