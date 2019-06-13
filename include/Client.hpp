#pragma once

#include <string>

#include "NetworkNode.hpp"

namespace bs {

class Client: public NetworkNode
{
public:
  Client(std::string sessionFile);

  void Wait() override;

protected:
  std::string sessionFile;
};

} // namespace bs