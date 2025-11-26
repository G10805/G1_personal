pkgconfigdir = $(libdir)/pkgconfig
#pkgconfig_DATA = mm_audio_headers.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS = -I ${top_srcdir}/acdbdata
AM_CFLAGS += -I ${top_srcdir}/audio_log_utils
AM_CFLAGS += -I ${top_srcdir}/audio-codecs/flac
AM_CFLAGS += -I ${top_srcdir}/audio-listen
AM_CFLAGS += -I $(top_srcdir)/capiv2_api

mm_audio_headers_sources = ${top_srcdir}/acdbdata/kvh2xml.h \
              ${top_srcdir}/audio_log_utils/log_utils.h \
              ${top_srcdir}/audio_log_utils/log_xml_parser.h \
              ${top_srcdir}/audio-codecs/flac/FLACDec_API.h \
              ${top_srcdir}/audio-codecs/flac/FLACDec_BitStream.h \
              ${top_srcdir}/audio-codecs/flac/FLACDec_Common.h \
              ${top_srcdir}/audio-codecs/flac/FLACDec_MetaData.h \
              ${top_srcdir}/audio-codecs/flac/FLACDec_Riff.h \
              ${top_srcdir}/audio-codecs/flac/FLACDec_Struct.h \
              ${top_srcdir}/audio-codecs/flac/FLACDec_typedefs.h \
              ${top_srcdir}/audio-codecs/flac/FLACDec_Wrapper.h \
              ${top_srcdir}/audio-listen/ListenSoundModelLib.h \
              ${top_srcdir}/audio-listen/lsm_server_wrapper.h \
              ${top_srcdir}/capiv2_api/capi_v2_events.h \
              ${top_srcdir}/capiv2_api/capi_v2_extn.h \
              ${top_srcdir}/capiv2_api/capi_v2_properties.h \
              ${top_srcdir}/capiv2_api/capi_v2_types.h \
              ${top_srcdir}/capiv2_api/capi_v2.h \
              ${top_srcdir}/capiv2_api/mmdefs.h

lib_includedir = $(includedir)
lib_include_HEADERS = $(mm_audio_headers_sources)
