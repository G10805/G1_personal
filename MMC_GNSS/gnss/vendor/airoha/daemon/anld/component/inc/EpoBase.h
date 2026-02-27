#ifndef EPO_BASE_H
#define EPO_BASE_H
#include <stdint.h>
#include <time.h>
namespace base {
struct EpoSegment {
    uint32_t gpsHour : 24;
    uint32_t svid : 8;
    uint8_t data[64];
    uint8_t checksum[4];
} __attribute__((__packed__));
static_assert(sizeof(EpoSegment) == 72, "EpoSegment Size Error");
struct EpoReader {
public:

   struct Timestamp {
       uint16_t year;
       uint8_t month;
       uint8_t day;
       uint8_t hour;
       uint8_t minute;
       uint8_t second;
       int64_t secondSinceEpoch;
       static Timestamp fromTm(const struct tm *t);
       static Timestamp fromSecSinceEpoch(time_t sec);
   };
   struct Result {
       bool valid;
       Timestamp start;
       Timestamp end;
   };
   static Result decodeEpoFile(const char *filename, int64_t leap_second,
                               bool skipHeader);

};
}  // namespace base
#endif