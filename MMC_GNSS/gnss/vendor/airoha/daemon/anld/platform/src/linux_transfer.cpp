#define LOG_TAG "LinuxTrans"
#include "linux_transfer.h"
#include <arpa/inet.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <memory.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "native_call.h"
#include "platform_event.h"
#include "simulation.h"
#define FILE_TRANSMIT_STEP 4096
#define LOCAL_SOCKET_PATH "/data/vendor/airoha/local_server"
LinuxTransfer::LinuxTransfer(const char *ip, uint16_t port, bool isServer)
    : SuperTransfer(1024 * 8) {
    pthread_mutex_init(&mMutex, nullptr);
    mIsServer = isServer;
    mPeerFd = -1;
    mPort = 0;
    strncpy(mIpaddr, ip, sizeof(mIpaddr));
    mPort = port;
    mThreadHdr = 0;
    mEpollFd = -1;
    transmitFileFd = nullptr;
    mLocalMsgFdClient = -1;
    mLocalMsgFdServer = -1;
}
LinuxTransfer::~LinuxTransfer() {
    pthread_mutex_destroy(&mMutex);
    return;
}
void *LinuxTransfer::handleThreadFunc(void *p) {
    LinuxTransfer *instance = (LinuxTransfer *)p;
    instance->mSignalFd = LinuxTransfer::createSignalFd();
    if (instance->mSignalFd == -1) {
        LOG_E("create signal error");
        return nullptr;
    }
    instance->mEpollFd = epoll_create(10);
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = instance->mSignalFd;
    epoll_ctl(instance->mEpollFd, EPOLL_CTL_ADD, instance->mSignalFd, &event);
    event.data.fd = instance->mSocketFd;
    epoll_ctl(instance->mEpollFd, EPOLL_CTL_ADD, instance->mSocketFd, &event);
    // for client, we add self socket fd to epoll
    instance->peerAdd(instance->mPeerFd);
    struct epoll_event revent[10];
    bool exitThread = false;
    for (;;) {
        int ret = epoll_wait(instance->mEpollFd, revent, 10, -1);
        if (ret == -1) {
            if (errno == EINTR) {
                continue;
            }
            LOG_E("epoll unexpected error=[%s]", strerror(errno));
            break;
        }
        for (int i = 0; i < ret; i++) {
            if (revent[i].data.fd == instance->mSignalFd) {
                exitThread = true;
                LOG_D("Exit Signal Receive");
                break;
            } else if (revent[i].data.fd == instance->mSocketFd ||
                       revent[i].data.fd == instance->mPeerFd) {
                instance->socketHandler(revent[i].data.fd);
            }
        }
        if (exitThread) {
            break;
        }
    }
    close(instance->mEpollFd);
    close(instance->mSignalFd);
    LOG_I("handle thread exit %p", p);
    return nullptr;
}
int LinuxTransfer::createSocketFd() {
    if (mIsServer) {
        return createTcpServer(mIpaddr, mPort, 1);
    } else {
        return createTcpClient(mIpaddr, mPort);
    }
    return -1;
}
int LinuxTransfer::createTcpClient(const char *ip, uint16_t port) {
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        LOG_E("create TCP Client FAILED,");
        return -1;
    }
    struct sockaddr_in addrinfo;
    memset(&addrinfo, 0, sizeof(addrinfo));
    addrinfo.sin_family = AF_INET;
    addrinfo.sin_port = htons(port);
    if (connect(fd, (const sockaddr *)&addrinfo, sizeof(struct sockaddr_in)) !=
        0) {
        LOG_E("connect server %s:%d error:%s", ip, port, strerror(errno));
        close(fd);
        return -1;
    }
    return fd;
}
int LinuxTransfer::createTCPSocket(const char *abstractName,
                                   int supportClient) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un un_s;
    memset(&un_s, 0, sizeof(un_s));
    if (fd < 0) {
        LOG_E("New socket error");
        return -1;
    }
    un_s.sun_family = AF_UNIX;
    un_s.sun_path[0] = '\0';
    strcpy(&(un_s.sun_path[1]), abstractName);
    LOG_D("create IPC socket, fd:%d", fd);
    unlink(un_s.sun_path);
    if (::bind(fd, (struct sockaddr *)&un_s, (socklen_t)sizeof(un_s)) < 0) {
        LOG_E("bind error %d,%s", errno, strerror(errno));
        return -1;
    }
    if (listen(fd, supportClient) < 0) {
        return -1;
    }
    return fd;
}
int LinuxTransfer::createTcpServer(const char *ip, uint16_t port,
                                   int supportClient) {
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        LOG_E("create TCP socket FAILED, %s[%d]", strerror(errno), errno);
        return -1;
    }
    struct sockaddr_in addrinfo;
    memset(&addrinfo, 0, sizeof(addrinfo));
    addrinfo.sin_family = AF_INET;
    addrinfo.sin_port = htons(port);
    inet_aton(ip, &addrinfo.sin_addr);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (::bind(fd, (sockaddr *)&addrinfo, sizeof(addrinfo)) == -1) {
        LOG_E("bind failed, %s[%d]", strerror(errno), errno);
        ::close(fd);
        return -1;
    }
    if (listen(fd, supportClient) < 0) {  // support 1 client
        LOG_E("listen failed, %s[%d]", strerror(errno), errno);
        ::close(fd);
        return -1;
    }
    LOG_I("create tcp socket:%s %d %d", ip, port, supportClient);
    return fd;
}
int LinuxTransfer::createSignalFd() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        LOG_E("proc mask signal error");
        return -1;
    }
    int fd = signalfd(-1, &mask, 0);
    LOG_D("create signal %d", fd);
    return fd;
}
bool LinuxTransfer::connectionSetup() {
    pthread_mutex_lock(&mMutex);
    LOG_D("connection setup... server: %d", mIsServer);
    mSocketFd = createSocketFd();
    if (!mIsServer) {
        mPeerFd = mSocketFd;
    }
    if (mSocketFd == -1) {
        pthread_mutex_unlock(&mMutex);
        return false;
    }
    int ret = pthread_create(&mThreadHdr, NULL, handleThreadFunc, this);
    if (ret != 0) {
        pthread_mutex_unlock(&mMutex);
        return false;
    }
    pthread_mutex_unlock(&mMutex);
    return true;
}
bool LinuxTransfer::connectionRelease() {
    LOG_D("connection release...");
    pthread_mutex_lock(&mMutex);
    if (mThreadHdr == 0) {
        LOG_E("close but thread hdr is null");
        pthread_mutex_unlock(&mMutex);
        return true;
    }
    pthread_kill(mThreadHdr, SIGUSR1);
    pthread_join(mThreadHdr, nullptr);
    if (mIsServer) {
        if (mPeerFd != -1) {
            close(mPeerFd);
            close(mSocketFd);
            mPeerFd = -1;
            mSocketFd = -1;
        }
    } else {
        if (mSocketFd != -1) {
            close(mSocketFd);
            mSocketFd = -1;
            mPeerFd = -1;
        }
    }
    pthread_mutex_unlock(&mMutex);
    return true;
}
void LinuxTransfer::peerAdd(int peer) {
    assert(mEpollFd != -1);
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = peer;
    epoll_ctl(mEpollFd, EPOLL_CTL_ADD, peer, &event);
    mPeerFd = peer;
}
void LinuxTransfer::peerRemove(int peer) {
    assert(mPeerFd == peer);
    epoll_ctl(mEpollFd, EPOLL_CTL_DEL, peer, nullptr);
    mPeerFd = -1;
}
void LinuxTransfer::socketHandler(int fd) {
    if (fd == mPeerFd) {
        // LOG_D("Peer Message");
        uint8_t buf[1024];
        int ret = read(fd, buf, sizeof(buf));
        if (ret == 0) {
            LOG_D("peer leave");
            peerRemove(fd);
            close(fd);
            mPeerFd = -1;
            if (transmitFileFd) {
                LOG_W("file still transmit, close it..");
                fclose(transmitFileFd);
                transmitFileFd = nullptr;
            }
            if (!mIsServer) {
                mSocketFd = -1;
            }
        } else if (ret > 0) {
            int32_t handleNum = streamInput(buf, ret);
            if (handleNum != ret) {
                LOG_E("unexpected: handle num not match");
            }
        }
    } else if (fd == mSocketFd) {
        struct sockaddr_in addr;
        socklen_t addrLen = sizeof(addr);
        mPeerFd = accept(mSocketFd, (sockaddr *)&addr, &addrLen);
        char ipaddr[50];
        inet_ntop(AF_INET, &(addr.sin_addr.s_addr), ipaddr, sizeof(ipaddr));
        LOG_D("accept socket: %s, %d", ipaddr, addr.sin_port);
        peerAdd(mPeerFd);
    }
}
int32_t LinuxTransfer::streamOutput(const uint8_t *buffer, int32_t length) {
    LOG_D("stream output %p, %d", buffer, length);
    if (length <= 0) {
        LOG_E("streamOutput error");
        return -1;
    }
    if (mPeerFd != -1) {
        int32_t sentBytes = 0;
        while (sentBytes < length) {
            int ret = native_safe_write(mPeerFd, buffer + sentBytes,
                                        length - sentBytes);
            if (ret == -1 && errno != EAGAIN) {
                LOG_E("streamOutput write error");
                return -1;
            } else if (ret > 0) {
                sentBytes += ret;
            } else {
                return -1;
            }
        }
    }
    return length;
}
bool LinuxTransfer::onFilePullRequestReq(const char *filename) {
    super::SuperTransferReponse response;
    transmitFileFd = fopen(filename, "rb");
    if (transmitFileFd == nullptr) {
        response.status = super::STS_FILE_NOT_EXIST;
    } else {
        response.status = super::STS_OK;
    }
    sendSuperMessage(super::SUPER_ID_PULL_FILE_RSP, &response,
                     sizeof(response));
    if (transmitFileFd) {
        sendSuperMessage(super::SUPER_ID_FILE_TRANSFER_BEGIN_REQ, nullptr, 0);
    }
    return true;
}
bool LinuxTransfer::onFilePushRequestReq(const char *filename) {
    super::SuperTransferReponse response;
    if (transmitFileFd) {
        LOG_W("a previous push Request is running");
        fclose(transmitFileFd);
        transmitFileFd = nullptr;
    }
    transmitFileFd = fopen(filename, "wb+");
    if (transmitFileFd == nullptr) {
        response.status = super::STS_FILE_OPEN_FAILED;
    } else {
        response.status = super::STS_OK;
    }
    sendSuperMessage(super::SUPER_ID_PUSH_FILE_RSP, &response,
                     sizeof(response));
    return true;
}
bool LinuxTransfer::onFileTransferDataReq(const uint8_t *buffer,
                                          int32_t length) {
    super::SuperTransferReponse response;
    response.status = super::STS_OK;
    if (!transmitFileFd) {
        response.status = super::STS_FILE_REQUEST_EMPTY;
    } else if (length > 0) {
        fwrite(buffer, 1, length, transmitFileFd);
    } else {
        response.status = super::STS_FILE_REQUEST_PARAMETER_ERROR;
    }
    sendSuperMessage(super::SUPER_ID_FILE_TRANSFER_DATA_RSP, &response,
                     sizeof(response));
    return true;
}
bool LinuxTransfer::onFileTransferDataRsp(
    const SuperTransferReponse *response) {
    assert(transmitFileFd != nullptr);
    if (response->status != super::STS_OK) {
        LOG_E("data transfer rsp: %d", response->status);
        sendSuperMessage(super::SUPER_ID_FILE_TRANSFER_FINISH_REQ, nullptr, 0);
        return true;
    }
    processFileData();
    return true;
}
bool LinuxTransfer::onFileTransferBeginRsp(
    const SuperTransferReponse *response) {
    assert(transmitFileFd != nullptr);
    if (response->status != super::STS_OK) {
        sendSuperMessage(super::SUPER_ID_FILE_TRANSFER_FINISH_REQ, nullptr, 0);
        return true;
    }
    processFileData();
    return true;
}
void LinuxTransfer::processFileData() {
    char buffer[FILE_TRANSMIT_STEP];
    size_t ret = fread(buffer, 1, FILE_TRANSMIT_STEP, transmitFileFd);
    if ((int32_t)ret == 0) {
        sendSuperMessage(super::SUPER_ID_FILE_TRANSFER_FINISH_REQ, nullptr, 0);
    } else {
        sendSuperMessage(super::SUPER_ID_FILE_TRANSFER_DATA_REQ, buffer, ret);
    }
}
bool LinuxTransfer::onFileTransferBeginReq() {
    super::SuperTransferReponse response;
    response.status =
        transmitFileFd ? super::STS_OK : super::STS_FILE_REQUEST_EMPTY;
    sendSuperMessage(super::SUPER_ID_FILE_TRANSFER_BEGIN_RSP, &response,
                     sizeof(response));
    return true;
}
bool LinuxTransfer::onFileTransferFinishReq() {
    super::SuperTransferReponse response;
    response.status = super::STS_OK;
    if (transmitFileFd) {
        fclose(transmitFileFd);
    }
    transmitFileFd = nullptr;
    sendSuperMessage(super::SUPER_ID_FILE_TRANSFER_FINISH_RSP, &response,
                     sizeof(response));
    return true;
}
bool LinuxTransfer::onFileTransferFinishRsp(
    const SuperTransferReponse *response) {
    (void)response;
    fclose(transmitFileFd);
    transmitFileFd = nullptr;
    return true;
}
bool LinuxTransfer::onFileListQueryReq(const char *dirname) {
    LOG_D("file list query req: %s", dirname);
    super::SuperCompressVariant variant;
    DIR *d = opendir(dirname);
    if (d == NULL) {
        LOG_E("=====Init SD Card: read dir error(%d) =====", errno);
        return false;
    }
    dirent *dic;
    variant.pushString(dirname);
    while ((dic = readdir(d)) != NULL) {
        LOG_D("Dir:%s", dic->d_name);
        if (strcmp(dic->d_name, ".") == 0) {
            continue;
        }
        if (strcmp(dic->d_name, "..") == 0) {
            continue;
        }
        int32_t filenameLength = strlen(dic->d_name) + 1;  // add '\0'
        super::SuperMsgFileInfo *info = (super::SuperMsgFileInfo *)malloc(
            sizeof(super::SuperMsgFileInfo) + filenameLength);
        info->filenameLength = filenameLength;
        LOG_D("info addr %p, filename %p", info, info->filename);
        if (dic->d_type == DT_DIR) {
            info->type = super::FT_DIR;
        } else if (dic->d_type == DT_REG) {
            info->type = super::FT_FILE;
        } else {
            info->type = super::FT_OTHER;
        }
        memcpy(info->filename, dic->d_name, (size_t)filenameLength);
        variant.pushBytesArray(
            info, sizeof(super::SuperMsgFileInfo) + filenameLength);
        free(info);
    }
    closedir(d);
    sendSuperMessage(super::SUPER_ID_QUERY_FILE_LIST_RSP, variant.getBuffer(),
                     variant.getLength());
    return true;
}
bool LinuxTransfer::onFileListQueryRsp(const void *buffer, int32_t length) {
    super::SuperCompressVariant variant;
    variant.setData(buffer, length);
    void *p = malloc(257);  // max is 256(filename) + 1 type
    while (variant.getBytesArray(p, 257)) {
        super::SuperMsgFileInfo *info = (super::SuperMsgFileInfo *)p;
        LOG_D("query file :%s, name length: %d, type: %d", info->filename,
              info->filenameLength, info->type);
    }
    free(p);
    return true;
}
bool LinuxTransfer::onOtaTriggerReq() {
    PlatformEventLoop::getInstance()->sendMessage(
        PlatformEventLoop::EVENT_OTA_DOWNLOAD_FIRMWARE, nullptr, 0);
    sendSuperMessage(super::SUPER_ID_OTA_TRIGGER_RSP, nullptr,
                     0);
    return true;
}

