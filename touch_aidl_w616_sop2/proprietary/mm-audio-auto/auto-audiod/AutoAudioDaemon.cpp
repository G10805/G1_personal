/*
 * Copyright (c) 2019-2021, 2023-2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2012-2016, 2019 The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "AutoAudioDaemon"
#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0

#include <dirent.h>
#include <sys/poll.h>
#include <cutils/properties.h>

#include "AutoAudioDaemon.h"
#include "auto_audio_ext.h"
#include "auto_audio_ext_v2.h"

#ifndef DEFAULT_AUDIO_FRAMEWORK
#define DEFAULT_AUDIO_FRAMEWORK "\0"
#endif

#define MAX_SLEEP_RETRY 100
#define AUDIO_INIT_SLEEP_WAIT 100 /* 100 ms */
#define SND_CARD_SLEEP_WAIT 100 /* 10 ms */

static bool bootup_complete = false;
static char g_audio_framework[PROPERTY_VALUE_MAX];

namespace android {

    static void place_marker(char const *name)
    {
        int fd=open("/sys/kernel/boot_kpi/kpi_values", O_WRONLY);
        if (fd > 0)
        {
            /* Only allow marker text shorter than MARKER_STRING_WIDTH */
            char earlyapp[100] = {0};
            strlcpy(earlyapp, name, sizeof(earlyapp));
            write(fd, earlyapp, strlen(earlyapp));
            close(fd);
        }
    }

    AutoAudioDaemon::AutoAudioDaemon()
#ifdef AHAL_EXT
            : mAudioHalDeathRecipient(new AudioHalDeathRecipient(this))
#endif
    {
        property_get("ro.boot.audio", g_audio_framework, DEFAULT_AUDIO_FRAMEWORK);
        if (!strcmp(g_audio_framework, "audioreach"))
            auto_audio_ar_ext_init();
        else
            auto_audio_ext_init();
    }

    AutoAudioDaemon::~AutoAudioDaemon() {
        if (!strcmp(g_audio_framework, "audioreach"))
            auto_audio_ar_ext_deinit();
        else
            auto_audio_ext_deinit();
    }

    bool AutoAudioDaemon::getSndCardFDs(std::vector<snd_card_fd> &sndcardFds)
    {
        FILE *fp = NULL;
        int fd = 0;
        char *ptr = NULL, *saveptr = NULL, *card_id = NULL;
        char buffer[128] = {0};
        int line = 0;
        String8 path;
        const char* cards = "/proc/asound/cards";
        struct snd_card_fd localFDs = {0, 0, 0};

        if ((fp = fopen(cards, "r")) == NULL) {
            ALOGE("Cannot open %s file to get list of sound cars", cards);
            return false;
        }

        sndcardFds.clear();

        while ((fgets(buffer, sizeof(buffer), fp) != NULL)) {
            if (line++ % 2)
                continue;

            ptr = strtok_r(buffer, " [", &saveptr);
            if (!ptr)
                continue;

            card_id = strtok_r(saveptr+1, "]", &saveptr);
            if (!card_id)
                continue;

            //Only consider sound cards associated with ADSP
            if ((strncasecmp(card_id, "msm", 3) != 0) &&
                (strncasecmp(card_id, "apq", 3) != 0) &&
                (strncasecmp(card_id, "sa", 2) != 0)) {
                ALOGD("Skipping non-ADSP sound card %s", card_id);
                continue;
            }

            if (ptr) {
                localFDs.snd_card = atoi(ptr);
#ifdef KERNEL_515
                path = "/sys/kernel/snd_card/";
                path += "card_state";
#else
                path = "/proc/asound/card";
                path += ptr;
                path += "/state";
#endif

#ifdef AOSP_UPGRADE_CHANGES
                ALOGD("Opening sound card state : %s", path.c_str());
                fd = open(path.c_str(), O_RDONLY);
                if (fd == -1) {
                    ALOGE("Open %s failed : %s", path.c_str(), strerror(errno));
                    continue;
                }
#else
                ALOGD("Opening sound card state : %s", path.string());
                fd = open(path.string(), O_RDONLY);
                if (fd == -1) {
                    ALOGE("Open %s failed : %s", path.string(), strerror(errno));
                    continue;
                }
#endif
                localFDs.state = fd;

#ifdef LINUX_ENABLED
/*if LV platform, no need to monitor power node
disable audiod power related, need figure out new solutiona
*/
#else
#ifndef VHAL_EXT
                path.clear();
                path = "/proc/asound/card";
                path += ptr;
                path += "/power";

#ifdef AOSP_UPGRADE_CHANGES
                ALOGD("Opening sound card power : %s", path.c_str());
                fd = open(path.c_str(), O_RDONLY);
                if (fd == -1) {
                    ALOGE("Open %s failed : %s", path.c_str(), strerror(errno));
                    close(localFDs.state);
                    continue;
                }
#else
                ALOGD("Opening sound card power : %s", path.string());
                fd = open(path.string(), O_RDONLY);
                if (fd == -1) {
                    ALOGE("Open %s failed : %s", path.string(), strerror(errno));
                    close(localFDs.state);
                    continue;
                }

#endif
                /* returns vector of pair<snd_card_fd> */
                localFDs.power = fd;
#endif
#endif
                sndcardFds.push_back(localFDs);
                auto_audio_ext_set_snd_card(localFDs.snd_card);
            }
        }

        ALOGV("%s: %d sound cards detected", __func__, (int)sndcardFds.size());
        fclose(fp);

        return sndcardFds.size() > 0 ? true : false;
    }

