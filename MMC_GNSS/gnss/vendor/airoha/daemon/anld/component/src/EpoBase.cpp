#include "EpoBase.h"
#include <stdio.h>
#include <inttypes.h>
#include <time.h>
#include "simulation.h"
using base::EpoReader;
using base::EpoSegment;
EpoReader::Result EpoReader::decodeEpoFile(const char *filename, int64_t leap_second,
                            bool skipHeader) {
    LOG_D("EpoReader decode: %s, %" PRId64 ", %d", filename, leap_second,
          skipHeader);
    Result res;
    res.valid = false;
    FILE *f = fopen(filename, "rb");
    if (!f) {
        return res;
    }
    fseek(f, 0, SEEK_END);
    long pos = ftell(f);
    if (pos % 72 != 0 || pos == 0) {
        LOG_E("File size is not divisible by 72 or not valid(%ld)", pos);
        fclose(f);
        return res;
    }
    fseek(f, 0, SEEK_SET);
    if (skipHeader) {
        fseek(f, 72, SEEK_SET);
    }
    res.valid = true;
    EpoSegment seg;
    // get start utc
    fread(&seg, sizeof(seg), 1, f);
    res.start = Timestamp::fromSecSinceEpoch(seg.gpsHour * 3600 + 315964800 -
                                             leap_second);
    uint32_t lastGpsHour = 0;
    while (fread(&seg, sizeof(seg), 1, f) > 0) {
        if (seg.gpsHour > lastGpsHour) {
            lastGpsHour = seg.gpsHour;
        }
    }
    LOG_D("end gps time: %" PRId64, (int64_t)(lastGpsHour * 3600 + 315964800));
    res.end = Timestamp::fromSecSinceEpoch(lastGpsHour * 3600 + 315964800 -
                                           leap_second);
    fclose(f);
    return res;
}
EpoReader::Timestamp EpoReader::Timestamp::fromTm(const struct tm *t) {
    Timestamp ts;
    ts.year = t->tm_year + 1900;
    ts.month = t->tm_mon + 1;
    ts.day = t->tm_mday;
    ts.hour = t->tm_hour;
    ts.minute = t->tm_min;
    ts.second = t->tm_sec;
    time_t tSec = timegm(const_cast<struct tm *>(t));
    ts.secondSinceEpoch = tSec;
    return ts;
}
EpoReader::Timestamp EpoReader::Timestamp::fromSecSinceEpoch(time_t sec) {
    struct tm t;
    gmtime_r(&sec, &t);
    return fromTm(&t);
}