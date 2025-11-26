INCLUDE(FindPkgConfig)
INCLUDE(GNUInstallDirs)

PROJECT(alsaucm-conf C)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.8)
SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

SET(PROJECT_NAME "ALSA UCM BINS")
SET(PROJECT_PRETTY_NAME "ALSA UCM BINS")
SET(PROJECT_DESCRIPTION "AUDIO COMPONENT TO STORAGE ALSA UCM BIN FILES")
SET(PROJECT_VERSION "0.1")

############################################################
if (DUNFELL_SUPPORT STREQUAL "true")
    SET(CARDDRIV ${CARDDRIV}-snd-card)
endif()

if (LXC_SUPPORT STREQUAL "true")
    if (HYP_SUPPORT STREQUAL "true")
        INSTALL(DIRECTORY lxc-gvm-adp-star-snd-card/
        DESTINATION ${UCMDIR}/${CARDDRIV})
    else ()
        if (TARGET_BOARD_PLATFORM STREQUAL "sa6155")
            INSTALL(DIRECTORY lxc-sa6155-adp-star-snd-card/
            DESTINATION ${UCMDIR}/${CARDDRIV})
        elseif (TARGET_BOARD_PLATFORM STREQUAL "sa8295")
            INSTALL(DIRECTORY lxc-sa8295-adp-star-snd-card/
            DESTINATION ${UCMDIR}/${CARDDRIV})
        elseif (TARGET_BOARD_PLATFORM STREQUAL "lemans")
            if (AR_SUPPORT STREQUAL "true")
                INSTALL(DIRECTORY auto8255-adp-star-snd-card/ucm-conf/
                DESTINATION ${UCMDIR}/${CARDDRIV})

                INSTALL(DIRECTORY auto8255-adp-star-snd-card/acdb-blob/
                DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/acdb-blob)
            endif()
        else ()
            INSTALL(DIRECTORY sa8155-adp-star-snd-card/
            DESTINATION ${UCMDIR}/${CARDDRIV})
        endif()
    endif()
else ()
    if (HYP_SUPPORT STREQUAL "true")
        if (TARGET_BOARD_PLATFORM STREQUAL "qtiquingvm8295")
            INSTALL(DIRECTORY sa8295-adp-star-snd-card/
            DESTINATION ${UCMDIR}/${CARDDRIV})
        elseif (TARGET_BOARD_PLATFORM STREQUAL "quin-gvm-gen4" OR TARGET_BOARD_PLATFORM STREQUAL "quin-gvm-gen4-2")
            INSTALL(DIRECTORY gvmauto8295-adp-star-snd-card/ucm-conf/
            DESTINATION ${UCMDIR}/${CARDDRIV})

            INSTALL(DIRECTORY gvmauto8295-adp-star-snd-card/gvmauto8295-adp-pp-control/
            DESTINATION ${UCMDIR_PPCTR}/${PPCTRDRIV})

            INSTALL(DIRECTORY gvmauto8295-adp-star-snd-card/acdb-blob/
            DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/acdb-blob)

            INSTALL(DIRECTORY gvmauto8295-adp-star-snd-card/acdb-blob-multiclient/
            DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/acdb-blob-multiclient)

            INSTALL(DIRECTORY gvmauto8295-adp-star-snd-card/audio-pp-blob/
            DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/audio-pp-blob)
        elseif (TARGET_BOARD_PLATFORM STREQUAL "quin-gvm-lemans")
            INSTALL(DIRECTORY gvmauto8255-adp-star-snd-card/ucm-conf/
            DESTINATION ${UCMDIR}/${CARDDRIV})

            INSTALL(DIRECTORY gvmauto8255-adp-star-snd-card/gvmauto8255-adp-pp-control/
            DESTINATION ${UCMDIR_PPCTR}/${PPCTRDRIV})

            INSTALL(DIRECTORY gvmauto8255-adp-star-snd-card/acdb-blob/
            DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/acdb-blob)

            INSTALL(DIRECTORY gvmauto8255-adp-star-snd-card/acdb-blob-multiclient/
            DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/acdb-blob-multiclient)

            INSTALL(DIRECTORY gvmauto8255-adp-star-snd-card/audio-pp-blob/
            DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/audio-pp-blob)
        else ()
            INSTALL(DIRECTORY sa8155-adp-star-snd-card/
            DESTINATION ${UCMDIR}/${CARDDRIV})
        endif()
    else ()
        if (TARGET_BOARD_PLATFORM STREQUAL "sa6155")
            INSTALL(DIRECTORY sa6155-adp-star-snd-card/
            DESTINATION ${UCMDIR}/${CARDDRIV})
        elseif (TARGET_BOARD_PLATFORM STREQUAL "sa8295")
            if (AR_SUPPORT STREQUAL "true")
                INSTALL(DIRECTORY auto8295-adp-star-snd-card/ucm-conf/
                DESTINATION ${UCMDIR}/${CARDDRIV})

                INSTALL(DIRECTORY auto8295-adp-star-snd-card/acdb-blob/
                DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/acdb-blob)
            else ()
                INSTALL(DIRECTORY sa8295-adp-star-snd-card/
                DESTINATION ${UCMDIR}/${CARDDRIV})
            endif()
        elseif (TARGET_BOARD_PLATFORM STREQUAL "lemans")
            if (AR_SUPPORT STREQUAL "true")
                INSTALL(DIRECTORY auto8255-adp-star-snd-card/ucm-conf/
                DESTINATION ${UCMDIR}/${CARDDRIV})

                INSTALL(DIRECTORY auto8255-adp-star-snd-card/acdb-blob/
                DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/acdb-blob)
            endif()
        elseif (TARGET_BOARD_PLATFORM STREQUAL "monaco")
            if (AR_SUPPORT STREQUAL "true")
                INSTALL(DIRECTORY auto7255-adp-star-snd-card/ucm-conf/
                DESTINATION ${UCMDIR}/${CARDDRIV})

                INSTALL(DIRECTORY auto7255-adp-star-snd-card/acdb-blob/
                DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/acdb-blob)
            endif()
        else ()
            INSTALL(DIRECTORY sa8155-adp-star-snd-card/
            DESTINATION ${UCMDIR}/${CARDDRIV})
        endif()
    endif()
endif()