    void AutoAudioDaemon::putSndCardFDs(std::vector<snd_card_fd> &sndcardFds)
    {
        unsigned int i = 0;

        for (i = 0; i < sndcardFds.size(); i++) {
            close(sndcardFds[i].state);
#ifndef VHAL_EXT
            close(sndcardFds[i].power);
#endif
        }
        sndcardFds.clear();
    }

#ifndef VHAL_EXT
    /* non-class member::helper functions */
    static bool isSndCardOnline(struct snd_card_fd *fd)
    {
        char rd_buf[9] = {0};
        ssize_t bytes = 0;
        bool ret = false;

        bytes = read(fd->state, (void *)rd_buf, 8);
        if (bytes == -1) {
            ALOGE("Error reading sound card %d state (%s)",
                fd->snd_card, strerror(errno));
        } else if (bytes == 0) {
            ALOGE("EOF reading sound card %d state",
                fd->snd_card);
        } else {
            rd_buf[8] = '\0';
            lseek(fd->state, 0, SEEK_SET);

            ALOGV("snd card state: %s", rd_buf);
#ifdef KERNEL_515
            if (atoi(rd_buf))
#else
            if (strstr(rd_buf, "ONLINE"))
#endif
                ret = true;
        }
        return ret;
    }

    /* non-class member::static worker thread */
    static void *workerThread(void *args)
    {
        notify_status cur_power_state = POWER_D0;
        notify_status prev_power_state = POWER_D0;
        char rd_buf[9] = {0};
        ssize_t bytes = 0;
        struct snd_card_fd *localFDs = (struct snd_card_fd *)args;

        ALOGV("Start workerThread()");

        while (1) {
            ALOGV("ready to read() power on snd card %d", localFDs->snd_card);
            bytes = read(localFDs->power, (void *)rd_buf, 8);
            ALOGV("out of read() power");

            if (bytes == -1) {
                ALOGE("Error reading sound card %d power: %d (%s)",
                    localFDs->snd_card, errno, strerror(errno));
                if (errno == EINTR) {
                    /* We are not locking wakeup here since user space processes
                     * are already being freezed. */
                    ALOGV("Disable hostless due to interrupt signal");
                    if (isSndCardOnline(localFDs))
                        auto_audio_ext_disable_hostless(localFDs->snd_card);
                    prev_power_state = POWER_D3hot;
                }
            } else if (bytes == 0) {
                ALOGE("EOF reading sound card %d power",
                    localFDs->snd_card);
            } else {
                wakelock_acquire();
                rd_buf[8] = '\0';
                lseek(localFDs->power, 0, SEEK_SET);

                ALOGV("sound card power: %s", rd_buf);
                if (strstr(rd_buf, "D3hot"))
                    cur_power_state = POWER_D3hot;
                else if (strstr(rd_buf, "D0"))
                    cur_power_state = POWER_D0;
                else
                    ALOGE("unknown snd card power: %s", rd_buf);

                if (prev_power_state != cur_power_state) {
                    if (cur_power_state == POWER_D3hot) {
                        if (isSndCardOnline(localFDs))
                            auto_audio_ext_disable_hostless(localFDs->snd_card);
                    }
                    else if (cur_power_state == POWER_D0) {
                        /* read snd card state */
                        if (isSndCardOnline(localFDs))
                            auto_audio_ext_enable_hostless(localFDs->snd_card);
                    }
                }
                prev_power_state = cur_power_state;
                wakelock_release();
            }
        }

        ALOGV("Exiting workerThread()");
        return NULL;
    }
#endif
    /***/

