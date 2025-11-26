#ifndef __ACDB_DATA_PROC_H__
#define __ACDB_DATA_PROC_H__
/**
*=============================================================================
* \file acdb_data_proc.h
*
* \brief
*      Processes data for the ACDB SW commands.
*
* \copyright
*  Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*=============================================================================
*/

#include "acdb_delta_file_mgr.h"

/* ---------------------------------------------------------------------------
* Type Declarations
*--------------------------------------------------------------------------- */

typedef enum _acdb_data_persistance_type_t {
	ACDB_DATA_UNKNOWN,
    ACDB_DATA_NON_PERSISTENT = 0x0,
    ACDB_DATA_PERSISTENT,
    ACDB_DATA_GLOBALY_PERSISTENT,
    ACDB_DATA_DONT_CARE,
} AcdbDataPersistanceType;

/**< Specifies the clients that make requests to ACDB SW*/
typedef enum _acdb_data_client_t {
    ACDB_DATA_ACDB_CLIENT = 0x0,  /**< A client of ACDB SW (e.g GSL)*/
    ACDB_DATA_ATS_CLIENT          /**< A client of ATS (e.g ARCT)*/
}AcdbDataClient;

typedef struct _processor_module_iid_t ProcIIDMap;
#include "acdb_begin_pack.h"
struct _processor_module_iid_t
{
	uint32_t procID;
	uint32_t iid_mid_count;
	AcdbModuleInstance iid_mid_list[0];
}
#include "acdb_end_pack.h"
;

typedef struct _subgraph_module_iid_t SubgraphModuleIIDMap;
#include "acdb_begin_pack.h"
struct _subgraph_module_iid_t
{
	uint32_t subgraph_id;
	uint32_t proc_count;
	uint32_t size;
	ProcIIDMap *proc_info;
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_blob_module_info_t AcdbBlobModuleInfo;
#include "acdb_begin_pack.h"
struct _acdb_blob_module_info_t {
	//Offset of the module data within the AcdbBlob
	uint32_t blob_offset;

