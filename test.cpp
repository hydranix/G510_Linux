#include "G510.hpp"
#include <iostream>
int main()
{
  G510 kbd;
  color oldcolor = kbd.get_color();
  color newcolor = {0xFF, 0x00, 0xFF};
  kbd.set_color(newcolor);
  if(oldcolor.good)
  {
    std::cout << "Old:" << (int)oldcolor.r << ',' << (int)oldcolor.g << ',' << (int)oldcolor.b << std::endl;
    std::cout << "New:" << (int)newcolor.r << ',' << (int)newcolor.g << ',' << (int)newcolor.b << std::endl;
  }
  else
  std::cout << "Fucking hell..." << std::endl;
}