    bool AutoAudioDaemon::threadLoop()
    {
        int i = 0, ret = 0;
        unsigned int sleepRetry = 0;
        bool audioInitDone = false;
        bool audioRestartNoti = false;
        struct pollfd *pfd = NULL;
        char rd_buf[9] = {0};
        ssize_t bytes = 0;
        notify_status cur_state = SND_CARD_OFFLINE;
        notify_status prev_state = SND_CARD_OFFLINE;

        while (audioInitDone == false && sleepRetry < MAX_SLEEP_RETRY) {
            if (mSndCardFd.empty() && !getSndCardFDs(mSndCardFd)) {
                ALOGE("Sleeping for 100 ms");
                usleep(AUDIO_INIT_SLEEP_WAIT*1000);
                sleepRetry++;
            } else {
                audioInitDone = true;
            }
        }

        if (audioInitDone == false) {
            ALOGE("Sound Card is empty!!!");
            goto thread_exit;
        } else {
            ALOGV("Start threadLoop()");
            ALOGD("Start the Silent Mode monitor");
            auto_audio_ext_silent_boot_monitor_start();
        }

#ifdef LINUX_ENABLED
/*if LV platform, no need to monitor power node
disable audiod power related, need figure out new solutiona
*/
#else
#ifndef VHAL_EXT
        pthread_t thread;
        /* Create separate workerThread for individual snd card power
         * state reading. threadLoop will poll snd card state. */
        for (i = 0; i < (int)mSndCardFd.size(); i++) {
            ret = pthread_create(&thread, (const pthread_attr_t *) NULL,
                                workerThread, (void *)&mSndCardFd[i]);
            if (ret) {
                ALOGE("Failed to create worker thread for snd card %d",
                    mSndCardFd[i].snd_card);
            } else {
                mThread.push_back(thread);
            }
        }
#endif
#endif

        pfd = new pollfd[mSndCardFd.size()];
        bzero(pfd, (sizeof(*pfd) * mSndCardFd.size()));
        for (i = 0; i < (int)mSndCardFd.size(); i++) {
            pfd[i].fd = mSndCardFd[i].state;
            pfd[i].events = POLLPRI;

#ifndef LINUX_ENABLED
        char prop_value[92];
        property_get("vendor.audio.audiod.started", prop_value, NULL);
        if (strcmp(prop_value, "true") == 0) {
            ALOGD("AudioD restart happen!");
            audioRestartNoti = true;
            bootup_complete = 1;
            if (strlen(g_audio_framework) <= 0 || !strcmp(g_audio_framework, "elite")) {
                auto_audio_ext_open_mixer(mSndCardFd[i].snd_card);
            }
            bytes = read(pfd[i].fd, (void *)rd_buf, 8);
            if (bytes < 0) {
                ALOGE("Error reading sound card %d state: %d (%s)",
                mSndCardFd[i].snd_card, errno, strerror(errno));
            } else if (bytes == 0) {
                ALOGE("EOF reading sound card %d state",
                mSndCardFd[i].snd_card);
            } else {
                rd_buf[8] = '\0';
                lseek(pfd[i].fd, 0, SEEK_SET);
                ALOGV("sound card state first : %s, bootup_complete: %d",
                    rd_buf, bootup_complete);
#ifdef KERNEL_515
                if (!atoi(rd_buf))
                    cur_state = SND_CARD_OFFLINE;
                else if (atoi(rd_buf))
                    cur_state = SND_CARD_ONLINE;
#else
                if (strstr(rd_buf, "OFFLINE"))
                    cur_state = SND_CARD_OFFLINE;
                else if (strstr(rd_buf, "ONLINE"))
                    cur_state = SND_CARD_ONLINE;
#endif
                else
                    ALOGE("unknown snd card state: %s", rd_buf);
                    if (cur_state == SND_CARD_ONLINE && prev_state == SND_CARD_OFFLINE) {
                        ALOGD("AudioD restart happen!");
                        if (!strcmp(g_audio_framework, "audioreach")) {
                            ALOGV("enable AR hostless sessions and boot_complete : %d",
                                    bootup_complete );
                            auto_audio_ext_ar_enable_hostless(mSndCardFd[i].snd_card);
                        } else {
                            for (i = 0; i < (int)mSndCardFd.size(); i++) {
                                ALOGV("enable hostless sessions and boot_complete : %d",
                                    bootup_complete );
                                auto_audio_ext_enable_hostless(mSndCardFd[i].snd_card);
                            }
                        }
                        prev_state = cur_state;
                    }
            }
        } else {
            ALOGD("Set AudioD property!");
            property_set("vendor.audio.audiod.started", "true");
            audioRestartNoti = false;
        }
#endif//LINUX_ENABLED
        }

        while (1) {
            ALOGD("poll() for sound card state change");
            if (poll(pfd, mSndCardFd.size(), -1) < 0) {
                ALOGE("poll() failed: %d (%s)", errno, strerror(errno));
                ret = false;
                break;
            }
            ALOGD("out of poll() for sound card state change");

            for (i = 0; i < (int)mSndCardFd.size(); i++) {
                if (pfd[i].revents & POLLPRI) {
                    bytes = read(pfd[i].fd, (void *)rd_buf, 8);
                    if (bytes < 0) {
                        ALOGE("Error reading sound card %d state: %d (%s)",
                            mSndCardFd[i].snd_card, errno, strerror(errno));
                    } else if (bytes == 0) {
                        ALOGE("EOF reading sound card %d state",
                            mSndCardFd[i].snd_card);
                    } else {
                        rd_buf[8] = '\0';
                        lseek(pfd[i].fd, 0, SEEK_SET);

                        ALOGV("sound card state: %s, bootup_complete: %d",
                                rd_buf, bootup_complete);
#ifdef KERNEL_515
                        if (!atoi(rd_buf))
#else
                        if (strstr(rd_buf, "OFFLINE"))
#endif
                            cur_state = SND_CARD_OFFLINE;
#ifdef KERNEL_515
                        else if (atoi(rd_buf))
#else
                        else if (strstr(rd_buf, "ONLINE"))
#endif
                            cur_state = SND_CARD_ONLINE;
                        else
                            ALOGE("unknown snd card state: %s", rd_buf);

                        if (bootup_complete && (prev_state != cur_state)) {
                            if (cur_state == SND_CARD_ONLINE) {
                                if (!strcmp(g_audio_framework, "audioreach")) {
                                    auto_audio_ext_ar_enable_hostless(mSndCardFd[i].snd_card);
                                } else {
                                    auto_audio_ext_enable_hostless(mSndCardFd[i].snd_card);
                                }
                            }

#ifdef AHAL_EXT
                            ALOGV("bootup_complete, so notifyAudioDevice");
                            notifyAudioDevice(mSndCardFd[i].snd_card, cur_state,
                                SND_CARD_STATE);
                            audioRestartNoti = false;
#endif
                            if (cur_state == SND_CARD_OFFLINE) {
                                if (!strcmp(g_audio_framework, "audioreach")) {
                                    auto_audio_ext_ar_disable_hostless(mSndCardFd[i].snd_card);
                                } else {
                                    auto_audio_ext_disable_hostless(mSndCardFd[i].snd_card);
                                }
                            }
                        } else if (audioRestartNoti) {
#ifdef AHAL_EXT
                            ALOGV("audioRestartNoti, so notifyAudioDevice");
                            notifyAudioDevice(mSndCardFd[i].snd_card, cur_state,
                                SND_CARD_STATE);
                            audioRestartNoti = false;
#endif
                        } else {
                            ALOGV("sound card status has no change");
                        }

                        if (!bootup_complete && (cur_state == SND_CARD_ONLINE)) {
                            bootup_complete = 1;
                            ALOGV("sound card bootup completed");
                            if (!strcmp(g_audio_framework, "audioreach")) {
                                place_marker("M - AudioD enable hostless start");
                                auto_audio_ext_ar_enable_hostless(mSndCardFd[i].snd_card);
                                place_marker("M - AudioD enable hostless complete");
                            } else {
                                auto_audio_ext_open_mixer(mSndCardFd[i].snd_card);
                                auto_audio_ext_enable_hostless(mSndCardFd[i].snd_card);
                            }
#ifdef VHAL_EXT
                            mAutoPower = ::ndk::SharedRefBase::make<AutoPower>();
                            mAutoPower->initialize();
#endif
                        }
                        prev_state = cur_state;
                    }
                }
            }
        }

#ifdef LINUX_ENABLED
/*if LV platform, no need to monitor power node
disable audiod power related, need figure out new solutiona
*/
#else
#ifndef VHAL_EXT
        /* main workerThread exited, try join child threads */
        for (i = 0; i < (int)mThread.size(); i++) {
            pthread_join(mThread[i], (void **) NULL);
        }
        mThread.clear();
#endif
#endif

        putSndCardFDs(mSndCardFd);

    thread_exit:
        ALOGV("Exiting ThreadLoop()");
        return true;
    }

#ifdef AHAL_EXT
    status_t AutoAudioDaemon::parametersFromStr(const String8& key,
                                const String8& value,
                                hidl_vec<ParameterValue> *hidlParams)
    {
        size_t idx = hidlParams->size();
        hidlParams->resize(idx + 1);
#ifdef AOSP_UPGRADE_CHANGES
        (*hidlParams)[idx].key = key.c_str();
        (*hidlParams)[idx].value = value.c_str();
#else
        (*hidlParams)[idx].key = key.string();
        (*hidlParams)[idx].value = value.string();
#endif
        return OK;
    }

