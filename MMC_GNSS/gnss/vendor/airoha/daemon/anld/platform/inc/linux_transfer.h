#ifndef LINUX_TRANSFER_H
#define LINUX_TRANSFER_H
#include <pthread.h>
#include "super_transfer.h"
using super::SuperTransfer;
using super::SuperTransferReponse;
class LinuxTransfer : public SuperTransfer {
 public:
    LinuxTransfer(const char *ip, uint16_t port, bool isServer);
    ~LinuxTransfer();
    bool setServer();
    bool sendFilePushRequest(const char *remoteFilename, const char *localFile);
    bool sendFilePullRequest(const char *remoteFilename, const char *localFile);
    bool sendFileListQueryRequest(const char *dirName);
 protected:
    bool connectionSetup() override;
    bool connectionRelease() override;
    int32_t streamOutput(const uint8_t *buffer, int32_t length) override;
    bool onFileTransferBeginReq() override;
    bool onFileTransferBeginRsp(const SuperTransferReponse *response) override;
    // bool onFileTransferEnd(const char *filename) override;
    bool onFileTransferDataReq(const uint8_t *buffer, int32_t length) override;
    bool onFileTransferDataRsp(const SuperTransferReponse *response) override;
    bool onFilePullRequestReq(const char *filename) override;
    bool onFilePushRequestReq(const char *filename) override;
    bool onFilePushRequestRsp(const SuperTransferReponse *response) override;
    bool onFileTransferFinishReq() override;
    bool onFileTransferFinishRsp(const SuperTransferReponse *response) override;
    bool onFileListQueryReq(const char *dirname) override;
    bool onFileListQueryRsp(const void *buffer, int32_t length) override;
    bool onOtaTriggerReq() override;

 private:
    enum FileTransmitStatus {
        FILE_TRANS_IDLE,
        FILE_TRANS_SEND_FILE,
        FILE_TRANS_RECEIVE_FILE,
    };
    pthread_t mThreadHdr;
    void sendStop();
    static void *handleThreadFunc(void *);
    int mSignalFd;
    int mSocketFd;
    pthread_mutex_t mMutex;
    bool mIsServer;
    char mIpaddr[50];
    int mPeerFd;
    int mEpollFd;
    int mLocalMsgFdServer;
    int mLocalMsgFdClient;
    uint16_t mPort;
    int createSocketFd();
    static int createTcpClient(const char *ip, uint16_t port);
    static int createTcpServer(const char *ip, uint16_t port,
                               int supportClientt);
    static int createTCPSocket(const char *abstractName, int supportClient);
    static int createSignalFd();
    void peerAdd(int peer);
    void peerRemove(int peer);
    void socketHandler(int fd);
    void processFileData();
    FILE *transmitFileFd;
};
#endif