bool LinuxTransfer::sendFilePushRequest(const char *remoteFilename,
                                        const char *localFile) {
    if (transmitFileFd) {
        LOG_E("%s a previous request is handle", __FUNCTION__);
        return false;
    }
    transmitFileFd = fopen(localFile, "rb");
    if (transmitFileFd == nullptr) {
        LOG_E("open local file %s error", localFile);
        return false;
    }
    super::SuperMsgFileReq req;
    strncpy(req.filename, remoteFilename, sizeof(req.filename));
    req.fileNameLength = strlen(remoteFilename);
    sendSuperMessage(super::SUPER_ID_PUSH_FILE_REQ, &req,
                     sizeof(super::SuperMsgFileReq));
    return true;
}
bool LinuxTransfer::sendFilePullRequest(const char *remoteFilename,
                                        const char *localFile) {
    if (transmitFileFd) {
        LOG_E("%s a previous request is handle", __FUNCTION__);
        return false;
    }
    transmitFileFd = fopen(localFile, "wb+");
    if (transmitFileFd == nullptr) {
        LOG_E("open local file %s error", localFile);
        return false;
    }
    super::SuperMsgFileReq req;
    strncpy(req.filename, remoteFilename, sizeof(req.filename));
    req.fileNameLength = strlen(remoteFilename);
    sendSuperMessage(super::SUPER_ID_PULL_FILE_REQ, &req,
                     sizeof(super::SuperMsgFileReq));
    return true;
}
bool LinuxTransfer::sendFileListQueryRequest(const char *dirName) {
    sendSuperMessage(super::SUPER_ID_QUERY_FILE_LIST_REQ, dirName,
                     strlen(dirName) + 1);
    return true;
}
bool LinuxTransfer::onFilePushRequestRsp(const SuperTransferReponse *response) {
    (void)response;
    if (transmitFileFd) {
        sendSuperMessage(super::SUPER_ID_FILE_TRANSFER_BEGIN_REQ, nullptr, 0);
    } else {
        // Why Comes here?
        LOG_E("unexpected push request rsp");
    }
    return true;
}