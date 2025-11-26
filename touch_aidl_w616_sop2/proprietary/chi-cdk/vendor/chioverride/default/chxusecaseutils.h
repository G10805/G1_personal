////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chxusecaseselector.h
/// @brief CHX usecase selector utility class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHXUSECASEUTILS_H
#define CHXUSECASEUTILS_H

#include <assert.h>
#include "chxincs.h"
#include "chxextensionmodule.h"

#include "chxmetadata.h"

// generated headers
#include "g_pipelines.h"

#if defined(__ANDROID__)
#if defined(ANDROID_Q_AOSP)
#include <gralloc_priv.h>
#include <hardware/gralloc1.h>
#else
#include <QtiGralloc.h>
#endif //ANDROID_Q_AOSP
#elif defined(LE_CAMERA) || defined(__AGL__)
#include "linuxembeddedgralloc.h"
#elif defined(__QNXNTO__) || defined(CAMERA_UNITTEST)
#include <hardware/camera3.h>
#if defined(CAMX_USE_GRALLOC1)
#include "gralloc.h"
#include "gralloc1.h"
#endif //(CAMX_USE_GRALLOC1)
#endif

// NOWHINE ENTIRE FILE - Temporarily bypassing for existing CHI files

#define WAIT_BUFFER_TIMEOUT 700 // should be less than the timeout used for each request (800ms)

/// Forward Declaration
struct ChiUsecase;
class  Usecase;

/// @brief Usecase identifying enums
enum class UsecaseId
{
    NoMatch             = 0,
    Default             = 1,
    Preview             = 2,
    PreviewZSL          = 3,
    MFNR                = 4,
    MFSR                = 5,
    MultiCamera         = 6,
    QuadCFA             = 7,
    RawJPEG             = 8,
    MultiCameraVR       = 9,
    Torch               = 10,
    YUVInBlobOut        = 11,
    VideoLiveShot       = 12,
    SuperSlowMotionFRC  = 13,
    MaxUsecases         = 14,
};

#if defined(CAMX_USE_GRALLOC1)
/// @brief Gralloc1 interface functions
struct Gralloc1Interface
{
    INT32 (*CreateDescriptor)(
        gralloc1_device_t*             pGralloc1Device,
        gralloc1_buffer_descriptor_t*  pCreatedDescriptor);
    INT32 (*DestroyDescriptor)(
        gralloc1_device_t*            pGralloc1Device,
        gralloc1_buffer_descriptor_t  descriptor);
    INT32 (*SetDimensions)(
        gralloc1_device_t*           pGralloc1Device,
        gralloc1_buffer_descriptor_t descriptor,
        UINT32                       width,
        UINT32                       height);
    INT32 (*SetFormat)(
        gralloc1_device_t*           pGralloc1Device,
        gralloc1_buffer_descriptor_t descriptor,
        INT32                        format);
    INT32 (*SetProducerUsage)(
        gralloc1_device_t*           pGralloc1Device,
        gralloc1_buffer_descriptor_t descriptor,
        UINT64                       usage);
    INT32 (*SetConsumerUsage)(
        gralloc1_device_t*           pGralloc1Device,
        gralloc1_buffer_descriptor_t descriptor,
        UINT64                       usage);
    INT32 (*Allocate)(
        gralloc1_device_t*                  pGralloc1Device,
        UINT32                              numDescriptors,
        const gralloc1_buffer_descriptor_t* pDescriptors,
        buffer_handle_t*                    pAllocatedBuffers);
    INT32 (*GetStride)(
        gralloc1_device_t* pGralloc1Device,
        buffer_handle_t    buffer,
        UINT32*            pStride);
    INT32 (*Release)(
        gralloc1_device_t* pGralloc1Device,
        buffer_handle_t    buffer);
    INT32 (*Lock)(
            gralloc1_device_t*      device,
            buffer_handle_t         buffer,
            uint64_t                producerUsage,
            uint64_t                consumerUsage,
            const gralloc1_rect_t*  accessRegion,
            void**                  outData,
            int32_t                 acquireFence);
    gralloc1_error_t (*Perform)(
            gralloc1_device_t*   device,
            int                  operation, ...);
};
#endif //(CAMX_USE_GRALLOC1)