    status_t AutoAudioDaemon::getDeviceInstance()
    {
        if (mDevicesFactory == 0) {
            mDevicesFactory = IDevicesFactory::getService();
            if (mDevicesFactory != 0) {
                mDevicesFactory->linkToDeath(mAudioHalDeathRecipient, 0 /*cookie*/);
            } else {
                ALOGE("Failed to obtain IDevicesFactory service");
                return FAILED_TRANSACTION;
            }
        }

        if (mDevice == 0) {
            Result retval = Result::NOT_INITIALIZED;
            Return<void> ret = mDevicesFactory->openPrimaryDevice(
                    [&](Result r, const sp<IDevice>& result) {
                        retval = r;
                        if (retval == Result::OK)
                            mDevice = result;
                    });
            if (ret.isOk()) {
                if (retval == Result::OK) return OK;
                else if (retval == Result::INVALID_ARGUMENTS) return BAD_VALUE;
                else return NO_INIT;
            }
            return FAILED_TRANSACTION;
        }

        return OK;
    }

    void AutoAudioDaemon::notifyAudioDevice(int snd_card,
                                            notify_status status,
                                            notify_status_type type)
    {
        String8 key, value;
        char buf[4] = {0,};
        status_t ret = OK;
        int retryCount = 0;

        do {
            if (getDeviceInstance() == OK) {
                ALOGI("%s: getDeviceInstance() OK", __func__);
                break;
            } else {
                if (++retryCount <= MAX_SLEEP_RETRY) {
                    ALOGE("%s: Sleeping for 100 ms", __func__);
                    usleep(AUDIO_INIT_SLEEP_WAIT*1000);
                } else {
                    ALOGE("%s: getDeviceInstance() FAILED", __func__);
                    return;
                }
            }
        } while (1);

        if (type == SND_CARD_STATE) {
            key = "SND_CARD_STATUS";
            snprintf(buf, sizeof(buf), "%d", snd_card);
            value = buf;
            if (status == SND_CARD_ONLINE)
                value += ",ONLINE";
            else
                value += ",OFFLINE";
        } else {
            ALOGE("%s: unknown type %d", __func__, type);
            return;
        }
#ifdef AOSP_UPGRADE_CHANGES
        ALOGV("%s: %s=%s",
            __func__, key.c_str(), value.c_str());
#else
        ALOGV("%s: %s=%s",
            __func__, key.string(), value.string());
#endif
        hidl_vec<ParameterValue> hidlParams;
        ret = parametersFromStr(key, value, &hidlParams);
        if (ret != OK)
            return;
        mDevice->setParameters(NULL, hidlParams);
    }

