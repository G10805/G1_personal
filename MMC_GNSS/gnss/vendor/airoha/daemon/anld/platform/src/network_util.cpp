#define LOG_TAG "NET"
#include "network_util.h"
#include <assert.h>
#include <curl/curl.h>
#include "simulation.h"
#define CURL_DISABLE_VERIFY_PEER
bool Network::sIsCurlInit = false;
std::mutex Network::sMutex;
#define MAX_FILE_NAME_LENGTH 256
static size_t WriteCbFunc(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t sizeWritten = fwrite(ptr, size, nmemb, (FILE *)stream);
    return sizeWritten;
}
NetworkStatus Network::download(const char *url, const char *filename) {
    CURL *tCurl;
    tCurl = curl_easy_init();
    NetworkStatus ret = NET_STATUS_DOWNLOAD_FAILED;
    char tmpFilename[MAX_FILE_NAME_LENGTH] = {0};
    snprintf(tmpFilename, sizeof(tmpFilename), "%s.tmp", filename);
    // LOG_D("download url: %s, filename: %s", url, filename);
    LOG_D("temp file : %s", tmpFilename);
    TRACE_D("netdl, %s, %s", url, filename);
    if (tCurl) {
        FILE *downloadFile = fopen(tmpFilename, "wb+");
        if (downloadFile == NULL) {
            curl_easy_cleanup(tCurl);
            return NET_STATUS_IO_ERROR;
        }
        // LOG_W("downloadurl: %s",downloadURL.c_str());
        curl_easy_setopt(tCurl, CURLOPT_URL, url);
        curl_easy_setopt(tCurl, CURLOPT_TIMEOUT, 10);
        curl_easy_setopt(tCurl, CURLOPT_WRITEFUNCTION, WriteCbFunc);
        curl_easy_setopt(tCurl, CURLOPT_WRITEDATA, downloadFile);
#ifdef CURL_DISABLE_VERIFY_PEER
        curl_easy_setopt(tCurl, CURLOPT_SSL_VERIFYPEER, false);
#endif
        CURLcode res = curl_easy_perform(tCurl);
        long httpFlag = 0;
        curl_easy_getinfo(tCurl, CURLINFO_RESPONSE_CODE, &httpFlag);
        fclose(downloadFile);
        if (res == CURLE_OK) {
            if (httpFlag == 200) {
                ret = NET_STATUS_OK;
                LOG_D("download %s success ret code:%ld ", filename, httpFlag);
            } else {
                LOG_E("download %s failed!!! ret code:%ld", filename, httpFlag);
                ret = NET_STATUS_DOWNLOAD_FAILED;
            }
        } else {
            LOG_E("download %s failed!!! %d, %s", filename, res,
                  curl_easy_strerror(res));
            ret = NET_STATUS_DOWNLOAD_FAILED;
        }
        TRACE_D("curl, %s, %s, %d, %ld", url, filename, res, httpFlag);
        curl_easy_cleanup(tCurl);
    } else {
        ret = NET_STATUS_CURL_INIT_ERROR;
    }
    if (ret == NET_STATUS_OK) {
        LOG_D("download pass, copy result: %d",
        copyFile(filename, tmpFilename));
    }
    unlink(tmpFilename);
    TRACE_D("netdl, %s, %s, %d", url, filename, ret);
    return ret;
}
void Network::init() {
    sMutex.lock();
    LOG_D("==== network init ===== (%d)", sIsCurlInit);
    if (!sIsCurlInit) {
        CURLcode ret = curl_global_init(CURL_GLOBAL_DEFAULT);
        assert(ret == CURLE_OK);
        sIsCurlInit = true;
    }
    sMutex.unlock();
}
void Network::deinit() {
    sMutex.lock();
    LOG_D("==== network deinit ===== (%d)", sIsCurlInit);
    if (sIsCurlInit) {
        curl_global_cleanup();
        sIsCurlInit = false;
    }
    sMutex.unlock();
}
bool Network::copyFile(const char *dest, const char *source) {
    FILE *fDst = fopen(dest, "wb+");
    FILE *fSrc = fopen(source, "rb");
    if (!(fDst && fSrc)) {
        LOG_E("open file fail!!! %p, %p", fDst, fSrc);
        if (fDst) {
            fclose(fDst);
        }
        if (fSrc) {
            fclose(fSrc);
        }
        return false;
    }
    char buf[4096];
    size_t readBytes;
    while ((readBytes = fread(buf, 1, sizeof(buf), fSrc)) > 0) {
        fwrite(buf, 1, readBytes, fDst);
    }
    fclose(fDst);
    fclose(fSrc);
    return true;
}
