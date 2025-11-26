/*
 **************************************************************************************************
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#undef LOG_TAG
#define LOG_TAG "ImageConverterTest"

#include <unistd.h>  // for usleep
#include <string.h>  // for memset
#include <memory>
#include <gtest/gtest.h>

#include <C2PlatformSupport.h>

#include "QC2.h"
#include "QC2ImageConverter.h"
#include "QC2Constants.h"
#include "QC2Buffer.h"
#include "QC2Component.h"
#include "QC2ComponentStore.h"
#include "QC2TestInputStream.h"
#include "QC2FileDump.h"
#include "QC2Platform.h"

using namespace android;
using namespace qc2;

class QC2ImageConverterTest : public ::testing::Test {
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;

    std::shared_ptr<QC2Component> mComp;
    std::shared_ptr<C2BlockPool> mSrcPool;
    std::shared_ptr<C2BlockPool> mDstPoolIntermediate;
    std::shared_ptr<C2BlockPool> mDstPoolFinal;

    void doConvertPatternAndDump(
                std::unique_ptr<QC2ImageConverter> &converter,
                std::unique_ptr<QC2GraphicInputSource> &patternSource,
                std::unique_ptr<QC2GraphicBufferPool> &dstPool,
                std::unique_ptr<QC2FileDump> &inputDump,
                std::unique_ptr<QC2FileDump> &outputDump,
                uint32_t nFrames);

    void do2PassConvertPatternAndDump(
                std::unique_ptr<QC2ImageConverter> &converter1Pass,
                std::unique_ptr<QC2ImageConverter> &converter2Pass,
                std::unique_ptr<QC2GraphicInputSource> &patternSource,
                std::unique_ptr<QC2GraphicBufferPool> &dstIntermediatePool,
                std::unique_ptr<QC2GraphicBufferPool> &dstFinalPool,
                std::unique_ptr<QC2FileDump> &inputDump,
                std::unique_ptr<QC2FileDump> &IntermediateDump,
                std::unique_ptr<QC2FileDump> &outputDump,
                uint32_t nFrames);
};

static void QC2ImageConverterTest_deleter(C2Component *comp) {
    QLOGI("Deleting component");
    delete comp;
}

void QC2ImageConverterTest::SetUp() {
    auto status = QC2Component::Create(
        "c2.qti.avc.decoder", 0, nullptr, QC2ImageConverterTest_deleter, &mComp);
    ASSERT_TRUE(status == QC2_OK);
    ASSERT_TRUE(mComp != nullptr);

    c2_status_t srcRet = GetCodec2BlockPool(C2BlockPool::BASIC_GRAPHIC, mComp, &mSrcPool);
    c2_status_t dstIntermediateRet = GetCodec2BlockPool(C2BlockPool::BASIC_GRAPHIC, mComp, &mDstPoolIntermediate);
    c2_status_t dstFinalRet = GetCodec2BlockPool(C2BlockPool::BASIC_GRAPHIC, mComp, &mDstPoolFinal);

    ASSERT_TRUE(mComp != nullptr);
    ASSERT_TRUE(srcRet == C2_OK);
    ASSERT_TRUE(dstIntermediateRet == C2_OK);
    ASSERT_TRUE(dstFinalRet == C2_OK);
    ASSERT_TRUE(mSrcPool != nullptr);
    ASSERT_TRUE(mDstPoolIntermediate != nullptr);
    ASSERT_TRUE(mDstPoolFinal != nullptr);
}

void QC2ImageConverterTest::TearDown() {
    mComp = nullptr;
    mSrcPool = nullptr;
    mDstPoolIntermediate = nullptr;
    mDstPoolFinal = nullptr;
}

void QC2ImageConverterTest::doConvertPatternAndDump(
            std::unique_ptr<QC2ImageConverter> &converter,
            std::unique_ptr<QC2GraphicInputSource> &patternSource,
            std::unique_ptr<QC2GraphicBufferPool> &dstPool,
            std::unique_ptr<QC2FileDump> &inputDump,
            std::unique_ptr<QC2FileDump> &outputDump,
            uint32_t nFrames = 1) {

    for (uint32_t i = 0; i < nFrames; ++i) {
        std::shared_ptr<QC2Buffer> frame;
        QC2Status ret = patternSource->nextFrame(&frame);
        ASSERT_TRUE(ret == QC2_OK);
        ASSERT_TRUE(frame != NULL);
        ASSERT_EQ(inputDump->write(frame), QC2_OK);

        std::shared_ptr<QC2Buffer> dstBuf;
        QC2Status dstRet = dstPool->allocate(&dstBuf);

        ASSERT_TRUE(dstRet == QC2_OK);
        ASSERT_TRUE(dstBuf != nullptr);
        ASSERT_TRUE(dstBuf->isGraphic());

        auto &srcGraphic = frame->graphic();
        auto &dstGraphic = dstBuf->graphic();

        ret = converter->process(&srcGraphic, &dstGraphic);
        ASSERT_TRUE(ret == QC2_OK);

        ASSERT_EQ(outputDump->write(dstBuf), QC2_OK);
    }
}

void QC2ImageConverterTest::do2PassConvertPatternAndDump(
            std::unique_ptr<QC2ImageConverter> &converter1Pass,
            std::unique_ptr<QC2ImageConverter> &converter2Pass,
            std::unique_ptr<QC2GraphicInputSource> &patternSource,
            std::unique_ptr<QC2GraphicBufferPool> &dstIntermediatePool,
            std::unique_ptr<QC2GraphicBufferPool> &dstFinalPool,
            std::unique_ptr<QC2FileDump> &inputDump,
            std::unique_ptr<QC2FileDump> &IntermediateDump,
            std::unique_ptr<QC2FileDump> &outputDump,
            uint32_t nFrames = 1) {

    for (uint32_t i = 0; i < nFrames; ++i) {
        std::shared_ptr<QC2Buffer> frame;
        QC2Status ret = patternSource->nextFrame(&frame);
        ASSERT_TRUE(ret == QC2_OK);
        ASSERT_TRUE(frame != NULL);
        ASSERT_EQ(inputDump->write(frame), QC2_OK);

        std::shared_ptr<QC2Buffer> dstIntermediateBuf;
        QC2Status dstRet = dstIntermediatePool->allocate(&dstIntermediateBuf);

        ASSERT_TRUE(dstRet == QC2_OK);
        ASSERT_TRUE(dstIntermediateBuf != nullptr);
        ASSERT_TRUE(dstIntermediateBuf->isGraphic());

        auto &srcGraphic = frame->graphic();
        auto &dstIntermediateGraphic = dstIntermediateBuf->graphic();

        ret = converter1Pass->process(&srcGraphic, &dstIntermediateGraphic);
        ASSERT_TRUE(ret == QC2_OK);

        ASSERT_EQ(IntermediateDump->write(dstIntermediateBuf), QC2_OK);

        std::shared_ptr<QC2Buffer> dstFinalBuf;
        dstRet = dstFinalPool->allocate(&dstFinalBuf);

        ASSERT_TRUE(dstRet == QC2_OK);
        ASSERT_TRUE(dstFinalBuf != nullptr);
        ASSERT_TRUE(dstFinalBuf->isGraphic());

        auto &dstFinalGraphic = dstFinalBuf->graphic();

        ret = converter2Pass->process(&dstIntermediateGraphic, &dstFinalGraphic);
        ASSERT_TRUE(ret == QC2_OK);

        ASSERT_EQ(outputDump->write(dstFinalBuf), QC2_OK);

    }
}

//---------------------------------------------------------------------------------------------
// Tests
//---------------------------------------------------------------------------------------------
// rotate 90 then flip in horizontal
// + + x        + +
// + + +   -->  + +
//              x +
TEST_F(QC2ImageConverterTest, TestQC2GeneratePatternConvertDumpRotate90FlipH) {

    std::shared_ptr<QC2GraphicBufferPool> srcPool =
            std::make_shared<QC2GraphicBufferPoolImpl>(mSrcPool, 0);
    std::unique_ptr<QC2GraphicBufferPool> dstPool =
            std::make_unique<QC2GraphicBufferPoolImpl>(mDstPoolFinal, 0);

    std::unique_ptr<QC2FileDump> inputDump;
    ASSERT_EQ(QC2FileDump::Create("video/raw", "/sdcard/test-input-R90FH.yuv", &inputDump), QC2_OK);
    ASSERT_TRUE(inputDump != nullptr);

    std::unique_ptr<QC2FileDump> outputDump;
    ASSERT_EQ(QC2FileDump::Create("video/raw", "/sdcard/test-output-R90FH.yuv", &outputDump), QC2_OK);
    ASSERT_TRUE(outputDump != nullptr);

    dstPool->setFormat(PixFormat::VENUS_NV12);

    // Produce, convert and dump 720p
    {
        auto converter = QC2ImageConverter::Builder({PixFormat::RGBA8888, 1280, 720})
                .withDestinationColor(PixFormat::VENUS_NV12)
                .withRotation(QC2ImageConverter::Capabilities::RotationAngle::ROTATION_90)
                .withFlip(QC2ImageConverter::Capabilities::FlipDirection::FLIP_HORIZONTAL)
                .build();
        std::unique_ptr<QC2GraphicInputSource> source;
        QC2GraphicInputSource::Create(1280, 720, PixFormat::RGBA8888, "",
                "", srcPool, &source);
        ASSERT_TRUE(source != nullptr);

        dstPool->setResolution(720, 1280);
        doConvertPatternAndDump(converter, source, dstPool, inputDump, outputDump, 4);
    }
}

/* 2 Pass Conversion - VENUS_NV12 -> VENUS_NV12_UBWC -> VENUS_NV12 */
TEST_F(QC2ImageConverterTest, TestQC2GeneratePatternConvert2PassDumpUBWC2VenusNV12) {

    std::shared_ptr<QC2GraphicBufferPool> srcPool =
            std::make_shared<QC2GraphicBufferPoolImpl>(mSrcPool, 0);
    std::unique_ptr<QC2GraphicBufferPool> dstIntermediatePool =
            std::make_unique<QC2GraphicBufferPoolImpl>(mDstPoolIntermediate, 0);
    std::unique_ptr<QC2GraphicBufferPool> dstFinalPool =
            std::make_unique<QC2GraphicBufferPoolImpl>(mDstPoolFinal, 0);

    std::unique_ptr<QC2FileDump> inputDump;
    ASSERT_EQ(QC2FileDump::Create("video/raw", "/sdcard/test-inputUBWC2VenusNV12.yuv", &inputDump), QC2_OK);
    ASSERT_TRUE(inputDump != nullptr);

    std::unique_ptr<QC2FileDump> intermediateDump;
    ASSERT_EQ(QC2FileDump::Create("video/raw", "/sdcard/test-intermediateUBWC2VenusNV12.yuv", &intermediateDump), QC2_OK);
    ASSERT_TRUE(intermediateDump != nullptr);

    std::unique_ptr<QC2FileDump> outputDump;
    ASSERT_EQ(QC2FileDump::Create("video/raw", "/sdcard/test-outputUBWC2VenusNV12.yuv", &outputDump), QC2_OK);
    ASSERT_TRUE(outputDump != nullptr);

    dstIntermediatePool->setFormat(PixFormat::VENUS_NV12_UBWC);
    dstFinalPool->setFormat(PixFormat::VENUS_NV12);

    // Produce, convert and dump 720p
    {
        std::unique_ptr<QC2GraphicInputSource> source;
        QC2GraphicInputSource::Create(1280, 720, PixFormat::VENUS_NV12, "",
                "", srcPool, &source);
        ASSERT_TRUE(source != nullptr);

        dstIntermediatePool->setResolution(1280, 720);
        dstFinalPool->setResolution(1280, 720);

        auto converter1Pass = QC2ImageConverter::Builder({PixFormat::VENUS_NV12, 1280, 720})
                .withDestinationColor(PixFormat::VENUS_NV12_UBWC)
                .build();

        auto converter2pass = QC2ImageConverter::Builder({PixFormat::VENUS_NV12_UBWC, 1280, 720})
                .withDestinationColor(PixFormat::VENUS_NV12)
                .build();

        do2PassConvertPatternAndDump(converter1Pass, converter2pass, source, dstIntermediatePool,
                dstFinalPool, inputDump, intermediateDump, outputDump, 4);
    }
}

