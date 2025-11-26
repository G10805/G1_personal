/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_UTILS_EVENT_H_
#define _QC2AUDIO_UTILS_EVENT_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <type_traits>

#include "QC2.h"

namespace qc2audio {

/// @addtogroup utils Utilities
/// @{

/**
 * @brief Collection of key-value pairs
 *
 * Contains a list of keyvalue pairs
 * -# keys are strings
 * -# values are of standard types (u)int(32/64)/string or objects of type Storable\n
 * Bundle provides interface to add and look-up for values for specified keys
 */
class Bundle {
 public:
    /**
     * @brief Base for a value-type that can be added to a Bundle
     *
     * Types derived from this base can be added as values to a Bundle
     */
    class Storable {
     public:
        virtual ~Storable() {}
    };

    Bundle() = default;

    /**
     * @brief generic setter to add a key-value pair
     *
     * If a pair exists with the same key, it will be overriden with new value
     * @param[in] key string represeting a key
     * @param[in] val value of supported type
     * @return QC2_CORRUPTED key-value could not be added
     * @return QC2_OK        key-value added successfully
     */
    template <typename T>
    QC2Status put(const std::string& key, T val) {
        auto itr = mMap.find(key);
        if (itr != mMap.end()) {
            mMap.erase(itr);
        }
        auto ret = mMap.emplace(key, Item(val));
        if (!ret.second) {
            QLOGE("failed to emplace : %s", key.c_str());
            return QC2_CORRUPTED;
        }
        return QC2_OK;
    }

    /**
     * @brief generic getter for native types
     *
     * This method supports native types (u)int(32/64)/string
     * @param[in]  key string represeting a key
     * @param[out] val value for the key
     * @return QC2_BAD_VALUE invalid reference to value passed
     * @return QC2_BAD_INDEX key does not exist
     * @return QC2_OK        value retrieved successfully
     */
    template <typename T>
    QC2Status get(const std::string& key, T *val) const {
        if (val == nullptr) {
            return QC2_BAD_VALUE;
        }
        auto itr = mMap.find(key);
        if (itr != mMap.end()) {
            return itr->second.assign(val);
        } else {
            return QC2_BAD_INDEX;  // key not found
        }
    }

    /**
     * @brief specialized getter for objects of Storable type
     *
     * This method supports retrieving objects of types derived from Storable
     * @param[in]  key string represeting a key
     * @param[out] val shared reference for the value for the key
     * @return QC2_BAD_VALUE invalid reference to value-reference passed
     * @return QC2_BAD_INDEX key does not exist
     * @return QC2_CANNOT_DO requested value type is not Storable
     * @return QC2_OK        value retrieved successfully
     */
    // getter specialized for shared_ptr<derivates-of-Storable> types
    template <typename T>
    QC2Status get(const std::string& key, std::shared_ptr<T> *val) const {
        if (val == nullptr) {
            return QC2_BAD_VALUE;
        }
        // allow only objects derived from Storable
        if (!std::is_base_of<Storable, T>::value) {
            return QC2_CANNOT_DO;  // Invalid object type
        }

        auto itr = mMap.find(key);
        if (itr != mMap.end()) {
            std::shared_ptr<Storable> temp;
            auto ret = itr->second.assign(&temp);
            if (ret == 0) {
                *val = std::static_pointer_cast<T>(temp);
            }
            return ret;
        } else {
            return QC2_BAD_INDEX;  // key not found
        }
    }

    bool hasKey(const std::string& key) const {
        return mMap.count(key) > 0;
    }

    /// copy all the key-values to the bundle passed
    void copyTo(Bundle* b) {
        if (b) {
            for (auto& pair : mMap) {
                b->mMap.emplace(pair.first, Item(pair.second));
            }
        }
    }

    void clear() {
        mMap.clear();
    }

 private:
    DECLARE_NON_COPYASSIGNABLE(Bundle);

