/*    G510_Linux - Class for controlling LED backlight on Logitech G510
 *      Adapted from rainbow510 by hazardousparticle (https://github.com/hazardousparticle/rainbow510)
 */
#include "G510.hpp"

#include <libusb-1.0/libusb.h>

#define USB_REQUEST_SET_LED LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_OUT
#define USB_REQUEST_GET_LED LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_IN

#define USB_TRANSFER_REQUEST_GET 0x01
#define USB_TRANSFER_REQUEST_SET 0x09
#define USB_TRANSFER_VALUE 0x0305
#define USB_TRANSFER_INDEX 0x01

G510::G510()
  : kbddev(nullptr)
{
  libusb_init(nullptr);
  libusb_device** devs = nullptr;
  libusb_device * dev = nullptr;
  ssize_t count;
  count = libusb_get_device_list(nullptr, &devs);
  if(count < 0)
    throw;
  int idx = 0;
  while((dev = devs[idx++]) && !kbddev)
  {
    libusb_device_descriptor desc = {};
    libusb_config_descriptor *cfgdesc = nullptr;
    if(libusb_get_device_descriptor(dev, &desc) == 0)
    {
      if(desc.bDeviceClass != LIBUSB_CLASS_PER_INTERFACE)
        continue;
      libusb_get_config_descriptor(dev, 0, &cfgdesc);
      if(cfgdesc)
      {
        for(int i=0;i<cfgdesc->bNumInterfaces && !kbddev;i++)
        {
          const libusb_interface *inf = &cfgdesc->interface[i];
          for(int j=0;j<inf->num_altsetting && !kbddev;j++)
          {
            const libusb_interface_descriptor *infdesc;
            infdesc = &inf->altsetting[j];
            if(infdesc->bInterfaceClass == LIBUSB_CLASS_HID)
            {
              if(desc.idVendor == 0x046d && desc.idProduct == 0xc22d)
              {
                kbddev = dev;
                inf_num = infdesc->bInterfaceNumber+1;
              }
            }
          }
        }
      }
    }
  }
  libusb_free_device_list(devs, 1);
  if(!kbddev)
    throw;
}


G510::~G510()
{
  libusb_exit(nullptr);
}

color G510::get_color()
{
  libusb_device_handle *hdev;
  color c = {};
  // 0x05 color change command
  uint8_t reply[4] = {0x05, 0x00, 0x00, 0x00};
  if(libusb_open(kbddev, &hdev)==0)
  {
    if(libusb_kernel_driver_active(hdev, inf_num) >= 0)
    {
      libusb_detach_kernel_driver(hdev, inf_num);
    }
    if(libusb_claim_interface(hdev, inf_num) == 0)
    {
      int res = libusb_control_transfer(hdev,
                                        USB_REQUEST_GET_LED,
                                        USB_TRANSFER_REQUEST_GET,
                                        USB_TRANSFER_VALUE,
                                        USB_TRANSFER_INDEX,
                                        reinterpret_cast<unsigned char*>(reply),
                                        sizeof(reply)/sizeof(char),
                                        200);
      if(res != LIBUSB_ERROR_TIMEOUT &&
         res != LIBUSB_ERROR_PIPE &&
         res != LIBUSB_ERROR_NO_DEVICE &&
         res > 0)
        {
          c.r = reply[1];
          c.g = reply[2];
          c.b = reply[3];
          c.good = true;
        }
        else
        {
          c.good = false;
        }
    }
    if(!libusb_kernel_driver_active(hdev, inf_num))
    {
      libusb_release_interface(hdev, inf_num);
      libusb_attach_kernel_driver(hdev, inf_num);
    }
    libusb_close(hdev);
  }
  return c;
}

void G510::set_color(color c)
{
  libusb_device_handle *hdev;
  uint8_t cmd[4] = {0x05, c.r, c.g, c.b};
  if(libusb_open(kbddev, &hdev)==0)
  {
    if(libusb_kernel_driver_active(hdev, inf_num) >= 0)
    {
      libusb_detach_kernel_driver(hdev, inf_num);
    }
    if(libusb_claim_interface(hdev, inf_num) == 0)
    {
      libusb_control_transfer(hdev,
                              USB_REQUEST_SET_LED,
                              USB_TRANSFER_REQUEST_SET,
                              USB_TRANSFER_VALUE,
                              USB_TRANSFER_INDEX,
                              reinterpret_cast<unsigned char*>(cmd),
                              sizeof(cmd)/sizeof(char),
                              200);
    }
    if(!libusb_kernel_driver_active(hdev, inf_num))
    {
      libusb_release_interface(hdev, inf_num);
      libusb_attach_kernel_driver(hdev, inf_num);
    }
    libusb_close(hdev);
  }
}