    void AutoAudioDaemon::handleAudioHalDeath()
    {
        int retryCount = 0;

        mDevicesFactory->unlinkToDeath(mAudioHalDeathRecipient);
        mDevicesFactory = NULL;
        mDevice = NULL;

        ALOGI("%s: Reconnecting to Audio HAL", __func__);
        do {
            if (getDeviceInstance() == OK) {
                ALOGI("%s: getDeviceInstance() OK", __func__);
                break;
            } else {
                if (++retryCount <= MAX_SLEEP_RETRY) {
                    ALOGE("%s: Sleeping for 100 ms", __func__);
                    usleep(AUDIO_INIT_SLEEP_WAIT*1000);
                } else {
                    ALOGE("%s: getDeviceInstance() FAILED", __func__);
                    return;
                }
            }
        } while (1);
        ALOGI("%s: Audio HAL Reconnected", __func__);
    }

    void AutoAudioDaemon::binderDied(const wp<IBinder> &the_late_who __unused) {
        ALOGE("%s: Binder died, need to clean up", __func__);
    }

    // ----------------------------------------------------------------------------

    void AutoAudioDaemon::AudioHalDeathRecipient::serviceDied(uint64_t cookie __unused,
            const wp<hidl::base::V1_0::IBase>& who __unused) {

        ALOGI("Audio HAL Died");
        mAutoAudioDaemon->handleAudioHalDeath();
    }
#endif

#ifdef LINUX_ENABLED
    void AutoAudioDaemon::binderDied(const wp<IBinder> &the_late_who __unused) {
        ALOGE("%s: Binder died, need to clean up", __func__);
    }
#endif

}
