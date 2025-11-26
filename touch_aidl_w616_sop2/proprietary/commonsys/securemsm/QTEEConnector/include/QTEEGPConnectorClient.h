/********************************************************************
 * Copyright (c) 2017-2019,2021, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/

#ifndef CLIENT_INCLUDE_QTEEGPCONNECTORCLIENT_H_
#define CLIENT_INCLUDE_QTEEGPCONNECTORCLIENT_H_

#include <utils/Log.h>
#include <vendor/qti/hardware/qteeconnector/1.0/IGPApp.h>
#include <vendor/qti/hardware/qteeconnector/1.0/IGPAppConnector.h>
#include <aidl/vendor/qti/hardware/qteeconnector/BnGPApp.h>
#include <aidl/vendor/qti/hardware/qteeconnector/BnGPAppConnector.h>
#include <mutex>
#include <string>
#include <vector>
#include "QSEEComAPI.h"

namespace QTEE {

using IGPAppHidl = ::vendor::qti::hardware::qteeconnector::V1_0::IGPApp;
using IGPAppConnectorHidl = ::vendor::qti::hardware::qteeconnector::V1_0::IGPAppConnector;
using ::vendor::qti::hardware::qteeconnector::V1_0::QTEECom_ion_fd_info;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_death_recipient;
using android::sp;
using android::wp;
using IGPAppAidl = ::aidl::vendor::qti::hardware::qteeconnector::IGPApp;
using IGPAppConnectorAidl = ::aidl::vendor::qti::hardware::qteeconnector::IGPAppConnector;
using ::ndk::SpAIBinder;

/**
 * @brief Helper class for connection via IGPApp/IGPAppConnector
 *
 * This class wraps the IGPAppConnector/IGPApp hwbinder interface used to load and to communicate
 * with
 * GP applications. It should be transparent to the GPTEE environment.
 */
class QTEEGPConnectorClient {
  /**
   * @brief Notifier if the service dies
   *
   * If the service dies, the notifier tries to re-establish the connection.
   */
  struct QTEEGPDeathNotifier : hidl_death_recipient {
    QTEEGPDeathNotifier(QTEEGPConnectorClient& QTEEGPc);
    virtual void serviceDied(uint64_t cookie, android::wp<IBase> const& who);
    QTEEGPConnectorClient& mQTEEGPc;
  };

 public:
  QTEEGPConnectorClient() = delete;
  /**
   * @brief create a QTEEGPConnectorClient
   *
   * The constructor tries to establish a connection to the service.
   *
   * @note The size of the shared memory can be overridden by the daemon, if the
   * QSEE Application is configured to require a larger memory than here requested.
   *
   * @note The requested size MUST take into account alignment and padding of all the
   * command and response buffers which are going to be used in subsequent calls to
   * openSession, invokeCommand or closeSession.
   *
   * @param[in] path path in the HLOS file system where the QSEE App is located
   * @param[in] name name of the QSEE application to be loaded
   * @param[in] requestedSize size of the shared memory associated with the QSEE Application
   */
  QTEEGPConnectorClient(std::string const& path, std::string const& name, uint32_t requestedSize);

  /**
   * @brief Destructor
   *
   * If the app is still loaded, the client attempts to unload the application.
   */
  virtual ~QTEEGPConnectorClient();

  /**
   * @brief load the GP application
   *
   * @return true on success, false otherwise
   */
  bool load();

  /**
  * @brief load the GP application
  *
  * @return true on success, false otherwise
  */
  bool load(android::status_t &err);

  /**
   * @brief unload the GP application
   *
   * @return true on success, false otherwise
   */
  void unload();
  /**
   * @brief unload the trusted application
   *
   * @return true on success, false otherwise
   */
  void unloadHidl();
  /**
   * @brief unload the trusted application
   *
   * @return true on success, false otherwise
   */
  void unloadAidl();
  /**
   * @brief check whether the loaded app is a 64bit application
   *
   * @return true if the loaded app is 64bit, false otherwise
   */
  bool isApp64() const { return mIsApp64; }

  /**
   * @brief invoke a command in the GP application

   * @param[in] req the request buffer
   * @param[in] reqLen length of the request buffer
   * @param[out] rsp the response buffer
   * @param[in] rspLen length of the response buffer
   * @param[in] info a description of the memory references shared with the trusted application
   *
   * @return android::OK on success, errorcode otherwise
   */
  android::status_t invokeCommand(void const* req, uint32_t reqLen, void* rsp, uint32_t rspLen,
                                  struct QSEECom_ion_fd_info const* info);

  /**
   * @brief invoke a command in the GP application

   * @param[in] req the request buffer
   * @param[in] reqLen length of the request buffer
   * @param[out] rsp the response buffer
   * @param[in] rspLen length of the response buffer
   * @param[in] info a description of the memory references shared with the trusted application
   *
   * @return android::OK on success, errorcode otherwise
   */
  android::status_t invokeCommandAidl(void const* req, uint32_t reqLen, void* rsp, uint32_t rspLen,
                                  struct QSEECom_ion_fd_info const* info);

  /**
   * @brief invoke a command in the GP application

   * @param[in] req the request buffer
   * @param[in] reqLen length of the request buffer
   * @param[out] rsp the response buffer
   * @param[in] rspLen length of the response buffer
   * @param[in] info a description of the memory references shared with the trusted application
   *
   * @return android::OK on success, errorcode otherwise
   */
  android::status_t invokeCommandHidl(void const* req, uint32_t reqLen, void* rsp, uint32_t rspLen,
                                  struct QSEECom_ion_fd_info const* info);