	//Subgraph that the module belongs to
	uint32_t subgraph_id;
}
#include "acdb_end_pack.h"
;

/**< Stores information about an Acdb File table*/
typedef struct _acdb_table_info_t AcdbTableInfo;
struct _acdb_table_info_t
{
    /**< Offset of the table in the ACDB file */
    uint32_t table_offset;
    /**< Size of the entire table */
    uint32_t table_size;
    /**< Size of the table entry */
    uint32_t table_entry_size;
};

/**< Contains infomation used to partitions and search in a sractch buffer*/
typedef struct _acdb_partition_search_info_t AcdbPartitionSearchInfo;
struct _acdb_partition_search_info_t
{
    /**< Number of Keys to use in the search */
    uint32_t num_search_keys;
    /**< Number of elements within the structure */
    uint32_t num_structure_elements;
    /**< Index of the entry within the table
    returned by the algorithm */
    uint32_t entry_index;
    /**< Offset of the entry within the table
    returned by the algorithm (In bytes)*/
    uint32_t entry_offset;
    /**< Buffer used to store and search partitions */
    AcdbBlob scratch_space;
    /**< Pointer to the structure containing the search keys */
    void *table_entry_struct;
};

/**< Contains infomation used to search a table for an item */
typedef struct _acdb_table_search_info_t AcdbTableSearchInfo;
struct _acdb_table_search_info_t
{
    /**< Number of Keys to use in the search */
    uint32_t num_search_keys;
    /**< Number of elements within the structure */
    uint32_t num_structure_elements;
    /**< Index of the entry within the table
    returned by the algorithm */
    uint32_t entry_index;
    /**< Offset of the entry within the table
    returned by the algorithm (In bytes)*/
    uint32_t entry_offset;
    /**< Pointer to the structure containing the search keys */
    void *table_entry_struct;
};

typedef struct _acdb_property_t AcdbProperty;
struct _acdb_property_t
{
    /**< Identifies the property */
    uint32_t property_id;
    /**< Size of the property data */
    uint32_t size;
    /**< The property data */
    uint8_t *property_data;
};

typedef struct _acdb_payload_t AcdbPayload;
struct _acdb_payload_t
{
    /**< Size of the payload */
    uint32_t size;
    /**< The payload */
    uint8_t *payload;
};

typedef struct acdb_hw_accel_subgraph_into_t AcdbHwAccelSubgraphInfo;
struct acdb_hw_accel_subgraph_into_t
{
    /**< The ID of the the subgraph that contains HW Accel data */
    uint32_t subgraph_id;
    /**< The type of memory used by the subgraphs calibration */
    uint32_t mem_type;
    /**< Offset of the module instance id list that also contains an offset
    to the list of parameters */
    uint32_t module_list_offset;
};

/* ---------------------------------------------------------------------------
* Function Prototypes
*--------------------------------------------------------------------------- */

/**
* \brief
*		Determines the persistence type of a Parameter. Persistence types are
*       Persist, Non-persist, and shared(aka global)
* \param[in] pid: Parameter ID to check the persistence type for.
* \param[in/out] persistence_type: Pointer to persistence type
* \return 0 on success, non-zero on failure
*/
int32_t DataProcGetPersistenceType(uint32_t pid, AcdbDataPersistanceType *persistence_type);

/**
* \brief
*		Retrieves calibration from the heap for a subgraph
* \param[in] persist_type: Get subgraph data for non-persistent, persistent, or globally persistent calibration
* \param[in] client: Indicates where the request originates from (GSL, ARCT, etc...).
* This is used by the heap to determine what type of nodes can be saved to the delta file
* \param[in] key_vector: key vector used to retrieve data from the heap. Can be CKV/TKV.
* \param[in] subgraph_id: The subgraph to retrieve data for
* \param[in] module_iid: The module to retrieve data for
* \param[in] param_id: The parameter to retrieve data for
* \param[in] is_offloaded_param: Indicates whether the parameter is offloaded
* \param[in] blob_offset: Offset of where to start writing data within the blob
* \param[in] blob: Calibration blob in the format MID, PID, ParamSize, Error Code, Payload
* \return 0 on success, non-zero on failure
*/
int32_t GetSubgraphCalData(
    AcdbDataPersistanceType persist_type, AcdbDataClient client,
    AcdbGraphKeyVector *key_vector,
    uint32_t subgraph_id, uint32_t module_iid, uint32_t param_id,
    bool_t is_offloaded_param, uint32_t *blob_offset, AcdbBlob *blob);

//TODO: Are these two functions needed? Remove if not
//int32_t DataProcGetCalData();
//int32_t DataProcGetTagData();

/**
* \brief
*		Sets calibration data to the heap based on the Key Vector and Subgraphs
* \param[in] cal_key_vector: The cal key vector that the subgraph data is based on
* \param[in] subgraph_param_data: The subgraphs and param data to set to the heap
* \sa AcdbSetCalDataReq
* \return 0 on success, non-zero on failure
*/
int32_t DataProcSetCalData(AcdbGraphKeyVector *cal_key_vector, AcdbSubgraphParamData* subgraph_param_data);

/**
* \brief
*		Sets tag data to the heap based on the Key Vector and Subgraphs
* \param[in] module_tag: Tag and Tag Key Vector and Subgraph data to set to the heap.
* \param[in] subgraph_param_data: The subgraphs and param data to set to the heap
* \sa AcdbSetTagDataReq
* \return 0 on success, non-zero on failure
*/
int32_t DataProcSetTagData(AcdbModuleTag *module_tag, AcdbSubgraphParamData* subgraph_param_data);

bool_t DoesSubgraphContainModule(uint32_t subgraph_id, uint32_t module_iid, int32_t* status);

/**
* \brief
* Retrieves the mapping between a subgraph and its module instance IDs
*
* \param[in] subgraph_id: subgraph to retrieve the mapping for
* \param[in/out] sg_iid_map: subgraph to instance ID map. This is a double pointer.
* Ex. pass in address of pointer(&*) or double pointer(**)
* \return 0 on success, non-zero on failure
*/
int32_t GetSubgraphIIDMap(uint32_t subgraph_id, SubgraphModuleIIDMap **sg_iid_map);

/**
* \brief
* Free the subgraph module instance id map
*
* \param[in/out] sg_iid_map: subgraph instance id map to free
*/
void FreeSubgraphIIDMap(SubgraphModuleIIDMap *sg_iid_map);

/**
* \brief
* Retrieves information about the nodes in the ACDB Heap
* This includes:
* 1. Key Vector
* 2. Map Size
* 3. Key Vector Type (CKV or TKV)
*
* \param[in] rsp: response data containing key vector and map size information for each heap node
*
* \return 0 on success, non-zero on failure
*/
int32_t DataProcGetHeapInfo(AcdbBufferContext *rsp);

/**
* \brief
* Retrieves heap data for one node in the heap. Each heap node is associated
* with one key vector
*
* \param[in] key_vector: the key vector to retrieve heap data for
* \param[in] rsp: response containing subgraph calibration data retrieved from the heap
*
* \sa AcdbDeltaSubgraphData
* \return 0 on success, non-zero on failure
*/
int32_t DataProcGetHeapData(AcdbGraphKeyVector *key_vector, AcdbBufferContext* rsp, uint32_t *blob_offset);

/**
* \brief
*       Writes the map to the heap. The map will be added if it doesnt exist on
*       the heap. Otherwise it will be diff/merged with an existing map.
*
* \param[in/out] map: The map to set to the heap
*
* \return 0 on success, non-zero on failure
*/
int32_t DataProcSetMapToHeap(AcdbDeltaKVToSubgraphDataMap *map);

/**
* \brief
*		Verifys whether a pid is globaly persistent.
* \param[in] pid: Parameter ID to verify for global persistance
* \return 0 on success, non-zero on failure
*/
int32_t IsPidGlobalyPersistent(uint32_t pid);

/**
* \brief
*		Verifys whether a pid is persistent.
* \param[in] pid: Parameter ID to verify for global persistance
* \return 0 on success, non-zero on failure
*/
int32_t IsPidPersistent(uint32_t pid);

/**
* \brief
* Retrieve tag data from the ACDB heap
*
* \param[in] aaaaaa:
*
* \return 0 on success, non-zero on failure
*/
//int32_t DataProcGetTagData(AcdbGraphKeyVector *tag_key_vector, uint32_t *blob_offset, AcdbBlob *blob);

/* \brief
*      Retrieves property data from the *.acdb file. The format
*      of the property blob is
*
*          Property = PropertySize PropertyData[PropertySize]
*
* \param[in/out] prop : the property to retrieve data for from the *.acdb file
*
* \sa AcdbGlobalProperty
* \return status
*/
int32_t GetAcdbProperty(AcdbProperty *prop);

/* \brief
*      Verifies whether a PID exists in the Offloaded Parameter Data Global
*      Property list. The format of the list is:
*
*
* \param[in] paramID : The parameter ID to check
* \param[in] AcdbUintList : A list of offloaded parameters retrieved from
*                      GetOffloadedParamList
*
* \sa ACDB_GBL_PROPID_OFFLOADED_PARAM_INFO
* \return status
*/
int32_t IsOffloadedParam(uint32_t param_id, AcdbUintList *offloaded_param_list);

/* \brief
*      Retrieves the Offloaaded parameter list from the Offloaded Parameter
*      Data Global Property. The format of the list is:
*
*          OffloadedParameterList = numPidEntries PIDEntry+
*          PIDEntry = parameterID
*
*      Offloaded parameters are parameters whoes actual data does not
*      reside in the ACDB Data files. This data is loaded from a file
*      on the target system and stitched into the calibration blob
*
* \param[out] AcdbUintList : A list of offloaded parameters retrieved from
*                      GetOffloadedParamList
*
* \sa ACDB_GBL_PROPID_OFFLOADED_PARAM_INFO
* \return status
*/
int32_t GetOffloadedParamList(AcdbUintList *offloaded_param_list);

/* \brief
*      Reads the offloaded param data stored on the file system into the
*      calibration buffer. It performs data alignment by adding padding to
*      the start of the file data and sets the starting offset of the data.
*
*      Parse the offloaded parameter data format.:
*
*          [ 4 Bytes        | 4 Bytes     | 4 Bytes   | File Size Bytes ]
*          [ Data Alignment | Data Offset | Data Size | Data Blob       ]
*
*      Offloaded parameters are parameters whoes actual data does not
*      reside in the ACDB Data files. This data is loaded from a file
*      on the target system and stitched into the calibration blob
*
* \param[in] calData : The ACDB calibration data that contains the file path
*                      to the parameter data file on the file system
* \param[in/out] paramSize : The size of the offloaded parameter which is
*                            sizeof(AcdbOffloadedParamHeader)
*                            + padding + data_size;
* \param[in/out] rsp : The buffer to write to
* \param[in/out] blob_offset : The current point to write to in the buffer
*
* \sa AcdbOffloadedParamHeader
* \sa ACDB_GPROP_OFFLOADED_PARAM_INFO
* \return status
*/
int32_t GetOffloadedParameterData(
    AcdbPayload *caldata, uint32_t *paramSize,
    AcdbBlob *rsp, uint32_t *blob_offset);

/**
* \brief
*       Determines the number of partitions to create based on the table
*       information provided
*
*       Formula for determining partition size:
*
*       Stratch Buffer Size - (Stratch Buffer Size % Table Entry Size)
*
*       E.x
*           buffer_size = 2500 bytes, table_entry_size = 12 bytes
*           2496 = 2500 - (2500 % 12)
*           so (2496/12) = 208 table entries can fit into one partition
*
* \depends Elements withing a table entry structure should be uint32_t
*
* \param[in] table_info: Contains information about an ACDB Data file table
* \param[in] scratch_buf_size: size of the buffer that will be used for
*           storing partitions
* \param[out] num_partitions: the recommended number of partitions based
*           on scratch_buf_size
* \param[in] partition_size: the size of each partition
*
* \return AR_EOK on success, non-zero otherwise
*/
int32_t GetPartitionInfo(AcdbTableInfo *table_info, uint32_t scratch_buf_size,
    uint32_t* num_partitions, uint32_t *partition_size);

/**
* \brief
*       Partitions an acdb file table and performs a binary search on each part.
*
* Example Setup
* //Setup Table Information
* table_info.table_offset = Starting Offset of the Table;
* table_info.table_size = Table Size;
* table_info.table_entry_size = sizeof(Table Entry);
*
* //Setup Search Information
* search_info.num_search_keys = search_index_count;
* search_info.num_structure_elements =
*     (sizeof(Table Entry) / sizeof(uint32_t));
* search_info.table_entry_struct = Entry to Search For;
* search_info.scratch_space.buf = &glb_buf_1;
* search_info.scratch_space.buf_size = sizeof(glb_buf_1);
*
* status = AcdbPartitionBinarySearch(&table_info, &search_info);
*
* \param[in] table_info: Contains information about an ACDB Data file table
* \param[in/out] part_info: Contains infomation used to
*               partitions and search in sractch buffer. The
*               part_info.table_entry_struct is updated if data is found
*
* \return AR_EOK on success, non-zero otherwise
*/
int32_t AcdbPartitionBinarySearch(AcdbTableInfo *table_info, AcdbPartitionSearchInfo *part_info);

/**
* \brief
*       Performs a binary search on an entire ACDB Table. No scratch buffers
*       are used like in AcdbPartitionBinarySearch(..)
*
* Example Setup
* //Setup Table Information
* table_info.table_offset = Starting Offset of the Table;
* table_info.table_size = Table Size;
* table_info.table_entry_size = sizeof(Table Entry);
*
* //Setup Search Information
* search_info.num_search_keys = search_index_count;
* search_info.num_structure_elements =
*     (sizeof(Table Entry) / sizeof(uint32_t));
* search_info.table_entry_struct = Entry to Search For;
*
* status = AcdbTableBinarySearch(&table_info, &search_info);
*
* \param[in] table_info: Contains information about an ACDB Data file table
* \param[in/out] part_info: Contains infomation used to
*               partitions and search in sractch buffer. The
*               part_info.table_entry_struct is updated if data is found
*
* \return AR_EOK on success, non-zero otherwise
* \sa AcdbPartitionBinarySearch
*/
int32_t AcdbTableBinarySearch(
    AcdbTableInfo *table_info, AcdbTableSearchInfo *part_info);

/* \brief
*      Retrieves the Hardware Accelration info for a Subgraph
*
* \param[in/out] info : The hw accel info for a subgraph. The caller provides
*                       the Subgraph ID
*
* \sa ACDB_GPROP_HW_ACCEL_SUBGRAPH_INFO
* \return 0 on success, non-zero on failure
*/
int32_t DataProcGetHwAccelSubgraphInfo(AcdbHwAccelSubgraphInfo* info);

/* \brief
*      Determines if a parameter has hardware acceleration data under a
*      module instance in some subgraph
*
* \depends
*      DataProcGetHwAccelSubgraphInfo(...) must be called before this API
*      to get the <module instance, param> list offset
*
* \param[in] module_list_offset : The hw accel <module instance, param>
*                                 list offset
* \param[in] module_iid : The module instance ID
* \param[in] param_id : The parameter ID
*
* \sa
* ACDB_GPROP_HW_ACCEL_SUBGRAPH_INFO
* DataProcGetHwAccelSubgraphInfo
*
* \return 0 on success, non-zero on failure
*/
int32_t DataProcIsHwAccelParam(uint32_t module_list_offset,
    uint32_t module_iid, uint32_t param_id);

#endif //__ACDB_DATA_PROC_H__
