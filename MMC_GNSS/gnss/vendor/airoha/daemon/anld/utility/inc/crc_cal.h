#ifndef CRC_CAL_H_
#define CRC_CAL_H_
#include <stdint.h>
#include <stdio.h>
#include <type_traits>
namespace algorithm {
template <typename T>
class CrcCalculator {
    static_assert(std::is_same<uint32_t, T>::value ||
                  std::is_same<uint16_t, T>::value ||
                  std::is_same<uint8_t, T>::value,
                  "Only accept u8,u16,u32");

 public:
    CrcCalculator(T polynomial, T initial, T finalXor, bool refin,
                  bool refout) {
        mPolynomial = polynomial;
        mInitial = initial;
        mFinalXor = finalXor;
        mRefIn = refin;
        mRefout = refout;
        if (mRefIn) {
            genReflectTable();
        } else {
            genTable();
        }
        // printf("new instance\n");
    }
    void genTable() {
        for (uint32_t i = 0; i < 256; i++) {
            table[i] = calByteCs(mPolynomial, i);
        }
    }
    void genReflectTable() {
        for (uint32_t i = 0; i < 256; i++) {
            uint8_t x = (uint8_t)i;
            table[i] = calByteCsReverse(bitReverse(mPolynomial), x);
        }
    }
    void printTable() {
        printf("Table:\n");
        for (size_t i = 0; i < 256; i++) {
            switch (sizeof(T)) {
                case sizeof(uint8_t): {
                    printf("0x%02x ", table[i]);
                    break;
                }
                case sizeof(uint16_t): {
                    printf("0x%04x ", table[i]);
                    break;
                }
                case sizeof(uint32_t): {
                    printf("0x%08x ", table[i]);
                    break;
                }
            }
            if (i % 4 == 3) {
                printf("\n");
            }
        }
        printf("==============\n");
    }
    T calculateCrc(const void *buffer, size_t length, T initial) {
        const uint8_t *p = static_cast<const uint8_t *>(buffer);
        T crc = initial;
        if (mRefIn) {
            crc = bitReverse(crc);
        }
        for (size_t i = 0; i < length; i++) {

            uint8_t data = p[i];
            // printf("data %x\n", data);
            if (mRefIn) {
                crc = table[(crc ^ data) & 0xFF] ^ (crc >> 8);
            } else {
                crc = table[(crc >> kPadding) ^ data] ^ (crc << 8);
            }
        }
        if (mRefIn) {
            crc = bitReverse(crc);
        }
        if (mRefout) {
            crc = bitReverse(crc);
        }
        // printf("crc %x\n", crc);
        crc ^= mFinalXor;
        return crc;
    }
    T getInitialValue() {
        return mInitial;
    }

 private:
    const size_t kPadding = (sizeof(T) * 8 - 8);
    T bitReverse(T value) {
        switch (sizeof(T)) {
            case sizeof(uint8_t): {
                return bitReverseU8((uint8_t)value);
            }
            case sizeof(uint16_t): {
                return bitReverseU16((uint16_t)value);
            }
            case sizeof(uint32_t): {
                return bitReverseU32((uint32_t)value);
            }
        }
    }
    T calByteCs(T poly, uint8_t byte) {
        T crc = ((T)byte) << (sizeof(T) * 8 - 8);
        for (uint8_t i = 0; i < 8; i++) {
            if (crc & 0x80000000) {
                crc <<= 1;
                crc ^= poly;
            } else {
                crc <<= 1;
            }
        }
        return crc;
    }
    T calByteCsReverse(T poly, uint8_t byte) {
        // uint8_t tmp = 0x80;
        T crc = ((T)byte);
        // uint8_t i = 0;
        for (uint8_t i = 0; i < 8; i++) {
            // printf("crc 0x%08x\n", crc);
            if (crc & 0x1) {
                crc >>= 1;
                crc ^= poly;
            } else {
                crc >>= 1;
            }
        }
        return crc;
    }
    uint8_t bitReverseU8(uint8_t byte) {
        byte = ((byte & 0x55) << 1) | ((byte & 0xAA) >> 1);
        byte = ((byte & 0x33) << 2) | ((byte & 0xCC) >> 2);
        byte = ((byte & 0x0F) << 4) | ((byte & 0xF0) >> 4);
        return byte;
    }
    uint16_t bitReverseU16(uint16_t byte) {
        byte = ((byte & 0x5555) << 1) | ((byte & 0xAAAA) >> 1);
        byte = ((byte & 0x3333) << 2) | ((byte & 0xCCCC) >> 2);
        byte = ((byte & 0x0F0F) << 4) | ((byte & 0xF0F0) >> 4);
        byte = ((byte & 0x00FF) << 8) | ((byte & 0xFF00) >> 8);
        return byte;
    }
    uint32_t bitReverseU32(uint32_t value) {
        value = ((value & 0x55555555) << 1) | ((value & 0xAAAAAAAA) >> 1);
        value = ((value & 0x33333333) << 2) | ((value & 0xCCCCCCCC) >> 2);
        value = ((value & 0x0F0F0F0F) << 4) | ((value & 0xF0F0F0F0) >> 4);
        value = ((value & 0x00FF00FF) << 8) | ((value & 0xFF00FF00) >> 8);
        value = ((value & 0x0000FFFF) << 16) | ((value & 0xFFFF0000) >> 16);
        return value;
    }

    T mPolynomial;
    T mInitial;
    T mFinalXor;
    bool mRefIn;
    bool mRefout;
    T table[256];
};
}  // namespace algorithm
#endif