  /**
   * @brief open a session to a GP application

   * @param[in] req the request buffer
   * @param[in] reqLen length of the request buffer
   * @param[out] rsp the response buffer
   * @param[in] rspLen length of the response buffer
   * @param[in] info a description of the memory references shared with the trusted application
   *
   * @return android::OK on success, errorcode otherwise
   */
  android::status_t openSession(void const* req, uint32_t reqLen, void* rsp, uint32_t rspLen,
                                struct QSEECom_ion_fd_info const* info);

  /**
   * @brief open a session to a GP application using AIDL

   * @param[in] req the request buffer
   * @param[in] reqLen length of the request buffer
   * @param[out] rsp the response buffer
   * @param[in] rspLen length of the response buffer
   * @param[in] info a description of the memory references shared with the trusted application
   *
   * @return android::OK on success, errorcode otherwise
   */
  android::status_t openSessionAidl(void const* req, uint32_t reqLen, void* rsp, uint32_t rspLen,
                                struct QSEECom_ion_fd_info const* info);

  /**
   * @brief open a session to a GP application using HIDL

   * @param[in] req the request buffer
   * @param[in] reqLen length of the request buffer
   * @param[out] rsp the response buffer
   * @param[in] rspLen length of the response buffer
   * @param[in] info a description of the memory references shared with the trusted application
   *
   * @return android::OK on success, errorcode otherwise
   */
  android::status_t openSessionHidl(void const* req, uint32_t reqLen, void* rsp, uint32_t rspLen,
                                struct QSEECom_ion_fd_info const* info);

  /**
   * @brief close a session with a GP application

   * @param[in] req the request buffer
   * @param[in] reqLen length of the request buffer
   * @param[out] rsp the response buffer
   * @param[in] rspLen length of the response buffer
   *
   * @return android::OK on success, errorcode otherwise
   */
  android::status_t closeSession(void const* req, uint32_t reqLen, void* rsp, uint32_t rspLen);

  /**
   * @brief close a session with a GP application using AIDL

   * @param[in] req the request buffer
   * @param[in] reqLen length of the request buffer
   * @param[out] rsp the response buffer
   * @param[in] rspLen length of the response buffer
   *
   * @return android::OK on success, errorcode otherwise
   */
  android::status_t closeSessionAidl(void const* req, uint32_t reqLen, void* rsp, uint32_t rspLen);

  /**
   * @brief close a session with a GP application using HIDL

   * @param[in] req the request buffer
   * @param[in] reqLen length of the request buffer
   * @param[out] rsp the response buffer
   * @param[in] rspLen length of the response buffer
   *
   * @return android::OK on success, errorcode otherwise
   */
  android::status_t closeSessionHidl(void const* req, uint32_t reqLen, void* rsp, uint32_t rspLen);

  /**
   * @brief request a cancellation
   * @param[in] sessionId the session id to be cancelled
   *
   * @return android::OK on success, errorcode otherwise
   */
  android::status_t requestCancellation(uint32_t sessionId);

  /**
   * @brief request a cancellation using AIDL
   * @param[in] sessionId the session id to be cancelled
   *
   * @return android::OK on success, errorcode otherwise
   */
  android::status_t requestCancellationAidl(uint32_t sessionId);

  /**
   * @brief request a cancellation using HIDL
   * @param[in] sessionId the session id to be cancelled
   *
   * @return android::OK on success, errorcode otherwise
   */
  android::status_t requestCancellationHidl(uint32_t sessionId);

  /**
   * @brief attempt to recover the connection to the Hidl service
   *
   * @return true if recovery succeeded, false otherwise
   */
  bool recoverHidl();
  /**
   * @brief attempt to recover the connection to the Aidl service
   *
   * @return true if recovery succeeded, false otherwise
   */
  bool recoverAidl();
  /**
   * @brief load the applications dependencies
   *
   * @return true on success, false otherwise
   */
  bool loadDependencies();
  std::mutex mutable mQTEEmutex;           ///< a mutex for all calls to the service

  private:
  /**
   * @brief load the hidl application
   *
   * @return true on success, false otherwise
   */
  bool doLoadHidl();
    /**
   * @brief load the aidl application
   *
   * @return true on success, false otherwise
   */
  bool doLoadAidl();

  /**
  * @brief load the application
  *
  * @return true on success, false otherwise
  */
  bool doLoad(android::status_t &err);
  /**
   * @brief Return the aidl service instance
   *
   * @return service object on success, nullptr otherwise
   */
  std::shared_ptr<IGPAppConnectorAidl> getServiceAidl();

  ::ndk::ScopedAIBinder_DeathRecipient mDeathRecipientAidl;
  sp<IGPAppConnectorHidl> mGPAppConnectorHidl;  ///< a strongpointer to the IGPAppConnector interface
  std::shared_ptr<IGPAppConnectorAidl> mGPAppConnectorAidl;  ///< a strongpointer to the IAppConnector interface
  sp<IGPAppHidl> mGPAppHidl;  ///< a strongpointer representing the application at the service side
  std::shared_ptr<IGPAppAidl> mGPAppAidl;  ///< a strongpointer representing the application at the service side
  sp<QTEEGPDeathNotifier> mDeathNotifier;  ///< the death notifier
  ::ndk::SpAIBinder mGPAppConnectorBinder;

  std::string const mPath;        ///< the path of the GP application
  std::string const mName;        ///< the name of the GP application
  uint32_t const mRequestedSizeHidl;  ///< the originally requested buffer size
  int32_t mRequestedSizeAidl;  ///< the originally requested buffer size
  bool mIsApp64;                  ///< whether the application is 64bit
  bool mLoaded;                   ///< whether the application is loaded
};
};  // namespace QTEE

#endif  // CLIENT_INCLUDE_QTEEGPCONNECTORCLIENT_H_

