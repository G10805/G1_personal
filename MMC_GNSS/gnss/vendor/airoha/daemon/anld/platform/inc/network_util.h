#ifndef NETWORK_UTIL_H
#define NETWORK_UTIL_H
#include <unistd.h>
#include <mutex>
enum NetworkStatus {
    NET_STATUS_OK,
    NET_STATUS_DOWNLOAD_FAILED,
    NET_STATUS_CURL_INIT_ERROR,
    NET_STATUS_IO_ERROR,
};
class Network {
 public:
    static void init();
    static void deinit();
    /**
     * Method to download file
     * block: true
     *
     * \param url The url to download file
     * \param local path filename (e.g  /data/a/b)
     * \return NetworkStatus, download ok if NET_STATUS_OK, else return code is
     * error
     */
    static NetworkStatus download(const char *url, const char *filename);

 private:
    static bool copyFile(const char *dest, const char *source);
    static bool sIsCurlInit;
    static std::mutex sMutex;
};
#endif  // NETWORK_UTIL_H
