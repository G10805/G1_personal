/*
 **************************************************************************************************
 * Copyright (c) 2018,2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef CODEC2_CODECS_INCLUDE_QC2METADATATRANSLATOR_H_
#define CODEC2_CODECS_INCLUDE_QC2METADATATRANSLATOR_H_

#include <memory>
#include <vector>
#include <set>
#include "QC2Buffer.h"
#include "QC2.h"
#include "QC2Constants.h"

namespace qc2 {

/// @addtogroup codec_impl
/// @{

/**
 * @brief Metadata Translator provides Reader/Writer to extract/package metadata.
 *
 * Reader/Writer translate C2Params(Infos/Tunings)/InfoBufs from/to Codec's implementation-specfic
 *  metadata payload format
 *
 * This is the default implementation that works for V4L2 HW Codec.
 * Other Codecs may Override the translator and supply their version of Reader/Writer
 */
class QC2MetadataTranslator {
 public:
    explicit QC2MetadataTranslator(ComponentKind kind)
        : mKind (kind) {
    }

    virtual ~QC2MetadataTranslator() = default;

    /**
     * @brief Write infos/infoBuffers into codec-specific blob
     *
     * Write-out specified infos/infoBuffers into metadata memory provided
     * @param[in] dst   writeable destination memory (metadata buffer) to write metadata on to
     * @param[in] capacity size of metadata buffer (dst) in bytes
     * @param[in] infos vector of infos to be flattened
     * @param[in] filter set of metadata to be filtered
     * @param[in] infoBuffers vector of infoBuffers to be flattened
     * @param[out] skipped vector of infos/infoBuffers that were not flattened, either due to
     *      -# skipped ones are not supported by the codec or
     *      -# ran out of memory
     *
     * @return QC2_OK if all of the infos/infoBuffers were flattened successfully
     * @return QC2_NO_MEMORY if the metadata buffer could not accommodate all the infos/infoBuffers
     * @return QC2_ERROR if pointer arguments are invalid (nullptr)
     * @return QC2_CANNOT_DO if some of infos/InfoBuffers are not supported by the Codec
     */
    virtual QC2Status flatten(
            uint8_t *dst __unused,
            uint32_t capacity __unused,
            const std::vector<std::shared_ptr<C2Info>>& infos __unused,
            const std::set<C2Param::Index> *filter __unused,
            const std::vector<std::shared_ptr<C2InfoBuffer>>& infoBufs __unused,
            std::vector<C2Param::Type> *skipped __unused) {
        return QC2_CANNOT_DO;
    }

    /**
     * @brief Parse codec-specific metadata blob into infos/infoBuffers
     *
     * Extract infos/infoBufs from metadata buffer packed in codec-specific blob
     * @param[in] src   read-only source memory (metadata buffer) to read metadata from
     * @param[in] capacity size of metadata buffer (src) in bytes
     * @param[in] filter (optional) set of infos(infoBufs) requested. If provided, return only the
     *  specified infos. If not provided (nullptr), return all the infos available in blob
     * @param[out] infos extracted infos
     * @param[out] infoBufs extracted infoBuffers
     *
     * @return QC2_OK if all infos/infoBufs were extracted successfully
     * @return QC2_CORRUPTED if extraction failed
     * @return QC2_ERROR if pointer arguments are invalid (nullptr)
     */
    virtual QC2Status extract(
            const uint8_t *src __unused,
            uint32_t capacity __unused,
            const std::set<C2Param::Index> *filter __unused,
            std::vector<std::shared_ptr<C2Info>> *infos __unused,
            std::vector<std::shared_ptr<C2InfoBuffer>> *infoBufs __unused) {
        return QC2_CORRUPTED;
    }

 protected:
    inline bool isInfoRequired(const std::set<C2Param::Index> *filter,
            C2Param::Index index) {
        return !filter || filter->count(index) > 0;
    }
    std::shared_ptr<C2Info> fetchInfo(const std::vector<std::shared_ptr<C2Info>>& infos,
            C2Param::Index index) {
        for (size_t i = 0; i < infos.size(); i++) {
            if (infos[i] && infos[i]->coreIndex().coreIndex()
                    && infos.at(i)->coreIndex().coreIndex() == index) {
                return infos[i];
            }
        }
        return nullptr;
    }
    const ComponentKind mKind;
};

/// @}

};  // namespace qc2

#endif  // CODEC2_CODECS_INCLUDE_QC2METADATATRANSLATOR_H_