/// @breif Generic lightweight doubly lined list node
struct LightweightDoublyLinkedListNode
{
    VOID*                                   pData; ///< Pointer to the data that needs to be stored.
    struct LightweightDoublyLinkedListNode* pPrev; ///< Pointer to the previous element in the list.
    struct LightweightDoublyLinkedListNode* pNext; ///< Pointer to the next element in the list.
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class that helps select a usecase from the usecases the override module supports
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class UsecaseSelector
{
public:

    /// Utility function to Calculate the FovRectIFE
    /// @TODO: Consider binning mode and sensor crop info in calculation
    static CHX_INLINE VOID CalculateFovRectIFE(
        CHIRECT*      fovRectIFE,
        const CHIRECT frameDimension,
        const CHIRECT activeArray)
    {
        UINT32 xStart = frameDimension.left;
        UINT32 yStart = frameDimension.top;

        fovRectIFE->left   = 0;
        fovRectIFE->top    = 0;
        fovRectIFE->width  = activeArray.width  - (2 * xStart);
        fovRectIFE->height = activeArray.height - (2 * yStart);
    }

    /// Creates an instance of this class
    static UsecaseSelector* Create(
        const ExtensionModule* pExtModule);

    /// Destroys an instance of the class
    VOID Destroy();

    /// Get Camera ID of Max resolution camera in Dualcamera usecase
    static UINT32 FindMaxResolutionCameraID(
        LogicalCameraInfo* pCameraInfo);

    /// Get Sensor Dimension
    static VOID getSensorDimension(
        const UINT32 cameraID, const camera3_stream_configuration_t* pStreamConfig,
        UINT32 *sensorw, UINT32 *sensorh, UINT32 downscaleFactor);

    /// Get Sensor Mode Info
    static CHISENSORMODEINFO* GetSensorModeInfo(
        const UINT32 cameraID, const camera3_stream_configuration_t* pStreamConfig,
        UINT32 downscaleFactor);

    /// Returns a matching usecase
    UsecaseId GetMatchingUsecase(
        const LogicalCameraInfo *pCamInfo,
        camera3_stream_configuration_t* pStreamConfig); ///< Stream configuration

    /// Returns a default matching usecase, Null otherwise
    static ChiUsecase* DefaultMatchingUsecase(
        camera3_stream_configuration_t* pStreamConfig); ///< Stream configuration

    /// Is the stream config a match for MFNR
    static BOOL MFNRMatchingUsecase(
        const camera3_stream_configuration_t* pStreamConfig);

    /// Is the stream config a match for MFSR
    static BOOL MFSRMatchingUsecase(
        const camera3_stream_configuration_t* pStreamConfig);

    /// Is the stream config a match for video liveshot
    static BOOL IsVideoLiveShotConfig(
        const camera3_stream_configuration_t* pStreamConfig);

    /// Is Video EISV2 enabled
    static BOOL IsVideoEISV2Enabled(
        camera3_stream_configuration_t* pStreamConfig);

    /// Is Video EISV3 enabled
    static BOOL IsVideoEISV3Enabled(
        camera3_stream_configuration_t* pStreamConfig);

    /// Is the stream config a match for Quad CFA
    static BOOL QuadCFAMatchingUsecase(
        const LogicalCameraInfo*              pCamInfo,
        const camera3_stream_configuration_t* pStreamConfig);

    /// Is this a Quad CFA sensor
    static BOOL IsQuadCFASensor(
        const LogicalCameraInfo* pCamInfo);

    /// Is this a preview stream
    static BOOL IsPreviewStream(
        const camera3_stream_t* pStream);

    /// Is this a video stream
    static BOOL IsVideoStream(
        const camera3_stream_t* pStream);

    /// Is this a HEIF stream
    static BOOL IsHEIFStream(
        const camera3_stream_t* pStream);

    /// Is this a snapshot YUV stream
    static BOOL IsYUVSnapshotStream(
        const camera3_stream_t* pStream);

    /// Is this a snapshot JPEG stream
    static BOOL IsJPEGSnapshotStream(
        const camera3_stream_t* pStream);

    /// Is this a Raw stream
    static BOOL IsRawStream(
        const camera3_stream_t* pStream);

#if defined(CAMX_ANDROID_API) && (CAMX_ANDROID_API >= 28) //Android-P or better
    static BOOL IsMatchingFormat(
        ChiStream*             pStream,
        UINT32                 numFormats,
        const ChiBufferFormat* pFormats);
#endif

    /// Is this a YUV input stream
    static BOOL IsYUVInStream(
        const camera3_stream_t* pStream);

    /// Is this a YUV output stream
    static BOOL IsYUVOutStream(
        const camera3_stream_t* pStream);

    /// Util functions to deep copy usecase structure
    static ChiUsecase* CloneUsecase(
        const ChiUsecase *pSrcUsecase, UINT32 numDesc, UINT32 *pDescIndex);

    /// Util function to destroy cloned usecase structure
    static VOID DestroyUsecase(ChiUsecase *pUsecase);

private:

    /// Initializes the instance
    CDKResult Initialize();

    /// Is the stream config a match for Preview+ZSL-YUV
    static BOOL IsPreviewZSLStreamConfig(
        const camera3_stream_configuration_t* pStreamConfig);

    /// Is the stream config a match for Raw+JPEG usecase
    static BOOL IsRawJPEGStreamConfig(
        const camera3_stream_configuration_t* pStreamConfig);

    /// Is the stream config a match YUV In and Blob out
    static BOOL IsYUVInBlobOutConfig(
        const camera3_stream_configuration_t* pStreamConfig);

    static BOOL IsMatchingUsecase(
        const camera3_stream_configuration_t* pStreamConfig,    ///< Stream config
        const ChiUsecase*                     pUsecase);        ///< Usecase attempted to match

#if defined(CAMX_ANDROID_API) && (CAMX_ANDROID_API < 28) //Android-O
    static BOOL IsMatchingFormat(
        ChiStream*             pStream,
        UINT32                 numFormats,
        const ChiBufferFormat* pFormats);
#endif

    static BOOL IsAllowedImplDefinedFormat(
        ChiBufferFormat format,
        GrallocUsage    streamGrallocUsage);

    /// Clone pipeline descriptor
    static CDKResult ClonePipelineDesc(
        const ChiPipelineTargetCreateDescriptor* pSrcDesc,
        ChiPipelineTargetCreateDescriptor*       pDstDesc);

    /// Clone chi target
    static ChiTarget* ClonePipelineDesc(
        const ChiTarget* pSrcDesc);

    /// Destroy cloned pipeline descriptor
    static VOID DestroyPipelineDesc(
        ChiPipelineTargetCreateDescriptor* pDesc);

    /// Destroy cloned chi target
    static VOID DestroyPipelineDesc(
        ChiTarget* pDesc);

    UsecaseSelector() = default;
    ~UsecaseSelector();
    // Do not support the copy constructor or assignment operator
    UsecaseSelector(const UsecaseSelector& rUsecaseSelector) = delete;
    UsecaseSelector& operator= (const UsecaseSelector& rUsecaseSelector) = delete;

    static UINT            NumImplDefinedFormats;
    static ChiBufferFormat AllowedImplDefinedFormats[16];
    static BOOL            GPURotationUsecase;             ///< Flag to select gpu rotation usecases
    static BOOL            GPUDownscaleUsecase;            ///< Flag to select gpu rotation usecases
    static BOOL            HFRNo3AUsecase;                 ///< Flag to select HFR without 3A usecases
    static UINT            VideoEISV2Usecase;              ///< Flag to select EIS V2 usecases
    static UINT            VideoEISV3Usecase;              ///< Flag to select EIS V3 usecases

    const ExtensionModule*      m_pExtModule;              ///< const pointer to the extension module
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class that implements the factory to create usecase specific objects
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class UsecaseFactory
{
public:
    /// Creates an object of this factory type
    static UsecaseFactory* Create(
        const ExtensionModule* pExtModule);
    /// Destroy the factory
    VOID Destroy();
    /// Create an usecase object of the specified type
    Usecase* CreateUsecaseObject(
        LogicalCameraInfo*              pLogicalCameraInfo, ///< [In] Camera associated with the usecase
        UsecaseId                       usecaseId,          ///< [In] Usecase Id
        camera3_stream_configuration_t* pStreamConfig);     ///< [In] Stream config

private:
    UsecaseFactory() = default;
    ~UsecaseFactory();
    UsecaseFactory(const UsecaseFactory&) = delete;             ///< Disallow the copy constructor
    UsecaseFactory& operator=(const UsecaseFactory&) = delete;  ///< Disallow assignment operator

    const ExtensionModule* m_pExtModule;              ///< const pointer to the extension module
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Generic POD lightweight doubly linked list implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LightweightDoublyLinkedList final
{
public:
    // Insert to Tail of the list
    CHX_INLINE VOID InsertToTail(
        LightweightDoublyLinkedListNode* pNodeToInsert)
    {
        CHX_ASSERT(pNodeToInsert->pPrev == NULL);
        CHX_ASSERT(pNodeToInsert->pNext == NULL);

        if (m_pHead == NULL)
        {
            m_pHead = pNodeToInsert;

            // First node going into the doubly linked list, so our head/tail are the same.
            m_pTail = m_pHead;
        }
        else
        {
            // Since we can always know where the tail it, inserts become trivial.
            m_pTail->pNext = pNodeToInsert;
            pNodeToInsert->pPrev = m_pTail;

            m_pTail = pNodeToInsert;
        }

        m_numNodes++;
    }

    // Insert to Head of the list
    CHX_INLINE VOID InsertToHead(
        LightweightDoublyLinkedListNode* pNodeToInsert)
    {
        CHX_ASSERT(pNodeToInsert->pPrev == NULL);
        CHX_ASSERT(pNodeToInsert->pNext == NULL);

        if (m_pHead == NULL)
        {
            m_pHead = pNodeToInsert;

            // First node going into the doubly linked list, so our head/tail are the same.
            m_pTail = m_pHead;
        }
        else
        {
            // Since we can always know where the head is, inserts become trivial.
            pNodeToInsert->pNext = m_pHead;
            m_pHead->pPrev = pNodeToInsert;
            m_pHead = pNodeToInsert;
        }

        m_numNodes++;
    }

    // Remove and return head node
    CHX_INLINE LightweightDoublyLinkedListNode* RemoveFromHead()
    {
        LightweightDoublyLinkedListNode* pNode = m_pHead;

        if (pNode != NULL)
        {
            // If the only node was removed, the tail must be updated to reflect the empty list.
            if (m_numNodes == 1)
            {
                CHX_ASSERT(pNode == m_pTail);

                m_pTail = NULL;
                m_pHead = NULL;
            }
            else
            {
                m_pHead = pNode->pNext;
                if (NULL != m_pHead)
                {
                    m_pHead->pPrev = NULL;
                }
            }

            pNode->pPrev = NULL;
            pNode->pNext = NULL;

            m_numNodes--;
        }

        return pNode;
    }

    // Remove Node from list
    CHX_INLINE VOID RemoveNode(
        LightweightDoublyLinkedListNode* pNode)
    {
        if (NULL != pNode)
        {
            if (pNode == m_pHead)
            {
                m_pHead = pNode->pNext;

                if (m_pHead != NULL)
                {
                    m_pHead->pPrev = NULL;
                }
            }
            else
            {
                if (NULL != pNode->pPrev)
                {
                    pNode->pPrev->pNext = pNode->pNext;
                }

                if (pNode->pNext != NULL)
                {
                    pNode->pNext->pPrev = pNode->pPrev;
                }
            }

            if (pNode == m_pTail)
            {
                m_pTail = pNode->pPrev;
            }

            pNode->pPrev = NULL;
            pNode->pNext = NULL;

            m_numNodes--;
        }
    }

    // Helper to fetch next node from current node
    CHX_INLINE static LightweightDoublyLinkedListNode* NextNode(
        LightweightDoublyLinkedListNode* pNode)
    {
        return (NULL != pNode) ? pNode->pNext : NULL;
    }

    // Helper to fetch head node
    CHX_INLINE LightweightDoublyLinkedListNode* Head() const { return m_pHead; }

    // Helper to fetch tail node
    CHX_INLINE LightweightDoublyLinkedListNode* Tail() const { return m_pTail; }

    // Helper to fetch node count
    CHX_INLINE UINT NumNodes() const { return m_numNodes; }

    // constructor
    // NOWHINE CP016: Basic final class wont have overrides
    CHX_INLINE LightweightDoublyLinkedList()
    {
        m_pHead     = NULL;
        m_pTail     = NULL;
        m_numNodes  = 0;
    }

    // destructor
    // NOWHINE CP022: Basic final class wont have overrides
    CHX_INLINE ~LightweightDoublyLinkedList()
    {
        CHX_ASSERT(NumNodes() == 0);
    }

private:
    // Member methods
    LightweightDoublyLinkedList(const LightweightDoublyLinkedList&) = delete;
    LightweightDoublyLinkedList& operator=(const LightweightDoublyLinkedList&) = delete;

    LightweightDoublyLinkedListNode* m_pHead;    ///< The first element in the list
    LightweightDoublyLinkedListNode* m_pTail;    ///< The last element in the list
    UINT                             m_numNodes; ///< The number of elements in the list
};

#ifndef CAMX_USE_GRALLOC1
/// @ brief Enums duplicated from Gralloc Legacy
/// To convert enum from legacy to Gralloc#4, use the new enum value
enum GrallocEnum
{
    COLOR_METADATA          = QTI_COLOR_METADATA,       ///< Set Color Metadata
    SET_CVP_METADATA        = QTI_CVP_METADATA,         ///< Set CVP Metadata
    SET_UBWC_CR_STATS_INFO  = QTI_UBWC_CR_STATS_INFO,   ///< UBWC Stats
    SET_VIDEO_PERF_MODE     = QTI_VIDEO_PERF_MODE,      ///< Set Video Perf mode
    SET_VT_TIMESTAMP        = QTI_VT_TIMESTAMP,         ///< Set VT Timestamp
    UPDATE_BUFFER_GEOMETRY  = QTI_SINGLE_BUFFER_MODE,   ///< Buffer Dimensions
};

/// @ brief struct duplicated from Gralloc Legacy
struct BufferDim_t
{
    UINT32 sliceWidth;  ///< Width
    UINT32 sliceHeight; ///< Height
};

/// @brief General Gralloc parameters needed to allocate buffer
struct BufferAllocParams
{
    UINT32  width;          ///< Width
    UINT32  height;         ///< Height
    UINT32  format;         ///< Gralloc Format
    UINT64  producerFlags;  ///< Produce Gralloc Usage flags
    UINT64  consumerFlags;  ///< Consumer Gralloc Usage flags
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief General Gralloc allocator class
///
/// Basic wrapping Gralloc alloc, free
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Gralloc final
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetInstance
    ///
    /// @brief  This function returns the singleton instance of the Gralloc
    ///
    /// @return A pointer to the singleton instance of the Gralloc
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static Gralloc* GetInstance();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// SetGrallocMetaData
    ///
    /// @brief  API to set Metadata into Gralloc handle.
    ///
    /// @param  hNativehandle       Pointer to Native Handle
    /// @param  grallocEnumType     Metadata type
    /// @param  metadataSize        Metadata size
    /// @param  pMetadata           pointer to metadata structure
    ///
    /// @return CDKResult Enum
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static CDKResult SetGrallocMetaData(
        buffer_handle_t     hNativehandle,
        UINT32              metadatatype,
        UINT32              metadataSize,
        VOID*               pMetadata);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetGrallocInterface
    ///
    /// @brief  This function returns the gralloc Interface pointer
    ///
    /// @return GrallocInterface structure pointer
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CHX_INLINE VOID* GetGrallocInterface()
    {
        return m_pGrallocIntf;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// AllocateGrallocBufferWithMapper4
    ///
    /// @brief  API to allocate Gralloc handle with Mapper#4.
    ///
    /// @param  width           Buffer width
    /// @param  height          Buffer height
    /// @param  format          Buffer format
    /// @param  producerFlags   Buffer producer flags
    /// @param  consumerFlags   Buffer consumer flags
    /// @param  hGrallocBuffer  Gralloc buffer handle
    /// @param  pStride         pointer to stride parameter
    ///
    /// @return CDKResult Enum
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CDKResult AllocateGrallocBufferWithMapper4(
        UINT32              width,
        UINT32              height,
        UINT32              format,
        UINT64              producerFlags,
        UINT64              consumerFlags,
        buffer_handle_t&    hGrallocBuffer,
        UINT32*             pStride);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FreeGrallocBuffer
    ///
    /// @brief  This function frees the gralloc buffer
    ///
    /// @param  hGrallocBuffer    Gralloc buffer handle
    ///
    /// @return CDKResult Enum
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CDKResult FreeGrallocBuffer(
        buffer_handle_t hGrallocBuffer);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetBufferFD
    ///
    /// @brief  API to get Gralloc Buffer File Descriptor
    ///
    /// @param  hNativehandle       Pointer to Native Handle
    ///
    /// @return INT32 FD value
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    INT32 GetBufferFD(
        buffer_handle_t hNativehandle);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetBufferOffset
    ///
    /// @brief  API to get Gralloc Buffer offset
    ///
    /// @param  hNativehandle       Pointer to Native Handle
    ///
    /// @return UINT64 offset value
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT64 GetBufferOffset(
        buffer_handle_t hNativehandle);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetBufferSize
    ///
    /// @brief  API to get Gralloc Buffer Size.
    ///
    /// @param  hNativehandle       Pointer to Native Handle
    ///
    /// @return INT32 size value
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT32 GetBufferSize(
        buffer_handle_t hNativehandle);

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~Gralloc
    ///
    /// @brief  Gralloc dtor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ~Gralloc();

    Gralloc()                                   = default;  ///< Default constructor
    Gralloc(const Gralloc& rOther)              = delete;   ///< Disallow the copy constructor
    Gralloc(const Gralloc&& rrOther)            = delete;   ///< Disallow the move  constructor
    Gralloc& operator=(const Gralloc& rOther)   = delete;   ///< Disallow assignment operator
    Gralloc& operator=(const Gralloc&& rrOther) = delete;   ///< Disallow move assignment operator

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Initialize
    ///
    /// @brief  Sets gralloc interface functions.
    ///
    /// @return CDKResult Enum
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CDKResult Initialize();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// InitializeMapper4
    ///
    /// @brief  Function to initialize Mapper#4.
    ///
    /// @return CDKResult Enum
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CDKResult InitializeMapper4();

    VOID* m_pGrallocIntf;       ///< Gralloc Interface Object, Typecast to GrallocInterface
};
#endif //(!CAMX_USE_GRALLOC1)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class that implements the ImageBuffer to allocate gralloc buffers and track reference count
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ImageBuffer
{
protected:
    ImageBuffer();
    ~ImageBuffer();

private:
    /// Static create function to make an instance of this class
#ifdef CAMX_USE_GRALLOC1
    static ImageBuffer* Create(Gralloc1Interface*  pGrallocInterface,
                               gralloc1_device_t*  pGralloc1Device,
                               UINT32              width,
                               UINT32              height,
                               UINT32              format,
                               UINT64              producerUsageFlags,
                               UINT64              consumerUsageFlags,
                               UINT32*             pStride);
    CDKResult AllocateGrallocBuffer(Gralloc1Interface*  pGrallocInterface,
                                    gralloc1_device_t*  pGralloc1Device,
                                    UINT32              width,
                                    UINT32              height,
                                    UINT32              format,
                                    UINT64              producerUsageFlags,
                                    UINT64              consumerUsageFlags,
                                    UINT32*             pStride);

    VOID Destroy(Gralloc1Interface*  pGrallocInterface,
             gralloc1_device_t*  pGralloc1Device);

#else
    static ImageBuffer* Create(UINT32              width,
                               UINT32              height,
                               UINT32              format,
                               UINT64              producerUsageFlags,
                               UINT64              consumerUsageFlags,
                               UINT32*             pStride);

    /// Allocate gralloc buffer
    CDKResult AllocateGrallocBuffer(UINT32              width,
                                    UINT32              height,
                                    UINT32              format,
                                    UINT64              producerUsageFlags,
                                    UINT64              consumerUsageFlags,
                                    UINT32*             pStride);

    /// Destroys an instance of the class
    VOID Destroy();
#endif

    /// Return reference count of this image buffer
    UINT32 GetReferenceCount();

    /// Add a reference to this image buffer
    VOID AddReference();

    /// Release a reference to this image buffer
    VOID ReleaseReference();

    /// Return the handle of this image buffer
    CHX_INLINE buffer_handle_t* GetBufferHandle()
    {
        return &pGrallocBuffer;
    }

    /// Return the buffer type of this image buffer
    CHX_INLINE CHIBUFFERTYPE GetBufferHandleType()
    {
        return ChiGralloc;
    }

    friend class CHIBufferManager;
    buffer_handle_t pGrallocBuffer;     ///< The buffer handle
    volatile UINT32 m_aReferenceCount;  ///< The reference count.
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief CHI Buffer Manager
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CHIBufferManager
{
public:
    /// Creates an instance of this class
    static CHIBufferManager* Create(
        const CHAR*                 pBufferManagerName,
        CHIBufferManagerCreateData* pCreateData);

    /// Destroys an instance of the class
    VOID Destroy();

    /// Return an Buffer info, which contains buffer handle and buffer type
    CHIBUFFERINFO GetImageBufferInfo();

    /// Add reference to the Buffer
    CDKResult AddReference(const CHIBUFFERINFO* pBufferInfo);

    /// Release reference to the Buffer
    CDKResult ReleaseReference(const CHIBUFFERINFO* pBufferInfo);

    /// Get reference of the Buffer
    UINT GetReference(const CHIBUFFERINFO* pBufferInfo);

    /// Activate this buffer manager. This gives a hint to manager underlying Image Buffer allocations
    CDKResult Activate();

    /// Deactivate this Buffer Manager. This releases all or some buffers allocated by the buffer manager
    CDKResult Deactivate(BOOL isPartialRelease);

    /// Bind backing buffer for this Buffer
    CDKResult BindBuffer(const CHIBUFFERINFO* pBufferInfo);

    // Get the Buffer size in the underlying buffer handle
    static UINT32 GetBufferSize(const CHIBUFFERINFO* pBufferInfo);

    /// Copy Buffer contents to another buffer
    static CDKResult CopyBuffer(const CHIBUFFERINFO*    pSrcBufferInfo,
                                CHIBUFFERINFO*          pDstBufferInfo);

    /// Set PerfMode metadata in this Buffer
    static VOID SetPerfMode(CHIBUFFERINFO* pBufferInfo);

    /// Get CPU virtual address for this Buffer
    static VOID* GetCPUAddress(
        const CHIBUFFERINFO*    pBufferInfo,
        INT                     size);

    /// Put CPU virtual address for this Buffer
    static VOID PutCPUAddress(
        const CHIBUFFERINFO*    pBufferInfo,
        INT                     size,
        VOID*                   pVirtualAddress);

    /// Get fd for this Buffer
    static INT GetFileDescriptor(const CHIBUFFERINFO* pBufferInfo);

    /// Get gralloc buffer_handle_t* address for this Buffer
    static buffer_handle_t* GetGrallocHandle(const CHIBUFFERINFO* pBufferInfo);

    /// Clean and/or invalidate the cache for all memory associated with this image buffer
    CDKResult CacheOps(
        const CHIBUFFERINFO* pBufferInfo,
        BOOL                 invalidate,
        BOOL                 clean);

private:
    CHIBufferManager();
    ~CHIBufferManager();

    /// Do not support the copy constructor or assignment operator
    CHIBufferManager(const CHIBufferManager&) = delete;
    CHIBufferManager& operator= (const CHIBufferManager&) = delete;

#ifdef CAMX_USE_GRALLOC1
    /// Setup gralloc1 interface functions
    CDKResult SetupGralloc1Interface();
#endif

    /// Initializes the instance
    CDKResult Initialize(
        const CHAR*                 pBufferManagerName,
        CHIBufferManagerCreateData* pCreateData);

    /// Reverse look up for image buffer pointer given buffer handle
    LightweightDoublyLinkedListNode* LookupImageBuffer(buffer_handle_t* pBufferHandle);

    /// Return an image buffer. Reference count is set to 1 when an image buffer is returned,
    /// Caller needs to release reference when the buffer is no longer needed.
    ImageBuffer* GetImageBuffer();

    /// Free buffers
    VOID FreeBuffers(BOOL isPartialFree);

    CHAR                            m_pBufferManagerName[MaxStringLength64];    ///< Name of the buffer manager
    BOOL                            m_bIsUnifiedBufferManagerEnabled;           ///< Boolean indicating if UBM is enabled
    CHIBUFFERMANAGERHANDLE          m_pUnifiedBufferManager;                    ///< Pointer to the Unified Buffer Manager
#ifdef CAMX_USE_GRALLOC1
    hw_module_t*                    m_pHwModule;                                ///< Gralloc1 module
    gralloc1_device_t*              m_pGralloc1Device;                          ///< Gralloc1 device
    Gralloc1Interface               m_grallocInterface;                         ///< Gralloc1 interface
#endif
    Mutex*                          m_pLock;                                    ///< Mutex protects free and busy list
    Condition*                      m_pWaitFreeBuffer;                          ///< Wait for a busy buffer becomes free
    LightweightDoublyLinkedList*    m_pFreeBufferList;                          ///< List manages free buffers
    LightweightDoublyLinkedList*    m_pBusyBufferList;                          ///< List manages busy buffers
    CHIBufferManagerCreateData      m_pBufferManagerData;                       ///< Data structure contains image buffer information
};

#endif // CHXUSECASEUTILS_H
