#ifndef __ACDB_FILE_MGR_H__
#define __ACDB_FILE_MGR_H__
/**
*=============================================================================
* \file acdb_file_mgr.h
*
* \brief
*		This file contains the definition of the acdb file manager
*		interfaces.
*
* \copyright
*  Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*=============================================================================
*/

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include "acdb.h"
#include "acdb_utility.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */
#define ACDB_MAX_DEVICES 500
#define ACDB_MAX_ACDB_FILES 20
#define INF 4294967295U

#define PTR_ARG_COUNT(...) ((uint32_t)(sizeof((const void*[]){ __VA_ARGS__ })/sizeof(const void*)))

#define ACDB_GET_CHUNK_INFO(...) AcdbGetChunkInfo2(PTR_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)

/**< List of elements of the specified type where the list is a pointer to an array of those elements */
#define ACDB_LIST(type) struct _acdb_list_t \
{\
/**< Number of elemts in the list */\
uint32_t count;\
/**< Max number of elemts that this list can store */\
uint32_t max_count;\
/**< The size of an element. For example <uint a; uint b> is an element where the size is 8 bytes */\
uint32_t element_size;\
/**< Pointer to array of 'type' */\
type *list;\
}

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

 /**<Specifies the command set for the ACDB File Manager*/
enum AcdbFileManCmd {
    ACDB_FILE_MAN_SET_FILE_INFO = 0,
    ACDB_FILE_MAN_SET_TEMP_PATH_INFO,
    ACDB_FILE_MAN_GET_AVAILABLE_FILE_SLOTS,
    ACDB_FILE_MAN_RESET,
    ACDB_FILE_MAN_GET_FILE_NAME,
    ACDB_FILE_MAN_GET_LOADED_FILES_INFO,
    ACDB_FILE_MAN_GET_LOADED_FILE_DATA,
    ACDB_FILE_MAN_GET_TEMP_PATH_INFO
};

typedef enum _acdb_file_type_t AcdbFileType;
enum _acdb_file_type_t {
    ACDB_FILE_TYPE_UNKNOWN,
    ACDB_FILE_TYPE_DATABASE,
    ACDB_FILE_TYPE_WORKSPACE
};


typedef enum _acdb_op_t AcdbOp;
enum _acdb_op_t {
    ACDB_OP_NONE,
    ACDB_OP_GET_SIZE,
    ACDB_OP_GET_DATA,
    ACDB_OP_GET_PARAM_DATA,
    ACDB_OP_GET_MODULE_DATA,
    ACDB_OP_GET_SUBGRAPH_DATA,
};

