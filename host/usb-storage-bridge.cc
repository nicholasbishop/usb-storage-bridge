#include <cstdio>
#include <stdexcept>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

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

  int fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "open failed\n");
    return 1;
  }

  struct stat sb;
  fstat(fd, &sb);
  printf("sb.st_size: %ld\n", sb.st_size);

  unsigned char* file_data = reinterpret_cast<unsigned char*>(
      mmap(nullptr, sb.st_size, PROT_READ, MAP_SHARED, fd, /*offset=*/0));
  if (!file_data) {
    fprintf(stderr, "mmap failed\n");
    return 1;
  }

  struct PullEvent {
    uint64_t offset;
    uint64_t length;
  };

  for (;;) {
    PullEvent event = {0, 0};

    int transferred = 0;
    rc = libusb_interrupt_transfer(device, /*endpoint=*/0x82,
                                   reinterpret_cast<unsigned char*>(&event),
                                   sizeof(event), &transferred, /*timeout=*/0);
    printf("input: rc=%s, transferred=%d, offset=%ld, length=%ld\n",
           libusb_error_name(rc), transferred, event.offset, event.length);
    if (rc) {
      break;
    }

    printf("data: %.*s\n", (int)event.length, file_data + event.offset);

    rc = libusb_bulk_transfer(device, /*endpoint=*/0x02,
                              file_data + event.offset, event.length,
                              &transferred, /*timeout=*/0);
    printf("output: rc=%s, transferred=%d\n", libusb_error_name(rc),
           transferred);
    if (rc) {
      break;
    }
  }

  libusb_close(device);

  return 0;
}
