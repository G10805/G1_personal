#define LOG_TAG "INF"
#include "IHidlTransportInterface.h"
#include <assert.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/epoll.h>
#include "native_call.h"
#include "simulation.h"
#include "transport/air_parcel.h"
#include "udp_protocol/udp_protocol.h"
using namespace udp_protocol;
using airoha::IHidlTransportInterface;
using Airoha::Gnss::PreAGnssDataConnectionConfig;
using ModId = Airoha::IPC::msg_module_id_t;
using MessageAGps = Airoha::IPC::ANLD_AGPS_MESSAGE;
using MessageNavigation = Airoha::IPC::ANLD_Nav_MESSAGE;
using MessageGeofencing = Airoha::IPC::ANLD_GEOFENCE_MESSAGE;
using MessageSystem = Airoha::IPC::ANLD_SYSTEM_MESSAGE;
void IHidlTransportInterface::sTransactionThreadCallback(void *argv) {
    LOG_I("Visteon IHidlTransportInterface Thread Start.");
    IHidlTransportInterface *inf = (IHidlTransportInterface *)argv;
    while (true) {
        if (inf->loop() == -1) {
            break;
        }
    }
    LOG_I("Visteon IHidlTransportInterface Thread Exit.");
}
void IHidlTransportInterface::sReconnectTimerCallback(union sigval val) {
    IHidlTransportInterface *inf = (IHidlTransportInterface *)val.sival_ptr;
    inf->sendIlmMessage(MESSAGE_TYPE_RECONNECT, nullptr, 0);
}
void IHidlTransportInterface::sendIlmMessage(MessageType type, const void *data,
                                             size_t length) {
    Message msg;
    msg.type = type;
    (void)data;
    (void)length;
    native_safe_write(mSocketCtrl[1], &msg, sizeof(msg));
}
int IHidlTransportInterface::loop() {
    LOG_E("Visteon triggered loop func");
    struct epoll_event evt[5];
    memset(evt, 0, sizeof(evt));
    int r = epoll_wait(mEpollFd, evt, 5, -1);
    if (r == -1) {
        if (errno == EINTR) {
            return 0;
        }
        LOG_E("ANLD service incorrect=[%s]", strerror(errno));
        return -1;
    }
    if (r == 0) {
        return 0;
    }
    bool exitFlag = false;
    for (int i = 0; i < r; i++) {
        if (evt[i].data.fd == mSocketCtrl[0]) {
            uint8_t buf[128] = {0};
            LOG_E("Visteon triggered native_safe_read");
            native_safe_read(mSocketCtrl[0], buf, sizeof(buf));
            if (handleIlmMessage((const Message *)buf) == -1) {
                exitFlag = true;
            }
        } else if (evt[i].data.fd == mSocketFd) {
            LOG_E("Visteon triggered native_safe_read else part");
            ssize_t retCode =
                native_safe_read(mSocketFd, getWrite(), canWrite());
            if (retCode > 0) {
                commitWroteBytes(retCode);
                parserIpcData();
            } else if (retCode == 0) {
                // Peer disconnected
                {
                    std::lock_guard<std::recursive_mutex> locker(mMutex);
                    close(mSocketFd);
                    mSocketFd = -1;
                }
                startReconnectTimer(1000);
                Bufferable::clear();
                onDisconnected();
            } else {
                LOG_E("Unexpected error: %zd %s", retCode, strerror(errno));
                // Ignore
            }
        }
    }
    LOG_E("Visteon loop func exit");
    if (exitFlag) return -1;
    return 0;
}
int IHidlTransportInterface::startup() {
    mEpollFd = epoll_create(5);
    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, mSocketCtrl) == -1) {
        return -1;
    }
    addFd(mSocketCtrl[0]);
    sendIlmMessage(MessageType::MESSAGE_TYPE_RECONNECT, nullptr, 0);
    return 0;
}
int IHidlTransportInterface::teardown() {
    close(mEpollFd);
    if (mSocketCtrl[0] > 0) {
        close(mSocketCtrl[0]);
    }
    if (mSocketCtrl[1] > 0) {
        close(mSocketCtrl[1]);
    }
    return 0;
}
bool IHidlTransportInterface::addFd(int fd) {
    LOG_D("static: add to epoll %d", fd);
    struct epoll_event e_event;
    memset(&e_event, 0, sizeof(e_event));
    e_event.data.fd = fd;
    e_event.events = EPOLLIN;
    int ret = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &e_event);
    if (ret == -1) {
        LOG_E("epoll ctl error(static), fd=%d, epollfd=%d,err=%d,%s", fd,
              mEpollFd, errno, strerror(errno));
        return false;
    }
    return true;
}
bool IHidlTransportInterface::delFd(int fd) {
    struct epoll_event e_event;
    memset(&e_event, 0, sizeof(e_event));
    e_event.data.fd = fd;
    e_event.events = EPOLLIN;
    int ret = epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, &e_event);
    if (ret == -1) {
        LOG_E("epoll del error, fd=%d, epollfd=%d,err=%d,%s", fd, mEpollFd,
              errno, strerror(errno));
        return false;
    }
    return true;
}
void IHidlTransportInterface::startReconnectTimer(int timeoutMs) {
    if (mTimer == nullptr) {
        mTimer = airoha::timer::create_timer(sReconnectTimerCallback, this);
    }
    airoha::timer::start_timer(mTimer, timeoutMs);
}
void IHidlTransportInterface::stopReconnectTimer() {
    if (mTimer) {
        airoha::timer::stop_timer(mTimer);
    }
}
IHidlTransportInterface::IHidlTransportInterface(const char *abstractSocketName)
    : Bufferable(kMaxBufferSize) {
    mSocketName = abstractSocketName;
}
int IHidlTransportInterface::handleIlmMessage(const Message *msg) {
    switch (msg->type) {
        case MessageType::MESSAGE_TYPE_EXIT: {
            {
                std::lock_guard<std::recursive_mutex> locker(mMutex);
                close(mSocketFd);
                mSocketFd = -1;
                stopReconnectTimer();
            }
            return -1;
        }
        case MessageType::MESSAGE_TYPE_RECONNECT: {
            if (connectServer() == -1) {
                startReconnectTimer(1000);
            }
            break;
        }
        default:
            LOG_W("Unhandle message %d", msg->type);
            break;
    }
    return 0;
}
bool IHidlTransportInterface::startTransaction() {
    startup();
    LOG_E("Visteon triggered startTransaction");
    mThread = std::thread(sTransactionThreadCallback, this);
    LOG_E("Visteon exit startTransaction");
    return true;
}
bool IHidlTransportInterface::stopTransaction() {
    sendIlmMessage(MessageType::MESSAGE_TYPE_EXIT, nullptr, 0);
    mThread.join();
    teardown();
    return true;
}
int IHidlTransportInterface::connectServer() {
    std::lock_guard<std::recursive_mutex> locker(mMutex);
    if (mSocketFd != -1) {
        LOG_W("Reconnect but socket is not -1(%d), maybe open?", mSocketFd);
    }
    mSocketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (mSocketFd == -1) {
        LOG_E("Fail to create socket: %d, %s", errno, strerror(errno));
        return -1;
    }
    struct sockaddr_un un_s;
    memset(&un_s, 0, sizeof(un_s));
    un_s.sun_family = AF_UNIX;
    un_s.sun_path[0] = '\0';  // abstract
    // change from strcpy to snprintf due to cpplint
    snprintf(&(un_s.sun_path[1]), sizeof(un_s.sun_path) - 1, "%s",
             HIDL2ANLD_FD);
    if (connect(mSocketFd, reinterpret_cast<struct sockaddr *>(&un_s),
                sizeof(un_s)) < 0) {
        LOG_E("connect error!! %d %s", errno, strerror(errno));
        close(mSocketFd);
        mSocketFd = -1;
        return -1;
    }
    int flags = fcntl(mSocketFd, F_GETFL, 0);
    flags = flags | O_NONBLOCK;
    if (fcntl(mSocketFd, F_SETFL, flags) < 0) {
        LOG_E("fcntl error %d %s", errno, strerror(errno));
        close(mSocketFd);
        mSocketFd = -1;
        return -1;
    }
    LOG_D("Connect ANLD successfully.");
    addFd(mSocketFd);
    registerClient();
    onConnected();
    return 0;
}
anld_status_t IHidlTransportInterface::registerClient() {
    Airoha::IPC::anld_sys_payload_t regInfo;
    memcpy(regInfo.name, "HIDL", 5);
    regInfo.utc = time(0);
    LOG_D("Register Client");
    return transmit(ModId::ANLD_MSG_MODULE_SYSTEM,
                    MessageSystem::ANLD_SYS_REGISTER, &regInfo,
                    sizeof(Airoha::IPC::anld_sys_payload_t));
}
IHidlTransportInterface ::~IHidlTransportInterface() {}
void IHidlTransportInterface::parserIpcData() {
    LOG_E("Visteon triggered parserIpcData");
    size_t offset = 0;
    anld_status_t ret = Airoha::IPC::decode_anld_hidl_message_v2a(
        getRead(), canRead(),
        [&](msg_module_id_t moduleId, uint32_t messageId, const uint8_t *data,
            size_t len) {
            handleModuleMessage(moduleId, messageId, data, len);
        },
        &offset);
    // ALOGD("decode ret %d", ret);
    if (ret == ANLD_STATUS_DATA_PACKET_FORMAT_ERROR) {
        LOG_E("packet format error, clear all ");
        Bufferable::clear();
    } else if (ret == ANLD_STATUS_PREAMBLE_NOT_FOUND) {
        LOG_E("preamble not found clear all");
        Bufferable::clear();
    } else if (ret == ANLD_STATUS_DATA_PACKET_INCOMPLETED) {
        LOG_E("packet imcomplete, wait...%zu/%zu", Bufferable::hasWritten(),
              offset);
        if (offset > 0) {
            Bufferable::commitReadBytes(offset);
            Bufferable::realign();
        }
    }
    if (ret == ANLD_STATUS_OK) {
        // ALOGD("packet parser done clear");
        Bufferable::clear();
    }
    // ALOGD("receive buffer len :%zu", revBuffer.size());
    if (Bufferable::canWrite() == 0) {
        LOG_E("Visteon receive buffer too long, clear all");
        Bufferable::clear();
    }
}
void IHidlTransportInterface::handleModuleMessage(msg_module_id_t moduleId,
                                                  uint32_t messageId,
                                                  const uint8_t *data,
                                                  size_t length) {
    LOG_E("Visteon triggered handleModuleMessage");
    switch (moduleId) {
        case ModId::ANLD_MSG_MODULE_SYSTEM:
            LOG_E("Visteon triggered ANLD_MSG_MODULE_SYSTEM"); 
            handleModuleMessageSystem(moduleId, messageId, data, length);
            break;
        case ModId::ANLD_MSG_MODULE_POWER:
            LOG_E("Visteon triggered ANLD_MSG_MODULE_POWER");
            handleModuleMessagePower(moduleId, messageId, data, length);
            break;
        case ModId::ANLD_MSG_MODULE_NAVIGATION:
            LOG_E("Visteon triggered ANLD_MSG_MODULE_NAVIGATION");
            handleModuleMessageNavigation(moduleId, messageId, data, length);
            break;
        case ModId::ANLD_MSG_MODULE_AGPS:
            LOG_E("Visteon triggered ANLD_MSG_MODULE_AGPS");
            handleModuleMessageAGps(moduleId, messageId, data, length);
            break;
        default:
            break;
    }
}
void IHidlTransportInterface::handleModuleMessageSystem(
    msg_module_id_t moduleId, uint32_t messageId, const uint8_t *data,
    size_t length) {
    (void)moduleId;
    LOG_E("Visteon triggered handleModuleMessageSystem");
    (void)messageId;
    (void)length;
    using Airoha::IPC::ANLD_SYSTEM_MESSAGE;
    assert(moduleId == ModId::ANLD_MSG_MODULE_SYSTEM);
    switch (messageId) {
        case ANLD_SYSTEM_MESSAGE::ANLD_SYS_REQUEST_HIDL_SYNC: {
            LOG_E("Visteon triggered ANLD_SYS_REQUEST_HIDL_SYNC");
            LOG_D("receive sync req");
            syncRequestInternal();
            onSyncRequest();
            break;
        }
        case ANLD_SYSTEM_MESSAGE::ANLD_SYS_HARDWARE_VERSION_REPORT: {
            LOG_E("Visteon triggered ANLD_SYS_HARDWARE_VERSION_REPORT");
            const Airoha::IPC::HardwareVersion *ver =
                (const Airoha::IPC::HardwareVersion *)data;
            onHardwareVersionReport(ver);
            break;
        }
        default:
            break;
    }
}
void IHidlTransportInterface::handleModuleMessageNavigation(
    msg_module_id_t moduleId, uint32_t messageId, const uint8_t *data,
    size_t length) {
    (void)moduleId;
    LOG_E("Visteon triggered handleModuleMessageNavigation");
    using NavMessageId = Airoha::IPC::ANLD_Nav_MESSAGE;
    switch (messageId) {
        case NavMessageId::ANLD_NAV_LOCATION_OUTPUT:
            LOG_E("Visteon triggered ANLD_NAV_LOCATION_OUTPUT");
            onLocationOutput(reinterpret_cast<const PreGnssLocation *>(data));
            /* code */
            break;
        case NavMessageId::ANLD_NAV_SV_STATUS_OUTPUT:
            LOG_E("Visteon triggered ANLD_NAV_SV_STATUS_OUTPUT");
            onSvStatusOutput(reinterpret_cast<const PreGnssSvStatus *>(data));
            break;
        case NavMessageId::ANLD_NAV_NMEA_OUTPUT:
            LOG_E("Visteon triggered ANLD_NAV_NMEA_OUTPUT");
            onNmeaOutput(reinterpret_cast<const NavNmea *>(data));
            break;
        case NavMessageId::ANLD_RAW_MEASUREMENT_GNSSDATA:
            LOG_E("Visteon triggered ANLD_RAW_MEASUREMENT_GNSSDATA");
            onGnssMeasurement(reinterpret_cast<const PreGnssData *>(data));
            break;
        case NavMessageId::ANLD_NAV_GFESTATUS_OUTPUT:
            LOG_E("Visteon triggered  ANLD_NAV_GFESTATUS_OUTPUT");
            onGnssGeofencingStatusOutput(
                reinterpret_cast<const PreGeofenceData *>(data));
            break;
        case NavMessageId::ANLD_NAVIGATION_MESSAGE_DATA:
            LOG_E("Visteon triggered ANLD_NAVIGATION_MESSAGE_DATA");
            onGnssNavigationData(
                reinterpret_cast<const PreGnssNavigationMessage *>(data));
            break;
        case NavMessageId::ANLD_NAV_REQUEST_LOCATION: {
            LOG_E("Visteon triggered ANLD_NAV_REQUEST_LOCATION");
            onRequestLocationAiding();
            break;
        }
        case NavMessageId::ANLD_NAV_REQUEST_TIME: {
            LOG_E("Visteon triggered ANLD_NAV_REQUEST_TIME");
            onRequestTimeAiding();
            break;
        }
        case NavMessageId::ANLD_NAVIGATION_REPORT_CAPABILITIES:
            onReportGnssCapabilities(
                reinterpret_cast<const PreGnssCapabilities *>(data));
            break;
        case NavMessageId::ANLD_NAV_BATCHING_LOCATION_OUTPUT: {
            LOG_E("Visteon triggered ANLD_NAV_BATCHING_LOCATION_OUTPUTE");
            if (length % sizeof(PreGnssLocation) != 0) break;
            std::vector<PreGnssLocation> locations;
            PreGnssLocation *ptr = (PreGnssLocation *)data;
            size_t num = length / sizeof(PreGnssLocation);
            for (size_t i = 0; i < num; i++) {
                locations.push_back(ptr[i]);
            }
            onBatchLocationReport(locations);
            break;
        }
        case NavMessageId::ANLD_NAV_GNSS_DEBUG_DATA: {
            LOG_E("Visteon triggered ANLD_NAV_GNSS_DEBUG_DATA");
            onGnssDebugData(reinterpret_cast<const PreDebugData *>(data));
            break;
        }
        case NavMessageId::ANLD_NAV_VISIBILITY_CONTROL_NOTIFY: {
            LOG_E("Visteon triggered ANLD_NAV_VISIBILITY_CONTROL_NOTIFY");
            using Airoha::Gnss::PreNfwNotification;
            using Airoha::Gnss::PreNfwProtocolStack;
            using Airoha::Gnss::PreNfwRequestor;
            using Airoha::Gnss::PreNfwResponseType;
            airoha::Parcel reader(data, (int32_t)length);
            PreNfwNotification notification;
            notification.proxyAppPackageName = reader.readString();
            notification.protocolStack =
                static_cast<PreNfwProtocolStack>(reader.readInt32());
            notification.otherProtocolStackName = reader.readString();
            notification.requestor =
                static_cast<PreNfwRequestor>(reader.readInt32());
            notification.requestorId = reader.readString();
            notification.responseType =
                static_cast<PreNfwResponseType>(reader.readInt32());
            notification.inEmergencyMode = reader.readBool();
            notification.isCachedLocation = reader.readBool();
            onNfwNotification(notification);
            break;
        }
        default:
            break;
    }
}
void IHidlTransportInterface::handleModuleMessagePower(msg_module_id_t moduleId,
                                                       uint32_t messageId,
                                                       const uint8_t *data,
                                                       size_t length) {
    using Airoha::IPC::ANLD_POWER_MESSAGE;
    assert(moduleId == ModId::ANLD_MSG_MODULE_POWER);
    (void)moduleId;
    LOG_E("Visteon triggered handleModuleMessagePower");
    (void)data;
    (void)length;
    switch (messageId) {
        // case ANLD_POWER_MSG_POWER_STATUS_POWER_ON:{
        //     if(Gnss::getCallback()){
        //         Gnss::getCallback()->gnssStatusCb(GnssStatusValue::ENGINE_ON);
        //     }
        //     break;
        // }
        // case ANLD_POWER_MSG_POWER_STATUS_POWER_OFF:{
        //     if(Gnss::getCallback()){
        //         Gnss::getCallback()->gnssStatusCb(GnssStatusValue::ENGINE_OFF);
        //     }
        //     break;
        // }
        case ANLD_POWER_MESSAGE::ANLD_POWER_MSG_POWER_STATUS_DSP_OFF: {
            LOG_E("Visteon triggered ANLD_POWER_MSG_POWER_STATUS_DSP_OFF");
            onGnssStatusReport(false);
            break;
        }
        case ANLD_POWER_MESSAGE::ANLD_POWER_MSG_POWER_STATUS_DSP_ON: {
            LOG_E("Visteon triggered ANLD_POWER_MSG_POWER_STATUS_DSP_ON");
            onGnssStatusReport(true);
            break;
        }
    }
}
void IHidlTransportInterface::handleModuleMessageAGps(msg_module_id_t moduleId,
                                                      uint32_t messageId,
                                                      const uint8_t *data,
                                                      size_t length) {
    (void)moduleId;
    (void)length;
    using Airoha::IPC::ANLD_AGPS_MESSAGE;
    switch (messageId) {
        case ANLD_AGPS_MESSAGE::AGPS_ANLD_SET_ID_REQ: {
            const PreSetIdFlags *flagsValue = (const PreSetIdFlags *)data;
            onAGpsSetIdRequest(*flagsValue);
            break;
        }
        case ANLD_AGPS_MESSAGE::AGPS_ANLD_REF_LOC_REQ: {
            onAGpsSetRefLocationRequest();
            break;
        }
        case ANLD_AGPS_MESSAGE::AGPS_ANLD_NI_NOTIFY: {
            // Not Support for NI Notify
            onAGpsNiNotify();
            break;
        }
        case ANLD_AGPS_MESSAGE::AGPS_ANLD_DATA_CONN_REQ: {
            LOG_W("do not support data conn req after Android 10");
            onAGpsDataConnectionRequest(PreAGnssType::SUPL);
            break;
        }
        case ANLD_AGPS_MESSAGE::AGPS_ANLD_DATA_CONN_REQ2: {
            const PreAGnssDataConnectionConfig *config =
                (const PreAGnssDataConnectionConfig *)data;
            assert(config->size == sizeof(PreAGnssDataConnectionConfig));
            onAGpsDataConnectionRequestv2(config);
            break;
        }
        case ANLD_AGPS_MESSAGE::AGPS_ANLD_DATA_CONN_RELEASE: {
            const PreAGnssDataConnectionConfig *config =
                (const PreAGnssDataConnectionConfig *)data;
            assert(config->size == sizeof(PreAGnssDataConnectionConfig));
            onAGpsDataConnectionRelease(config);
            break;
        }
        default:
            break;
    }
}
// ##############################
// #    SEND   API  BEGIN       #
// ##############################
anld_status_t IHidlTransportInterface::sendSetId(int type, const char *setid) {
    char buff[ANLD_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, type);
    put_string(buff, &offset, setid);
    return transmit(ModId::ANLD_MSG_MODULE_AGPS,
                    MessageAGps::ANLD_AGPS_SEND_SETID, buff, offset);
}
anld_status_t IHidlTransportInterface::sendRefLocation(int type, int mcc,
                                                       int mnc, int lac,
                                                       int cid) {
    char buff[ANLD_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, type);
    put_int(buff, &offset, mcc);
    put_int(buff, &offset, mnc);
    put_int(buff, &offset, lac);
    put_int(buff, &offset, cid);
    return transmit(ModId::ANLD_MSG_MODULE_AGPS,
                    MessageAGps::ANLD_AGPS_SEND_REFLOC, buff, offset);
}
anld_status_t IHidlTransportInterface::sendDataConnOpen(const char *apn) {
    char buff[ANLD_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    put_string(buff, &offset, apn);
    return transmit(ModId::ANLD_MSG_MODULE_AGPS,
                    MessageAGps::ANLD_AGPS_SEND_DATACONOPEN, buff, offset);
}
anld_status_t IHidlTransportInterface::sendDataConnOpenIpType(
    const char *apn, int ip_type, bool network_handle_valid,
    uint64_t network_handle) {
    char buff[ANLD_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    put_string(buff, &offset, apn);
    put_int(buff, &offset, ip_type);
    put_int(buff, &offset, network_handle_valid);
    put_long(buff, &offset, network_handle);
    return transmit(ModId::ANLD_MSG_MODULE_AGPS,
                    MessageAGps::ANLD_AGPS_SEND_DATA_CONN_OPEN_IP_TYPE, buff,
                    offset);
}
anld_status_t IHidlTransportInterface::sendDataConnFailed(void) {
    return transmit(ModId::ANLD_MSG_MODULE_AGPS,
                    MessageAGps::ANLD_AGPS_SEND_DATACONFAIL, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendDataConnClose(void) {
    return transmit(ModId::ANLD_MSG_MODULE_AGPS,
                    MessageAGps::ANLD_AGPS_SEND_DATACONCLOSE, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendSetServer(int type, int port,
                                                     const char *hostname) {
    char buff[ANLD_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, type);
    put_int(buff, &offset, port);
    put_string(buff, &offset, hostname);
    return transmit(ModId::ANLD_MSG_MODULE_AGPS,
                    MessageAGps::ANLD_AGPS_SEND_SETSERVER, buff, offset);
}
anld_status_t IHidlTransportInterface::sendUpdateNetworkAvailability(
    int available, const char *apn) {
    char buff[ANLD_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, available);
    put_string(buff, &offset, apn);
    return transmit(ModId::ANLD_MSG_MODULE_AGPS,
                    MessageAGps::ANLD_AGPS_SEND_UPDATANETSTATESAVAILABILITY,
                    buff, offset);
}
anld_status_t IHidlTransportInterface::sendUpdateNetworkState(int connected,
                                                              int type,
                                                              int roaming) {
    char buff[ANLD_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, connected);
    put_int(buff, &offset, type);
    put_int(buff, &offset, roaming);
    return transmit(ModId::ANLD_MSG_MODULE_AGPS,
                    MessageAGps::ANLD_AGPS_SEND_UPDATANETSTATES, buff, offset);
}
anld_status_t IHidlTransportInterface::sendUpdateNetworkStateExt(
    int64_t networkHandle, bool isConnected, int32_t capabilities,
    const char *apn) {
    char buff[ANLD_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    put_long(buff, &offset, networkHandle);
    put_int(buff, &offset, isConnected);
    put_int(buff, &offset, capabilities);
    put_string(buff, &offset, apn);
    return transmit(ModId::ANLD_MSG_MODULE_AGPS,
                    MessageAGps::ANLD_AGPS_SEND_UPDATANETSTATES_EXT, buff,
                    offset);
}
anld_status_t IHidlTransportInterface::sendNiRespond(int session_id,
                                                     int user_response) {
    char buff[ANLD_AGPS_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, session_id);
    put_int(buff, &offset, user_response);
    return transmit(ModId::ANLD_MSG_MODULE_AGPS,
                    MessageAGps::ANLD_AGPS_SEND_NIRESPOND, buff, offset);
}
anld_status_t IHidlTransportInterface::sendNiMessage(const void *data,
                                                     size_t length) {
    airoha::Parcel parcel;
    parcel.writeInt32(
        transport::ParcelHashKey::PARCEL_HASH_KEY_SUPL_NI_MESSAGE);
    parcel.writeBinary(data, length);
    return transmit(ModId::ANLD_MSG_MODULE_AGPS,
                    MessageAGps::HIDL_ANLD_SUPL_NI_MESSAGE, parcel.data(),
                    parcel.size());
}
anld_status_t IHidlTransportInterface::sendAddGeofence(
    PreGnssGeofenceconfig *data) {
    return transmit(ModId::ANLD_MSG_MODULE_GEOFENCE,
                    MessageGeofencing::HAL2ANLD_GEOFENCE_ADD, data,
                    sizeof(PreGnssGeofenceconfig));
}
anld_status_t IHidlTransportInterface::sendPauseGeofence(int32_t *geofenceId) {
    return transmit(ModId::ANLD_MSG_MODULE_GEOFENCE,
                    MessageGeofencing::HAL2ANLD_GEOFENCE_PAUSE, geofenceId,
                    sizeof(int32_t));
}
anld_status_t IHidlTransportInterface::sendResumeGeofence(int32_t *geofenceId) {
    return transmit(ModId::ANLD_MSG_MODULE_GEOFENCE,
                    MessageGeofencing::HAL2ANLD_GEOFENCE_RESUME, geofenceId,
                    sizeof(int32_t));
}
anld_status_t IHidlTransportInterface::sendRemoveGeofence(int32_t *geofenceId) {
    return transmit(ModId::ANLD_MSG_MODULE_GEOFENCE,
                    MessageGeofencing::HAL2ANLD_GEOFENCE_REMOVE, geofenceId,
                    sizeof(int32_t));
}
anld_status_t IHidlTransportInterface::sendInjectTime(NavTime_t *navTime) {
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_INJECT_TIME, navTime,
                    sizeof(NavTime_t));
}
anld_status_t IHidlTransportInterface::sendInjectLocation(
    PreGnssLocation *location) {
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_INJECT_LOCATION, location,
                    sizeof(PreGnssLocation));
}
anld_status_t IHidlTransportInterface::sendDeleteAidingData(
    PreGnssAidingData *data) {
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_DELETE_AIDING_DATA, data,
                    sizeof(PreGnssAidingData));
}
anld_status_t IHidlTransportInterface::sendPositionConfiguration(
    PreGnssPosisionPreference *data) {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mRequestGnssPreference = *data;
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_PREFERENCE, data,
                    sizeof(PreGnssPosisionPreference));
}
// batching
anld_status_t IHidlTransportInterface::sendBatchingInit() {
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_BATCHING_INIT, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendBatchingStart(
    PreBatchingOptions *opt) {
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_BATCHING_START, opt,
                    sizeof(PreBatchingOptions));
}
anld_status_t IHidlTransportInterface::sendBatchingStop() {
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_BATCHING_STOP, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendBatchingFlush() {
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_BATCHING_FLUSH, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendBatchingCleanup() {
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_BATCHING_CLEANUP, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendMeasurementUpdateRequest(
    const PreGnssMeasurementOptions &options) {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mRequestMeasurement = Tristate::T_TRUE;
    mRequestMeasurementOptions = options;
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_MEAS_START, &options,
                    sizeof(PreGnssMeasurementOptions));
}
anld_status_t IHidlTransportInterface::sendClearMeasurementRequest() {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mRequestMeasurement = Tristate::T_FALSE;
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_MEAS_STOP, NULL, 0);
}
// anld_status_t sendGnssLocationStart();
// anld_status_t sendGnssLocationStop();
anld_status_t IHidlTransportInterface::sendGnssLocationStartOnly() {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mRequestGnssStart = Tristate::T_TRUE;
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_START_ONLY_POSITION, NULL,
                    0);
}
anld_status_t IHidlTransportInterface::sendGnssLocationStopOnly() {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mRequestGnssStart = Tristate::T_FALSE;
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_STOP_ONLY_POSITION, NULL,
                    0);
}
anld_status_t IHidlTransportInterface::sendGnssLocationStart() {
    mRequestGnssStart = Tristate::T_TRUE;
    mRequestNmea = Tristate::T_TRUE;
    mRequestSvStatus = Tristate::T_TRUE;
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_POSITION_START, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendGnssLocationStop() {
    mRequestGnssStart = Tristate::T_FALSE;
    mRequestNmea = Tristate::T_FALSE;
    mRequestSvStatus = Tristate::T_FALSE;
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_POSITION_STOP, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendGnssLocationInit() {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mRequestPositionInit = Tristate::T_TRUE;
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_POSITION_INIT, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendGnssLocationCleanup() {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mRequestPositionInit = Tristate::T_FALSE;
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_POSITION_CLEANUP, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendNavigationMessageStart() {
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_NAVIGATION_START, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendNavigationMessageStop() {
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_NAVIGATION_STOP, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendPsdsSetCallback() {
    mPsdsEnable = Tristate::T_TRUE;
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_PSDS_SET_CALLBACK, NULL,
                    0);
}
anld_status_t IHidlTransportInterface::closePsds() {
    mPsdsEnable = Tristate::T_FALSE;
    return anld_status_t::ANLD_STATUS_OK;
}
anld_status_t IHidlTransportInterface::sendVisibilityControlProxyApps(
    const void *parcelData, size_t length) {
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_VISIBILITY_ENABLE_PROXY_APPS,
                    parcelData, length);
}
anld_status_t IHidlTransportInterface::sendStartNmea() {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mRequestNmea = Tristate::T_TRUE;
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_START_NMEA, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendStopNmea() {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mRequestNmea = Tristate::T_FALSE;
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_STOP_NMEA, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendStartSvStatus() {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mRequestSvStatus = Tristate::T_TRUE;
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_START_SV_STATUS, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendStopSvStatus() {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mRequestSvStatus = Tristate::T_FALSE;
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_STOP_SV_STATUS, NULL, 0);
}
anld_status_t IHidlTransportInterface::sendExtData(const void *data,
                                                   size_t length) {
    return transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                    MessageNavigation::ANLD_NAV_GNSS_SEND_EXTDATA, data,
                    length);
}
// ##############################
// #    SEND   API  END         #
// ##############################
void IHidlTransportInterface::syncRequestInternal() {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    if (mRequestPositionInit == Tristate::T_TRUE) {
        transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                 MessageNavigation::ANLD_NAV_GNSS_POSITION_INIT, NULL, 0);
    } else if (mRequestPositionInit == Tristate::T_FALSE) {
        transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                 MessageNavigation::ANLD_NAV_GNSS_POSITION_CLEANUP, NULL, 0);
    }
    if (mPsdsEnable == Tristate::T_TRUE) {
        transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                 MessageNavigation::ANLD_NAV_GNSS_PSDS_SET_CALLBACK, NULL, 0);
    }
    if (mRequestGnssStart == Tristate::T_TRUE) {
        transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                 MessageNavigation::ANLD_NAV_GNSS_PREFERENCE,
                 &mRequestGnssPreference, sizeof(PreGnssPosisionPreference));
        transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                 MessageNavigation::ANLD_NAV_GNSS_START_ONLY_POSITION, NULL, 0);
    } else if (mRequestGnssStart == Tristate::T_FALSE) {
        transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                 MessageNavigation::ANLD_NAV_GNSS_STOP_ONLY_POSITION, NULL, 0);
    }
    if (mRequestMeasurement == Tristate::T_TRUE) {
        transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                 MessageNavigation::ANLD_NAV_GNSS_MEAS_START,
                 &mRequestMeasurementOptions,
                 sizeof(PreGnssMeasurementOptions));
    } else if (mRequestMeasurement == Tristate::T_FALSE) {
        transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                 MessageNavigation::ANLD_NAV_GNSS_MEAS_STOP, NULL, 0);
    }
    if (mRequestNmea == Tristate::T_TRUE) {
        transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                 MessageNavigation::ANLD_NAV_GNSS_START_NMEA, NULL, 0);
    } else if (mRequestNmea == Tristate::T_FALSE) {
        transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                 MessageNavigation::ANLD_NAV_GNSS_STOP_NMEA, NULL, 0);
    }
    if (mRequestSvStatus == Tristate::T_TRUE) {
        transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                 MessageNavigation::ANLD_NAV_GNSS_START_SV_STATUS, NULL, 0);
    } else if (mRequestSvStatus == Tristate::T_FALSE) {
        transmit(ModId::ANLD_MSG_MODULE_NAVIGATION,
                 MessageNavigation::ANLD_NAV_GNSS_STOP_SV_STATUS, NULL, 0);
    }
}
void IHidlTransportInterface::lock() { mMutex.lock(); }
void IHidlTransportInterface::unlock() { mMutex.unlock(); }
std::recursive_mutex &IHidlTransportInterface::getLock() { return mMutex; }
anld_status_t IHidlTransportInterface::transmit(msg_module_id_t mod,
                                                uint32_t message_id,
                                                const void *userdata,
                                                size_t len) {
    using Airoha::IPC::anld_message_t;
    using Airoha::IPC::anld_msg_hdr_t;
    anld_message_t msg;
    size_t anld_message_size = 0;
    size_t anld_total_size = 0;
    size_t encodeLen = 0;
    // this mutex will be unlock when function return
    anld_message_size = len + sizeof(anld_msg_hdr_t);
    uint8_t *anld_payload = reinterpret_cast<uint8_t *>(
        malloc(anld_message_size * sizeof(uint8_t)));
    anld_status_t ret;
    msg.header.module_id = mod;
    msg.header.msg_id = message_id;
    msg.userdata = userdata;
    msg.header.payload_size = len;
    ret = Airoha::IPC::encode_message_payload(anld_payload, anld_message_size,
                                              &msg, &encodeLen);
    if (ret != ANLD_STATUS_OK) {
        LOG_E("encode error , %d", ret);
        free(anld_payload);
        return ret;
    }
    if (anld_message_size != encodeLen) {
        LOG_E("encode size not match! %zu,%zu", anld_message_size, encodeLen);
        free(anld_payload);
        return ANLD_STATUS_FAIL;
    }
    anld_total_size = anld_message_size + LENGTH_FIELD_LENGTH +
                      PREAMBLE_WORD_LENGTH + END_WORD_LENGTH;
    uint8_t *anld_packet =
        reinterpret_cast<uint8_t *>(malloc(anld_total_size * sizeof(uint8_t)));
    ret = Airoha::IPC::encode_anld_hidl_message(anld_packet, anld_total_size,
                                                anld_payload, anld_message_size,
                                                &encodeLen);
    if (ret != ANLD_STATUS_OK || encodeLen != anld_total_size) {
        LOG_E("encode hidl error,ret %d,encodeLen %zu, total_len %zu", ret,
              encodeLen, anld_total_size);
        free(anld_payload);
        free(anld_packet);
        assert(0);  // just for test
        return ANLD_STATUS_FAIL;
    }
    // ALOGD("send to client %d",client->getFd());
    lock();
    ssize_t sentBytes = -1;
    if (mSocketFd > 0) {
        sentBytes = native_safe_write(mSocketFd, anld_packet, anld_total_size);
    }
    unlock();
    free(anld_payload);
    free(anld_packet);
    if (sentBytes == (ssize_t)anld_total_size) return ANLD_STATUS_OK;
    return ANLD_STATUS_FAIL;
}
