#define LOG_TAG "uart_driver"
#include "uart_driver.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/signalfd.h>
#include <termios.h>
#include <string>
#include "simulation.h"
// #define ENABLE_FAST_UART_POLLING
#ifdef ENABLE_FAST_UART_POLLING
#define UART_POLL_TIMEOUT_MS 1
#else
#define UART_POLL_TIMEOUT_MS 20
#endif
using airoha::communicator::uart_size_t;
using airoha::communicator::UartDriver;
void *UartDriver::uartPthreadRx(void *vp) {
    UartDriver *p = (UartDriver *)vp;
    struct pollfd poll_fd[2];
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, nullptr);
    int sigFd = signalfd(-1, &set, SFD_NONBLOCK | SFD_CLOEXEC);
    LOG_D("uart rx run, %s", p->mDevName);
    uint8_t buffer[10240];
    int timeoutMs = -1;
    size_t readPtr = 0;
    poll_fd[0].fd = p->mUartFd;
    poll_fd[0].events = POLLIN;
    poll_fd[1].fd = sigFd;
    poll_fd[1].events = POLLIN;
    int receive = 0;
    while (1) {
        if (!(p->mIsRunning)) {
            break;
        }
        int poll_ret = poll(poll_fd, 2, timeoutMs);
        if (poll_ret == 0) {
            timeoutMs = -1;
            if (readPtr > 0) {
                buffer[readPtr] = 0;
                LOG_D("rx raw report [by timeout] :%zu(%d)", readPtr, receive);
                receive = 0;
                p->onDataIn(buffer, readPtr);
                readPtr = 0;
            }
            continue;
        } else if (poll_ret < 0) {
            continue;
        }
        int k = read(p->mUartFd, buffer + readPtr, sizeof(buffer) - readPtr - 1);
        // LOG_D("rx raw callback :%d", k);
        receive++;
        if (k > 0) {
            readPtr += k;
        }
        if (readPtr >= (sizeof(buffer) / 2)) {
            buffer[readPtr] = 0;
            LOG_D("rx raw report [by buffer exceed] :%zu(%d)", readPtr,
                  receive);
            receive = 0;
            p->onDataIn(buffer, readPtr);
            readPtr = 0;
        }
        timeoutMs = UART_POLL_TIMEOUT_MS;
    }
    LOG_D("close signal fd");
    ::close(sigFd);
    LOG_D("uart rx end, %s", p->mDevName);
    return NULL;
}
UartDriver::UartDriver(const char *devName, int baudrate, FlowControl fc) {
    strncpy(mDevName, devName, sizeof(mDevName));
    mBaudrate = baudrate;
    mFlowControl = fc;
    mIsRunning = false;
    mUartFd = -1;
}

