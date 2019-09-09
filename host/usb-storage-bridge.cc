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

  ~LibUsb() { libusb_exit(nullptr); }
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: usb-storage-bridge <path>\n");
    return 1;
  }

  const std::string path = argv[1];

  LibUsb lib;

  int rc = libusb_set_option(nullptr, LIBUSB_OPTION_LOG_LEVEL,
                             LIBUSB_LOG_LEVEL_DEBUG);
  if (rc) {
    fprintf(stderr, "libusb_set_option failed: %d\n", rc);
  }

  libusb_device_handle* device =
      libusb_open_device_with_vid_pid(nullptr, 0x1209, 0x5555);
  if (!device) {
    fprintf(stderr, "libusb_open_device_with_vid_pid failed\n");
    return 1;
  }

  libusb_set_auto_detach_kernel_driver(device, 1);

  rc = libusb_set_configuration(device, /*configuration=*/1);
  if (rc) {
    fprintf(stderr, "libusb_set_configuration failed: %s\n",
            libusb_error_name(rc));
  }

  rc = libusb_claim_interface(device, /*interface_number=*/0);
  if (rc) {
    fprintf(stderr, "libusb_claim_interface failed: %s\n",
            libusb_error_name(rc));
  }

  struct PullEvent {
    uint64_t offset;
    uint64_t length;
  };

  PullEvent event = {0, 0};

  int transferred = 0;
  rc = libusb_interrupt_transfer(device, /*endpoint=*/0x82,
                                 reinterpret_cast<unsigned char*>(&event),
                                 sizeof(event), &transferred, /*timeout=*/0);
  printf("rc=%s, transferred=%d, offset=%ld, length=%ld\n",
         libusb_error_name(rc), transferred, event.offset, event.length);

  libusb_close(device);

  return 0;
}