    /**
     * @brief container for individual value in Bundle
     */
    struct Item {
        explicit Item(int32_t);
        explicit Item(uint32_t);
        explicit Item(int64_t);
        explicit Item(uint64_t);
        explicit Item(const std::string&);
        explicit Item(std::shared_ptr<Storable>);
        Item(const Item& rhs);
        ~Item();

        enum Type {
            INVALID = 0,
            INT32 = 1,
            UINT32 = 2,
            INT64 = 3,
            UINT64 = 4,
            STRING = 5,
            OBJ = 6,
        } mT;

        union U {
            int32_t i32;        // INT32
            uint32_t u32;       // UINT32
            int64_t i64;        // INT64
            uint64_t u64;       // UINT64
            std::string *mStr;  // STRING
            std::shared_ptr<Storable> *mObj;    // OBJ
        } mU;

        // generic assign for basic types
        template <typename T>
        QC2Status assign(T *val) const {
            if (std::is_same<T, int32_t>::value && (mT == INT32)) {
                *val = mU.i32;
                return QC2_OK;
            } else if (std::is_same<T, uint32_t>::value && (mT == UINT32)) {
                *val = mU.u32;
                return QC2_OK;
            } else if (std::is_same<T, int64_t>::value && (mT == INT64)) {
                *val = mU.i64;
                return QC2_OK;
            } else if (std::is_same<T, uint64_t>::value && (mT == UINT64)) {
                *val = mU.u64;
                return QC2_OK;
            }
            return QC2_BAD_VALUE;
        }

        // specialized assign for std::string
        QC2Status assign(std::string *val) const {
            if (mT != STRING || !mU.mStr) {
                return QC2_BAD_VALUE;
            }
            *val = *(mU.mStr);
            return QC2_OK;
        }

        // specialized assign for Object types
        QC2Status assign(std::shared_ptr<Storable> *val) const {
            if (mT != OBJ || !mU.mObj) {
                return QC2_BAD_VALUE;
            }
            *val = *(mU.mObj);
            return QC2_OK;
        }
    };

    std::unordered_map<std::string, Item> mMap;
};

/**
 * @brief Event models the information about a specific action or response
 *
 * An Event contains following:
 * -# id       : what is it about ? (always)
 * -# priority : how urgent is it ? (always)
 * -# info     : details of a request/action (optional)
 * -# reply    : details of the response (optional)\n
 * @note id and priority are not mutable in the lifetime of an event\n
 * Events can be posted to an EventHandler
 */
class Event {
 public:
    enum class Priority {
        NORMAL = 0,
        HIGH = 1,
    };

    /**
     * @brief create an event with default priority
     */
    explicit Event(uint32_t id) : mId(id), mPriority(Priority::NORMAL), mRetired(false) {
    }

    /**
     * @brief create an event with requested priority
     */
    explicit Event(uint32_t id, Priority prio) : mId(id), mPriority(prio), mRetired(false) {
    }

    /**
     * @brief get the event's id
     */
    uint32_t id() const {
        return mId;
    }

    /**
     * @brief get the event's priority
     */
    Priority priority() const {
        return mPriority;
    }

    /**
     * @brief get a reference to the event's info
     */
    Bundle& info() {
        return mInfo;
    }

    const Bundle& info() const {
        return mInfo;
    }

    /**
     * @brief get a reference to the event's reply
     */
    Bundle& reply() {
        return mReply;
    }

    const Bundle& reply() const {
        return mReply;
    }

    void retire() {
        mRetired = true;
    }

    bool retired() const {
        return mRetired;
    }

 private:
    DECLARE_NON_COPYASSIGNABLE(Event);

    const uint32_t mId;         ///< id of the event (immutable, once set)
    const Priority mPriority;   ///< priority of the event (immutable, once set)
    bool mRetired;              ///< event has been handled
    Bundle mInfo;               ///< info associated with the requesting event
    Bundle mReply;              ///< reply associated with the handled event
};

/// @}

};  // namespace qc2audio

#endif  // _QC2AUDIO_UTILS_EVENT_H_