/* 2 Pass Conversion - RGBA8888 -> VENUS_NV12_UBWC -> RGBA8888 */
TEST_F(QC2ImageConverterTest, TestQC2GeneratePatternConvert2PassDumpUBWC2RGBA8888) {

    std::shared_ptr<QC2GraphicBufferPool> srcPool =
            std::make_shared<QC2GraphicBufferPoolImpl>(mSrcPool, 0);
    std::unique_ptr<QC2GraphicBufferPool> dstIntermediatePool =
            std::make_unique<QC2GraphicBufferPoolImpl>(mDstPoolIntermediate, 0);
    std::unique_ptr<QC2GraphicBufferPool> dstFinalPool =
            std::make_unique<QC2GraphicBufferPoolImpl>(mDstPoolFinal, 0);

    std::unique_ptr<QC2FileDump> inputDump;
    ASSERT_EQ(QC2FileDump::Create("video/raw", "/sdcard/test-inputRGBA2UBWC.yuv", &inputDump), QC2_OK);
    ASSERT_TRUE(inputDump != nullptr);

    std::unique_ptr<QC2FileDump> intermediateDump;
    ASSERT_EQ(QC2FileDump::Create("video/raw", "/sdcard/test-intermediateRGBA2UBWC.yuv", &intermediateDump), QC2_OK);
    ASSERT_TRUE(intermediateDump != nullptr);

    std::unique_ptr<QC2FileDump> outputDump;
    ASSERT_EQ(QC2FileDump::Create("video/raw", "/sdcard/test-outputRGBA2UBWC.yuv", &outputDump), QC2_OK);
    ASSERT_TRUE(outputDump != nullptr);

    dstIntermediatePool->setFormat(PixFormat::VENUS_NV12_UBWC);
    dstFinalPool->setFormat(PixFormat::RGBA8888);

    // Produce, convert and dump 720p
    {
        std::unique_ptr<QC2GraphicInputSource> source;
        QC2GraphicInputSource::Create(1280, 720, PixFormat::RGBA8888, "",
                "", srcPool, &source);
        ASSERT_TRUE(source != nullptr);

        dstIntermediatePool->setResolution(1280, 720);
        dstFinalPool->setResolution(1280, 720);

        auto converter1Pass = QC2ImageConverter::Builder({PixFormat::RGBA8888, 1280, 720})
                .withDestinationColor(PixFormat::VENUS_NV12_UBWC)
                .build();

        auto converter2pass = QC2ImageConverter::Builder({PixFormat::VENUS_NV12_UBWC, 1280, 720})
                .withDestinationColor(PixFormat::RGBA8888)
                .build();

        do2PassConvertPatternAndDump(converter1Pass, converter2pass, source, dstIntermediatePool,
                dstFinalPool, inputDump, intermediateDump, outputDump, 4);
    }
}