int UartDriver::getUartFd() { return mUartFd; }
bool UartDriver::open() {
    LOG_D("Visteon: openUart %s, baudrate: %d, flow control: %d", mDevName,
          mBaudrate, mFlowControl);
    if (mUartFd > 0) {
        LOG_E("uart already open");
        return true;
    }
    mUartFd = ::open(mDevName, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (mUartFd < 0) {
        LOG_D("openUart %s error: %s", mDevName, strerror(errno));
        return false;
    }
    LOG_D("Visteon: openUart %s successful, polling timeout: %d", mDevName, UART_POLL_TIMEOUT_MS);
    struct termios tty_config;
    int result = tcgetattr(mUartFd, &tty_config);
    if (result != 0) {
        LOG_E("get attr failed!!%d,%s", result, strerror(errno));
        return false;
    }
    tty_config.c_cflag = 0;
    tty_config.c_lflag = 0;
    tty_config.c_oflag = 0;
    cfsetispeed(&tty_config, baudrateMapping(mBaudrate));
    LOG_D("Visteon: set baudrate: %d", mBaudrate);
    cfsetospeed(&tty_config, baudrateMapping(mBaudrate));
    tty_config.c_cflag |= CS8;
    tty_config.c_cflag |= CREAD;
    // no echo, raw mode
    tty_config.c_lflag &=
        ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | NOFLSH | ISIG);
    tty_config.c_oflag &= ~(ONLCR | OPOST);
    tty_config.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
                            ICRNL | IXON | IXOFF | IXANY);
    tty_config.c_cc[VTIME] = 0;
    tty_config.c_cc[VMIN] = 1;
    tty_config.c_iflag = 0;
    // Ignore error.
    tty_config.c_iflag |= (IGNBRK | IGNPAR);
    LOG_D("Ignore IGNBRK/IGNPAR");
    if (mFlowControl == FC_SOFTWARE) {
        LOG_D("software control enable");
        tty_config.c_iflag |= IXON | IXOFF;
        // tty_config.c_iflag |= 0x80000000;
        tty_config.c_cc[VSTART] = 0x11;
        tty_config.c_cc[VSTOP] = 0x13;
    } else if (mFlowControl == FC_SOFTWARE_EXT) {
        LOG_D("software control ext enable");
        // tty_config.c_iflag |= IXON | IXOFF;
        tty_config.c_iflag |= 0x80000000;
        tty_config.c_cc[VSTART] = 0x11;
        tty_config.c_cc[VSTOP] = 0x13;
    } else if (mFlowControl == FC_HARDWARE) {
        tty_config.c_cflag |= CRTSCTS;
    } else {
        LOG_D("no flow control");
    }
    result = tcsetattr(mUartFd, TCSANOW, &tty_config);
    if (result != 0) {
        return false;
    }
    mIsRunning = true;
    pthread_create(&uartReadThread, NULL, uartPthreadRx, this);
    // uartWriteThread = std::thread(uartTx,this);
    return true;
}
uart_size_t UartDriver::sendData(const uint8_t *buffer, uart_size_t len) {
    if (len == 0) {
        LOG_E("parameter error.");
        return 0;
    }
    if ((!mIsRunning) || (mUartFd <= 0)) {
        LOG_W("Ignore data due to uart not connection.");
        return 0;
    }
    return safeSend(buffer, len);
}
void hexdump_uart_message(const void *buff, size_t length, const char *comment,
                          int retryCount) {
    TRACE_D("UART DUMP [%s][%zu] [%d]", comment, length, retryCount);
    TRACE_D_BIN(buff, length);
    return;
}
#define SAFE_SEND_RETRY_MAXIMUM (5)
uart_size_t UartDriver::safeSend(const void *buffer, uart_size_t length) {
    std::string bufferStr(static_cast<const char *>(buffer), length);
    // LOG_D("uart send data: content:%s,length:%zu", bufferStr.c_str(),
    // length);
    size_t i = 0;
    if (length == 0) {
        return 0;
    }
    // char hex[401] = {0};
    // const char *hexPat = "%02X";
    // char hexSingel[3] = {0};
    // for (size_t j = 0; j < length && j < 200; j++) {
    //     snprintf(hexSingel, sizeof(hexSingel), hexPat,
    //              (static_cast<const uint8_t *>(buffer))[j]);
    //     strcat(hex, hexSingel);
    // }
    // LOG_D("uart write data: %s", hex);
    int retryCount = SAFE_SEND_RETRY_MAXIMUM;
    while (length > 0 && retryCount > 0) {
        int ret =
            ::write(mUartFd, (static_cast<const char *>(buffer)) + i, length);
        LOG_D("uart send data:ret %d, length %zu", ret, length);
        if (ret > 0) {
            hexdump_uart_message((static_cast<const char *>(buffer)) + i, ret,
                                 "Snd", retryCount);
            length -= ret;
            i += ret;
        } else {
            usleep(1000);
        }
        retryCount--;
    }
    if (length > 0) {
        LOG_E("uart send data error");
        TRACE_D("uart data send error: %zu", length);
    }
    return i;
}
bool UartDriver::close() {
    if (mUartFd > 0) {
        mIsRunning = false;
        pthread_kill(uartReadThread, SIGUSR1);
        LOG_D("get lock, notify...");
        pthread_join(uartReadThread, NULL);
        LOG_D("read thread exit");
        tcflush(mUartFd, TCIOFLUSH);
        LOG_D("tcflush finish");
        ::close(mUartFd);
        LOG_D("Close uartfd (%d) finish", mUartFd);
        mUartFd = 0;
    }
    return true;
}
int UartDriver::baudrateMapping(int visibleBaudrate) {
    switch (visibleBaudrate) {
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
        case 460800:
            return B460800;
        case 921600:
            return B921600;
        case 3000000:
            return B3000000;
    }
    return 0;
}
UartDriver ::~UartDriver() {

}
