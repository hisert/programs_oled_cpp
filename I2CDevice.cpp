#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <string>

using namespace std;

class I2CDevice {
public:
    I2CDevice(const string& devicePath) : devicePath_(devicePath), file_(-1) {}

    bool openDevice() {
        file_ = ::open(devicePath_.c_str(), O_RDWR);
        if (file_ < 0) {
            return false;
        }
        return true;
    }

    void closeDevice() {
        if (file_ >= 0) {
            ::close(file_);
            file_ = -1;
        }
    }

    bool write(uint8_t address, const uint8_t* data, size_t length) {
        if (ioctl(file_, I2C_SLAVE, address) < 0) {
            return false;
        }

        if (::write(file_, data, length) != static_cast<ssize_t>(length)) {
            return false;
        }

        return true;
    }

private:
    string devicePath_;
    int file_;
};
