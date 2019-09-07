#include <cstdio>
#include <stdexcept>

#include <libusb-1.0/libusb.h>

class LibUsb {
 public:
  LibUsb() {
    int rc = libusb_init(nullptr);
    if (rc) {
      fprintf(stderr, "libusb_init failed: %d\n", rc);
      throw std::runtime_error("libusb_init failed");
    }
  }

  ~LibUsb() {
    libusb_exit(nullptr);
  }
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: usb-storage-bridge <path>\n");
    return 1;
  }

  const std::string path = argv[1];

  LibUsb lib;

  int rc = libusb_set_option(nullptr, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_WARNING);
  if (rc) {
    fprintf(stderr, "libusb_set_option failed: %d\n", rc);
  }

  return 0;
}