typedef struct _chunk_info ChunkInfo;
#include "acdb_begin_pack.h"
struct _chunk_info {
	uint32_t chunk_id;
	uint32_t chunk_size;
	uint32_t chunk_offset;
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_def_dot_pair_t AcdbDefDotPair;
#include "acdb_begin_pack.h"
struct _acdb_def_dot_pair_t {
    uint32_t offset_def;
    uint32_t offset_dot;
}
#include "acdb_end_pack.h"
;

typedef struct _partition Partition;
#include "acdb_begin_pack.h"
struct _partition {
	uint32_t size;
	uint32_t offset;
}
#include "acdb_end_pack.h"
;

typedef struct _key_table_header_t KeyTableHeader;
#include "acdb_begin_pack.h"
struct _key_table_header_t {
    uint32_t num_keys;
    uint32_t num_entries;
}
#include "acdb_end_pack.h"
;

typedef struct _sg_list_header_t SgListHeader;
#include "acdb_begin_pack.h"
struct _sg_list_header_t {
    uint32_t size;
    uint32_t num_subgraphs;
}
#include "acdb_end_pack.h"
;

typedef struct _sg_list_obj_header_t SgListObjHeader;
#include "acdb_begin_pack.h"
struct _sg_list_obj_header_t {
    uint32_t subgraph_id;
    uint32_t num_subgraphs;
}
#include "acdb_end_pack.h"
;

/**< Represents and entry in the Module Tag Key Data LUT  or Tagged
Module LUT chunks */
typedef struct _subgraph_tag_key_lut_entry_t SubgraphTagLutEntry;
#include "acdb_begin_pack.h"
struct _subgraph_tag_key_lut_entry_t {
    /**< Subgraph ID */
	uint32_t sg_id;
    /**< Module Tag */
	uint32_t tag_id;
    /**<  Module Tag Key Data LUT: an offset of the tag key value lut
    Tagged Module LUT: an offset to a list of <MID, PID> pairs*/
	uint32_t offset;
}
#include "acdb_end_pack.h"
;

/**< Represents and entry in the Module Tag Key List LUT Chunk */
typedef struct _tag_key_list_entry_t TagKeyListEntry;
#include "acdb_begin_pack.h"
struct _tag_key_list_entry_t {
    /**< Module Tag */
    uint32_t tag_id;
    /**< Offset of the key list in the data pool */
    uint32_t key_list_offset;
}
#include "acdb_end_pack.h"
;

/**< A Tag and Definition Offset pair */
typedef struct _tag_def_offset_pair AcdbTagDefOffsetPair;
struct _tag_def_offset_pair {
    uint32_t tag_id;
    uint32_t def_offset;
};

typedef struct _gsl_cal_lut_entry_t GslCalLutEntry;
#include "acdb_begin_pack.h"
struct _gsl_cal_lut_entry_t {
	uint32_t mid;
	uint32_t offset_calkey_table;
	uint32_t offset_caldata_table;
}
#include "acdb_end_pack.h"
;

typedef struct _gsl_cal_lut_header_t GslCalDataLutHeader;
#include "acdb_begin_pack.h"
struct _gsl_cal_lut_header_t
{
	uint32_t num_keys;
	uint32_t num_entries;
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_file_man_path_info_t AcdbFileManPathInfo;
#include "acdb_begin_pack.h"
struct _acdb_file_man_path_info_t{
   uint32_t path_len;
   char path[MAX_FILENAME_LENGTH];
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_file_man_file_info_req_t AcdbCmdFileInfo;
#include "acdb_begin_pack.h"
struct _acdb_file_man_file_info_req_t{
	uint32_t filename_len;
	char chFileName[MAX_FILENAME_LENGTH];
	ar_fhandle file_handle;
    AcdbFileType file_type;
    uint32_t file_index;
	uint32_t file_size;
    void* file_buffer;
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_file_info_t AcdbFileInfo;
#include "acdb_begin_pack.h"
struct _acdb_file_info_t
{
   AcdbFileType file_type;
   uint32_t major;
   uint32_t minor;
   uint32_t revision;
   uint32_t cplInfo;
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_file_man_get_file_data_req_t AcdbFileManGetFileDataReq;
#include "acdb_begin_pack.h"
struct _acdb_file_man_get_file_data_req_t
{
	uint32_t file_offset;
	uint32_t file_data_len;
	uint32_t file_name_len;
	uint8_t *file_name;
    //uint32_t file_index;//TODO: This can replace the file name to make searching for the file quicker
}
#include "acdb_end_pack.h"
;
typedef struct _acdb_file_man_rsp_t AcdbFileManBlob;
#include "acdb_begin_pack.h"
struct _acdb_file_man_rsp_t
{
    /*Size of the blob*/
	uint32_t size;
    /*Number of bytes filled in the buffer*/
	uint32_t bytes_filled;
    /*The buffer that contains a data blob*/
	uint8_t *buf;
}
#include "acdb_end_pack.h"
;

typedef struct _calibration_identifier_map_t CalibrationIdMap;
#include "acdb_begin_pack.h"
struct _calibration_identifier_map_t
{
	uint32_t cal_id;
	uint32_t param_id;
	uint32_t cal_data_offset;
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_parameter_tag_data_t AcdbParameterTagData;
#include "acdb_begin_pack.h"
struct _acdb_parameter_tag_data_t {
    uint32_t subgraph_id;
    uint32_t module_iid;
    uint32_t parameter_id;
    AcdbModuleTag module_tag;
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_parameter_cal_data_t AcdbParameterCalData;
#include "acdb_begin_pack.h"
struct _acdb_parameter_cal_data_t {
    uint32_t subgraph_id;
    uint32_t module_iid;
    uint32_t parameter_id;
    AcdbGraphKeyVector cal_key_vector;
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_mod_iid_param_id_pair_t AcdbModIIDParamIDPair;
#include "acdb_begin_pack.h"
struct _acdb_mod_iid_param_id_pair_t {
    uint32_t module_iid;
    uint32_t parameter_id;
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_sg_cal_lut_header_t AcdbSgCalLutHeader;
#include "acdb_begin_pack.h"
struct _acdb_sg_cal_lut_header_t {
    uint32_t subgraph_id;
    uint32_t num_ckv_entries;
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_cal_key_tbl_entry AcdbCalKeyTblEntry;
#include "acdb_begin_pack.h"
struct _acdb_cal_key_tbl_entry {
    uint32_t offset_cal_key_tbl;
    uint32_t offset_cal_lut;
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_ckv_lut_entry_offsets_t AcdbCkvLutEntryOffsets;
#include "acdb_begin_pack.h"
struct _acdb_ckv_lut_entry_offsets_t {
    uint32_t offset_def;
    uint32_t offset_dot;
    uint32_t offset_dot2;
}
#include "acdb_end_pack.h"
;

/**< The Header format used when retrieving calibration data for the DSP */
typedef struct _acdb_dsp_module_header_t AcdbDspModuleHeader;
#include "acdb_begin_pack.h"
struct _acdb_dsp_module_header_t
{
    /**< Module Instance ID*/
    uint32_t module_iid;
    /**< Parameter ID*/
    uint32_t parameter_id;
    /**< Size of the parameter*/
    uint32_t param_size;
    /**< Error code set by the dsp*/
    uint32_t error_code;
}
#include "acdb_end_pack.h"
;

/**< Same as AcdbDspModuleHeader except that this format does not
contain the error code */
typedef struct _acdb_module_header_t AcdbModuleHeader;
#include "acdb_begin_pack.h"
struct _acdb_module_header_t
{
    /**< Module Instance ID*/
    uint32_t module_iid;
    /**< Parameter ID*/
    uint32_t parameter_id;
    /**< Size of the parameter*/
    uint32_t param_size;
}
#include "acdb_end_pack.h"
;

/*VCPM Structures Start*/
typedef struct _acdb_vcpm_subgraph_cal_header_t AcdbVcpmSubgraphCalHeader;
#include "acdb_begin_pack.h"
struct _acdb_vcpm_subgraph_cal_header_t
{
    /**< VCPM Module Instance ID*/
    uint32_t module_iid;
    /**< VCPM Parameter ID*/
    uint32_t parameter_id;
    /**< Number of VCPM Subgraphs*/
    uint32_t num_subgraphs;
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_vcpm_subgraph_cal_table_t AcdbVcpmSubgraphCalTable;
#include "acdb_begin_pack.h"
struct _acdb_vcpm_subgraph_cal_table_t
{
    /**< */
    uint32_t subgraph_id;
    /**< */
    uint32_t table_size;
    /**< */
    uint32_t major;
    /**< */
    uint32_t minor;
    /**< */
    uint32_t offset_vcpm_master_key_table;
    /**< */
    uint32_t num_ckv_data_table;
    //List of voice ckv data tables
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_vcpm_ckv_data_table_t AcdbVcpmCkvDataTable;
#include "acdb_begin_pack.h"
struct _acdb_vcpm_ckv_data_table_t
{
    /**< Size of the VCPM Calibration Data Table*/
    uint32_t table_size;
    /**< Offset of the Voice Calibration Key ID table in the Voice Key Chunk*/
    uint32_t offset_voice_key_table;
    /**< Size of the calibration data offset table*/
    uint32_t cal_dot_size;
    /**< Number of Calibration data objects*/
    uint32_t num_caldata_obj;
    //Cal Dot Table
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_vcpm_caldata_object_t AcdbVcpmCalDataObj;
#include "acdb_begin_pack.h"
struct _acdb_vcpm_caldata_object_t
{
    /**< Offset of the Voice CKV LUT in the VCPM CKV LUT Chunk*/
    uint32_t offset_vcpm_ckv_lut;
    /**< Offset of the VCPM Cal Definition table in the VCPM Cal Def Chunk*/
    uint32_t offset_cal_def;
    /**< The number of parameter calibration payloads in the data pool
    This should be the same as the number of <IID, PID> pairs in the
    Cal Def Table*/
    uint32_t num_data_offsets;
    //Cal Dot Tables...
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_vcpm_param_info_t AcdbVcpmParamInfo;
#include "acdb_begin_pack.h"
struct _acdb_vcpm_param_info_t
{
    /**< Offset of the parameter in the VCPM data pool*/
    uint32_t offset_vcpm_data_pool;
    /**< Flag to check the wheter a parameter is persistent. 0:false, 1:true*/
    uint32_t is_persistent;
}
#include "acdb_end_pack.h"
;


typedef struct _acdb_mid_pid_pair_t AcdbMiidPidPair;
#include "acdb_begin_pack.h"
struct _acdb_mid_pid_pair_t
{
    /**< Module Instance ID*/
    uint32_t module_iid;
    /**< Parameter ID*/
    uint32_t parameter_id;
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_vcpm_key_info_t AcdbVcpmKeyInfo;
#include "acdb_begin_pack.h"
struct _acdb_vcpm_key_info_t
{
    /**< The Voice Calibration Key ID*/
    uint32_t key_id;
    /**< */
    uint32_t is_dynamic;
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_vcpm_chunk_properties_t AcdbVcpmChunkProperties;
#include "acdb_begin_pack.h"
struct _acdb_vcpm_chunk_properties_t
{
    /**< The chunks base offset (excluding the chunk size)*/
    uint32_t base_offset;
    /**< The current offset */
    uint32_t offset;
    /**< The chunk size */
    uint32_t size;
}
#include "acdb_end_pack.h"
;

//typedef struct _acdb_vcpm_kv_info_t AcdbVcpmKvInfo;
//#include "acdb_begin_pack.h"
//struct _acdb_vcpm_kv_info_t
//{
//    /**< Flag for detecting an empty key vector
//    0:Not an empty key vector 1: Is an empty key vector*/
//    uint32_t is_zero_kv;
//    /**< Offset of key vector key(s) in VCPM Key ID Chunk */
//    uint32_t offset_cal_key_id;
//    /**< Offset of key vector value(s) in VCPM Key LUT Chunk */
//    uint32_t offset_cal_key_lut;
//}
//#include "acdb_end_pack.h"
//;

typedef struct _acdb_vcpm_offloaded_param_info_t AcdbVcpmOffloadedParamInfo;
struct _acdb_vcpm_offloaded_param_info_t
{
    /**< File offset to the <IID, PID> pair in the VCPM Cal DEF Chunk */
    uint32_t file_offset_cal_def;
    /**< File offset to the parameters payload in the Global Datapool Chunk */
    uint32_t file_offset_data_pool;
    /**< Blob offset of the CalDataObj[n].ParamInfo.offset in the VCPM
    Subgraph Info Chunk */
    uint32_t blob_offset_vcpm_param_info;
};

typedef struct _acdb_vcpm_blob_info_t AcdbVcpmBlobInfo;
struct _acdb_vcpm_blob_info_t
{
    /**< Current operation being performed */
    AcdbOp op;
    /**< The VCPM Subgraph ID*/
    uint32_t subgraph_id;
    /**< The Processor to get calibration for */
    uint32_t proc_id;
    /**< A flag indicating whether check for hw accel based calibraion */
    bool_t should_check_hw_accel;
    /**< An offset to a Subgrahps hardware accel module list */
    uint32_t hw_accel_module_list_offset;
    /**< Starting offset of the VCPM Framework Module blob */
    uint32_t offset_vcpm_blob;
    /**< The number of offloaded parameters found */
    uint32_t num_offloaded_params;
    /**< A pointer to the current offloaded parameter info node */
    LinkedListNode *curr_opi_node;
    /**< List of AcdbVcpmOffloadedParamInfo for writting offloaded params
    at the end of the vcpm datapool */
    LinkedList offloaded_param_info_list;
    /**< List of offloaded parameter IDs*/
    AcdbUintList offloaded_param_list;
    /**< 0:Zero CKV has NOT been writen 1: Zero CKV has been writen */
    //AcdbVcpmKvInfo zero_key_vector_info;
    /**< A flag indicating whether the module CKV found is the default key
    vector(no keys) */
    bool_t is_default_module_ckv;
    /**< A flag indicating whether process(true) or skip(false) the
    default CKV */
    bool_t ignore_get_default_data;
    /**< A flag indicating whether to add (true) iids to the referenced_iid_list
    or do nothing (false) */
    bool_t ignore_iid_list_update;
    /**< Keeps track of module IIDs that are used in CKVs. This does not include
    the default CKV. The list contains AcdbIidRefCount objects */
    AcdbUintList referenced_iid_list;
    /**< A list of relative offsets to the ckv data tables with data to write */
    AcdbUintList ckv_data_table_offset_list;
    /**< A list of absolute offsets to the found cal key id tables */
    AcdbUintList cal_key_id_table_offset_list;
    /**< VCPM Subgraph Info Chunk*/
    AcdbVcpmChunkProperties chunk_subgraph_info;
    /**< Master Key Table Chunk that contains the super set of calibration
    keys used in the subgraph */
    AcdbVcpmChunkProperties chunk_master_key;
    /**< Voice Cal Key Table chunk that contains various key combinations.
    The keys are a subset of the keys defined in the Master Key Table */
    AcdbVcpmChunkProperties chunk_cal_key_id;
    /**< Voice Cal Key Lookup table that contains the values of the key
    combinations in the Voice Cal Key Table chunk */
    AcdbVcpmChunkProperties chunk_cal_key_lut;
    /**< Datapool chunk that contains calibration data for modules within
    the vcpm subgraph */
    AcdbVcpmChunkProperties chunk_data_pool;
};

/*VCPM Structures End*/

/**<Manages the file information for all initialized acdb files*/
typedef struct _acdb_file_man_file_info_t AcdbFileManFileInfo;
#include "acdb_begin_pack.h"
struct _acdb_file_man_file_info_t
{
    /**<Number of ACDB files currently being managed*/
	uint32_t file_count;
    /**<Holds information about each opened acdb file*/
	AcdbCmdFileInfo fInfo[ACDB_MAX_ACDB_FILES];
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_property_entry_t AcdbPropertyEntry;
#include "acdb_begin_pack.h"
struct _acdb_property_entry_t
{
    uint32_t property_id;
    uint32_t offset_property_data;
}
#include "acdb_end_pack.h"
;

typedef struct _acdb_offloaded_param_header_t AcdbOffloadedParamHeader;
#include "acdb_begin_pack.h"
struct _acdb_offloaded_param_header_t
{
    /**< the alignment that determines the amount of padding added before the data */
    uint32_t alignment;
    /**< the starting offset of the data */
    uint32_t data_offset;
    /**< Size of the data starting at data_offset */
    uint32_t data_size;
    /*uint8_t data[0]*/
    /*padding = data_length % 4*/
}
#include "acdb_end_pack.h"
;

/**< Contains a list of subgraphs and parameter data under the subgraphs. The
parameter data can be calibration or tag data */
typedef struct _acdb_subgraph_param_data_t AcdbSubgraphParamData;
struct _acdb_subgraph_param_data_t {
    /**< The list of subgraphs */
    AcdbUintList subgraph_id_list;
    /**<  Configures the cal/tag data blob parser to bypass the
    error code if its not included with each parameter. For example,
    param data set from qact is in the fomrat:
    <Instance ID, Param ID, Size, Data[Size]>*/
    bool_t ignore_error_code;
    /**< The size of the subgraph parameter data */
    uint32_t data_size;
    /**< Parameter data in the format of:
    [1]<Instance ID, Param ID, Size, Error Code, Data[Size]>
    ...
    [n]<Instance ID, Param ID, Size, Error Code, Data[Size]>*/
    void* data;
};

/* ---------------------------------------------------------------------------
* Function Declarations and Documentation
*--------------------------------------------------------------------------- */

int32_t acdb_file_man_ioctl(uint32_t cmd_id,
    uint8_t *req,
    uint32_t sz_req,
    uint8_t *rsp,
    uint32_t sz_rsp);

int32_t AcdbGetChunkInfo(uint32_t chkID, uint32_t* chkBuf, uint32_t* chkLen);

int32_t AcdbGetChunkInfo2(uint32_t count, ...);

int file_seek(long offset, ar_fseek_reference_t origin);

int file_read(void *buffer, size_t ele_size);

int32_t FileSeekRead(void* buffer, size_t read_size, uint32_t *offset);

int32_t FileManReadBuffer(void* buffer, size_t read_size, uint32_t *offset);

int32_t FileManGetFilePointer1(void** file_ptr, size_t data_size,
    uint32_t *offset);

int32_t FileManGetFilePointer2(void** file_ptr, uint32_t offset);

int32_t FileManReadFileToBuffer(const char_t* fname, size_t *fsize,
    uint8_t *fbuffer, uint32_t fbuffer_size);

int32_t AcdbFileManGetAcdbDirectoryPath(AcdbFileManPathInfo** path_info);

#endif /* __ACDB_FILE_MGR_H__ */
