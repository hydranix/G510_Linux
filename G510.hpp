#pragma once
#include <libusb-1.0/libusb.h>

struct color
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  bool good;
};


class G510
{
public:
  G510();
  ~G510();

color get_color();
void  set_color(color c);

private:
  libusb_device *kbddev;
  uint8_t inf_num;
};