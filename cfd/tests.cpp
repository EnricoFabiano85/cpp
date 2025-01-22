#include "FlowVariable.h"
#include "Loop.h"
#include "StorageManager.h"

#include <iostream>


int main()
{
  auto manager = StorageManager::getStorageManager();

  manager->addVariable("solution");

  std::cout << "ADDED A VARIABLE" << std::endl;

  auto var = FlowVariable("solution");

  LoopZ(c, std::execution::seq)
  {
    var[c] = double(I*J);
  }
  LoopEnd();

  Loop(c, X, std::execution::seq)
  {
    std::cout << I << " " << J << " " << var[c] << std::endl;
  } 
  LoopEnd();

  return 0;
}