/* ===========================================================================
 * Copyright (c) 2016-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */
#include "test_util.h"
#include "CameraOSServices.h"

#ifndef __INTEGRITY /* Using expat on Integrity for parsing XML*/
#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#else
#include <string.h>
#include "expat.h"
#include "camera_config_dump.h"
#include "camera_config_cvbs_dump.h"
#endif

unsigned char gDigitalMatrixUyvy[DIGITAL_NUM][DIGITAL_SIZE_UYVY] = DIGITAL_MATRIX_UYVY_INIT;
unsigned char gDigitalMatrixNv12[DIGITAL_NUM][DIGITAL_SIZE_NV12] = DIGITAL_MATRIX_NV12_INIT;
unsigned char gDigitalMatrixYu12[DIGITAL_NUM][DIGITAL_SIZE_YU12] = DIGITAL_MATRIX_YU12_INIT;
unsigned char gDigitalMatrixRgb888[DIGITAL_NUM][DIGITAL_SIZE_RGB888] = DIGITAL_MATRIX_RGB888_INIT;

#ifndef __INTEGRITY
#define XML_GET_INT_ATTR(_var_, _node_, _attr_, _opt_, _type_, _default_) \
do { \
    xmlChar* _p_ = xmlGetProp(_node_, (const xmlChar *)_attr_); \
    if (_p_) { \
        _var_ = (_type_)strtoul((const char *)_p_, NULL, 0);; \
        xmlFree(_p_); \
    } else if (_opt_) {\
        _var_ = (_type_)_default_; \
    } else  { \
        QCARCAM_ERRORMSG("could not get attribute " _attr_); \
        return -1; \
    } \
} while(0)

#define XML_GET_FLOAT_ATTR(_var_, _node_, _attr_, _opt_, _default_) \
do { \
    xmlChar* _p_ = xmlGetProp(_node_, (const xmlChar *)_attr_); \
    if (_p_) { \
        _var_ = strtof((const char *)_p_, NULL); \
        xmlFree(_p_); \
    } else if (_opt_) {\
        _var_ = _default_; \
    } else  { \
        QCARCAM_ERRORMSG("could not get attribute " _attr_); \
        return -1; \
    } \
} while(0)

#define XML_GET_STRING_ATTR(_var_, _node_, _attr_, _opt_, _default_) \
do { \
    xmlChar* _p_ = xmlGetProp(_node_, (const xmlChar *)_attr_); \
    if (_p_) { \
        snprintf(_var_, sizeof(_var_), "%s", _p_); \
        xmlFree(_p_); \
    } else if (_opt_) {\
        snprintf(_var_, sizeof(_var_), "%s", _default_); \
    } else  { \
        QCARCAM_ERRORMSG("could not get attribute " _attr_); \
        return -1; \
    } \
} while(0)
#else
#define XML_GET_INT_ATTR(_var_, _attr_, _label_, _opt_, type, _default_) \
{ \
    int _p_ = xml_find_attr(_label_, _attr_); \
    if (_p_ != -1) { \
        _var_ = (type)atoi((const char *)_attr_[_p_ + 1]); \
    } else if (_opt_) {\
        _var_ = (type)_default_; \
    } else  { \
        QCARCAM_ERRORMSG("could not get attribute " _label_); \
    } \
}

#define XML_GET_FLOAT_ATTR(_var_,_attr_, _label_, _opt_, _default_) \
{ \
    int _p_ = xml_find_attr(_label_, _attr_); \
    if (_p_ != -1) { \
        _var_ = atof((const char *)_attr_[_p_ + 1]); \
    } else if (_opt_) {\
        _var_ = _default_; \
    } else  { \
        QCARCAM_ERRORMSG("could not get attribute " _label_); \
    } \
}

#define XML_GET_STRING_ATTR(_var_, _attr_, _label_, _opt_, _default_) \
{ \
    int _p_ = xml_find_attr(_label_, _attr_); \
    if (_p_ != -1) { \
        snprintf(_var_, sizeof(_var_), "%s", _attr_[_p_ + 1]); \
    } else if (_opt_) {\
        snprintf(_var_, sizeof(_var_), "%s", _default_); \
    } else  { \
        QCARCAM_ERRORMSG("could not get attribute " _label_); \
    } \
}
#endif

static test_util_color_fmt_t translate_format_string(char* format)
{
    if (!strncmp(format, "rgba8888", strlen("rgba8888")))
    {
        return TESTUTIL_FMT_RGBX_8888;
    }
    else if (!strncmp(format, "rgb888", strlen("rgb888")))
    {
        return TESTUTIL_FMT_RGB_888;
    }
    else if (!strncmp(format, "rgb565", strlen("rgb565")))
    {
        return TESTUTIL_FMT_RGB_565;
    }
    else if (!strncmp(format, "uyvy", strlen("uyvy")))
    {
        return TESTUTIL_FMT_UYVY_8;
    }
    else if (!strncmp(format, "nv12", strlen("nv12")))
    {
        return TESTUTIL_FMT_NV12;
    }
    else if (!strncmp(format, "yu12", strlen("yu12")))
    {
        return TESTUTIL_FMT_YU12;
    }
    else if (!strncmp(format, "p010", strlen("p010")))
    {
        return TESTUTIL_FMT_P010;
    }
    else
    {
        QCARCAM_ERRORMSG("Default to RGB565");
        return TESTUTIL_FMT_RGB_565;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_di_sw_weave_30fps
///
/// @brief Deinterlace 2 fields from souce buf into dest new frame with software weave 30fps method
///
/// @param di_info         Input souce/dest buffer context
/// @param test_util_get_buf_ptr_func    Helper func to get source/destination buffer virtual address
///
/// @return Void
///////////////////////////////////////////////////////////////////////////////
void test_util_di_sw_weave_30fps(test_util_sw_di_t *di_info)
{
    unsigned char *odd_field_ptr = NULL;
    unsigned char *even_field_ptr = NULL;
    unsigned char *source_ptr = NULL;
    unsigned char *dest_ptr = NULL;
    test_util_buf_ptr_t buf_ptr;
    uint32_t line;

    buf_ptr.buf_idx = di_info->source_buf_idx;
    test_util_get_buf_ptr(di_info->qcarcam_window, &buf_ptr);
    source_ptr = buf_ptr.p_va[0];
    line = buf_ptr.stride[0];

    test_util_get_buf_ptr(di_info->display_window, &buf_ptr);
    dest_ptr = buf_ptr.p_va[0];


    if (di_info->field_type == QCARCAM_FIELD_ODD_EVEN)
    {
        odd_field_ptr = source_ptr + line * DEINTERLACE_ODD_HEADER_HEIGHT;
        even_field_ptr = source_ptr + line * (DEINTERLACE_ODD_HEIGHT + DEINTERLACE_EVEN_HEADER_HEIGHT);
    }
    else if (di_info->field_type == QCARCAM_FIELD_EVEN_ODD)
    {
        even_field_ptr = source_ptr + line * DEINTERLACE_EVEN_HEADER_HEIGHT;
        odd_field_ptr = source_ptr + line * (DEINTERLACE_EVEN_HEIGHT + DEINTERLACE_ODD_HEADER_HEIGHT);
    }
    else
    {
        /* shouldn't display this frame, so do nothing */
        return;
    }

    for(int i = 0; i < DEINTERLACE_FIELD_HEIGHT; i++)
    {
        memcpy(dest_ptr, odd_field_ptr, line);
        dest_ptr += line;
        odd_field_ptr += line;
        memcpy(dest_ptr, even_field_ptr, line);
        dest_ptr += line;
        even_field_ptr += line;
    }
}

#ifndef __INTEGRITY
static int parse_isp_instance_element(xmlDocPtr doc, xmlNodePtr cur, qcarcam_isp_usecase_config_t* isp_config)
{
    CAM_UNUSED(doc);

    XML_GET_INT_ATTR(isp_config->id, cur, "id", 0, unsigned int, 0);
    XML_GET_INT_ATTR(isp_config->camera_id, cur, "camera_id", 0, unsigned int, 0);
    XML_GET_INT_ATTR(isp_config->use_case, cur, "use_case", 0, qcarcam_isp_usecase_t, 0);

    return 0;
}

static int parse_input_element(xmlDocPtr doc, xmlNodePtr parent, test_util_xml_input_t* input)
{
    xmlNodePtr cur = parent->xmlChildrenNode;

    CAM_UNUSED(doc);

    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"properties"))) {
            XML_GET_INT_ATTR(input->properties.qcarcam_input_id, cur, "input_id", 0, qcarcam_input_desc_t, 0);
            XML_GET_INT_ATTR(input->properties.use_event_callback, cur, "event_callback", 1, int, 1);
            XML_GET_INT_ATTR(input->properties.frame_timeout, cur, "frame_timeout", 1, int, -1);
            XML_GET_INT_ATTR(input->properties.op_mode, cur, "op_mode", 1, qcarcam_opmode_type, QCARCAM_OPMODE_MAX);
            XML_GET_INT_ATTR(input->properties.subscribe_parameter_change, cur, "subscribe_parameter_change", 1, int, 0);
            XML_GET_INT_ATTR(input->properties.delay_time, cur, "delay_time", 1, int, 0);
            XML_GET_INT_ATTR(input->properties.recovery, cur, "recovery", 1, bool, 0);
            XML_GET_STRING_ATTR(input->properties.node_name, cur, "v4l2_node", 1, "/dev/video51");
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"display_setting")))
        {
            char format[16];

            XML_GET_INT_ATTR(input->window_params.display_id, cur, "display_id", 0, int, 0);
            XML_GET_INT_ATTR(input->window_params.is_offscreen, cur, "offscreen", 1, int, 0);

            XML_GET_FLOAT_ATTR(input->window_params.window_size[0], cur, "window_width", 1, 1.0f);
            XML_GET_FLOAT_ATTR(input->window_params.window_size[1], cur, "window_height", 1, 1.0f);
            XML_GET_FLOAT_ATTR(input->window_params.window_pos[0], cur, "window_pos_x", 1, 0.0f);
            XML_GET_FLOAT_ATTR(input->window_params.window_pos[1], cur, "window_pos_y", 1, 0.0f);
            XML_GET_FLOAT_ATTR(input->window_params.window_source_size[0], cur, "src_width", 1, 1.0f);
            XML_GET_FLOAT_ATTR(input->window_params.window_source_size[1], cur, "src_height", 1, 1.0f);
            XML_GET_FLOAT_ATTR(input->window_params.window_source_pos[0], cur, "src_x", 1, 0.0f);
            XML_GET_FLOAT_ATTR(input->window_params.window_source_pos[1], cur, "src_y", 1, 0.0f);
            XML_GET_INT_ATTR(input->window_params.zorder, cur, "zorder", 1, int, 0);
            XML_GET_INT_ATTR(input->window_params.pipeline_id, cur, "pipeline", 1, int, -1);
            XML_GET_INT_ATTR(input->window_params.n_buffers_display, cur, "nbufs", 1, int, 5);
            XML_GET_STRING_ATTR(format, cur, "format", 1, "uyvy");

            input->window_params.format = translate_format_string(format);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"output_setting")))
        {
            XML_GET_INT_ATTR(input->output_params.n_buffers, cur, "nbufs", 1, int, 5);
            XML_GET_INT_ATTR(input->output_params.width, cur, "width", 1, int, -1);
            XML_GET_INT_ATTR(input->output_params.height, cur, "height", 1, int, -1);
            XML_GET_INT_ATTR(input->output_params.stride, cur, "stride", 1, int, -1);
            XML_GET_INT_ATTR(input->output_params.format, cur, "format", 1, qcarcam_color_fmt_t, -1);
            XML_GET_INT_ATTR(input->output_params.frame_rate_config.frame_drop_mode, cur, "framedrop_mode", 1, qcarcam_frame_drop_mode_t, QCARCAM_KEEP_ALL_FRAMES);
            XML_GET_INT_ATTR(input->output_params.frame_rate_config.frame_drop_period, cur, "framedrop_period", 1, char, 0);
            XML_GET_INT_ATTR(input->output_params.frame_rate_config.frame_drop_pattern, cur, "framedrop_pattern", 1, int, 0);
            XML_GET_INT_ATTR(input->output_params.num_batch_frames, cur, "batch_frames", 1, int, 1);
            XML_GET_INT_ATTR(input->output_params.frame_increment, cur, "frame_increment", 1, int, 0);
        }
#ifdef ENABLE_INJECTION_SUPPORT
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"inject_setting")))
        {
            XML_GET_INT_ATTR(input->inject_params.framedrop_pattern, cur, "framedrop_pattern", 0, qcarcam_color_pattern_t, 0);
            XML_GET_INT_ATTR(input->inject_params.bitdepth, cur, "bitdepth", 0, qcarcam_color_bitdepth_t, 0);
            XML_GET_INT_ATTR(input->inject_params.pack, cur, "pack", 0, qcarcam_color_pack_t, 0);
            XML_GET_INT_ATTR(input->inject_params.buffer_size[0], cur, "width", 0, int, 0);
            XML_GET_INT_ATTR(input->inject_params.buffer_size[1], cur, "height", 0, int, 0);
            XML_GET_INT_ATTR(input->inject_params.stride, cur, "stride", 0, int, 0);
            XML_GET_INT_ATTR(input->inject_params.n_buffers, cur, "nbufs", 1, int, 1);
            XML_GET_STRING_ATTR(input->inject_params.filename, cur, "filename", 1, "bayer_input.raw");
        }
#endif
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"exposure_setting")))
        {
            XML_GET_FLOAT_ATTR(input->exp_params.exposure_time, cur, "exp_time", 1, 31.147f);
            XML_GET_FLOAT_ATTR(input->exp_params.gain, cur, "gain", 1, 1.5f);
            XML_GET_INT_ATTR(input->exp_params.manual_exp, cur, "enable_manual", 1, int, -1);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"isp_setting")))
        {
            int rc = 0;
            unsigned int numInstances = 0;
            xmlNodePtr Child = cur->xmlChildrenNode;
            while (Child != NULL)
            {
                if ((!xmlStrcmp(Child->name, (const xmlChar *)"isp_instance")))
                {
                    rc = parse_isp_instance_element(doc, Child, &input->isp_params.isp_config[numInstances]);
                    if (!rc)
                    {
                        numInstances++;
                    }
                    else
                    {
                        QCARCAM_ERRORMSG("Missing parameter in config file");
                    }

                    if (numInstances >= QCARCAM_MAX_ISP_INSTANCES)
                    {
                        QCARCAM_ERRORMSG("Parsed max number of ISP instances (%d). Skip rest", numInstances);
                        break;
                    }
                }
                Child = Child->next;
            }
            input->isp_params.num_isp_instances = numInstances;
        }

        cur = cur->next;
    }

    return 0;
}

static int parse_latency_mode(xmlDocPtr doc, xmlNodePtr parent, test_util_global_config_t *global_config)
{
    xmlNodePtr cur = parent->xmlChildrenNode;
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"properties"))) {
            XML_GET_INT_ATTR(global_config->latency_measurement_mode, cur, "latency_mode", 0, CameraLatencyMeasurementModeType, 0);
        }

        cur = cur->next;
    }

    return 0;
}

int test_util_parse_xml_config_file(const char *filename, test_util_xml_input_t *xml_inputs, unsigned int max_num_inputs, test_util_global_config_t *global_config)
{
    int rc = 0;
    unsigned int numInputs = 0;

    xmlDocPtr doc;
    xmlNodePtr cur;

    doc = xmlParseFile(filename);

    if (doc == NULL)
    {
        QCARCAM_ERRORMSG("Document not parsed successfully");
        return -1;
    }

    cur = xmlDocGetRootElement(doc);

    if (cur == NULL)
    {
        QCARCAM_ERRORMSG("Empty config file");
        xmlFreeDoc(doc);
        return -1;
    }

    if (xmlStrcmp(cur->name, (const xmlChar *) "qcarcam_inputs"))
    {
        QCARCAM_ERRORMSG("Wrong config file format, root node != qcarcam_inputs");
        xmlFreeDoc(doc);
        return -1;
    }

    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"input_device")))
        {
            rc = parse_input_element(doc, cur, &xml_inputs[numInputs]);
            if (!rc)
            {
                numInputs++;
            }
            else
            {
                QCARCAM_ERRORMSG("Missing parameter in config file");
            }

            if (numInputs >= max_num_inputs)
            {
                QCARCAM_ERRORMSG("Parsed max number of inputs (%d). Skip rest", numInputs);
                break;
            }
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"extra_mode")))
        {
            rc = parse_latency_mode(doc, cur, global_config);
            if (rc)
            {
                QCARCAM_ERRORMSG("Missing parameter in config file");
            }
        }
        cur = cur->next;
    }

    xmlFreeDoc(doc);
    return numInputs;
}
#else // #ifndef __INTEGRITY

#define MAX_ATTR 30                // Maximum number of attributes for each element in XML
#define MAX_READ_BUFF_SIZE 10000   // Maximum buffer size for file read in bytes

struct xml_data
{
    int root_found;
    int numInputs;
    XML_Parser parser;
    test_util_xml_input_t* inputs;
};

int xml_find_attr(const char *label, const char **attr)
{
    int rc = -1;
    for (int x = 0; x < MAX_ATTR; x += 2)
    {
        if (attr[x] == NULL)
        {
            break;
        }
        else if(!strncmp(label, attr[x], strlen(attr[x])))
        {
            rc = x;
            break;
        }
    }
    return rc;
}

static void xml_read_element(const char *name, const char **attr, test_util_xml_input_t* input)
{
    if (!strncmp(name, "properties", strlen("properties")))
    {
        XML_GET_INT_ATTR(input->properties.qcarcam_input_id, attr, "input_id", 0, qcarcam_input_desc_t, 0);
        XML_GET_INT_ATTR(input->properties.use_event_callback, attr, "event_callback", 1, int, 1);
        XML_GET_INT_ATTR(input->properties.frame_timeout, attr, "frame_timeout", 1, int, -1);
        XML_GET_INT_ATTR(input->properties.op_mode, attr, "op_mode", 1, qcarcam_opmode_type, QCARCAM_OPMODE_MAX);
        XML_GET_INT_ATTR(input->properties.subscribe_parameter_change, attr, "subscribe_parameter_change", 1, int, 0);
        XML_GET_INT_ATTR(input->properties.delay_time, attr, "delay_time", 1, int, 0);
        XML_GET_INT_ATTR(input->properties.recovery, attr, "recovery", 1, bool, 0);

    }
    else if (!strncmp(name, "display_setting", strlen("display_setting")))
    {
        char format[16];

        XML_GET_INT_ATTR(input->window_params.display_id, attr, "display_id", 0, int, 0);
        XML_GET_FLOAT_ATTR(input->window_params.window_size[0], attr, "window_width", 1, 1.0f);
        XML_GET_FLOAT_ATTR(input->window_params.window_size[1], attr, "window_height", 1, 1.0f);
        XML_GET_FLOAT_ATTR(input->window_params.window_pos[0], attr, "window_pos_x", 1, 0.0f);
        XML_GET_FLOAT_ATTR(input->window_params.window_pos[1], attr, "window_pos_y", 1, 0.0f);
        XML_GET_FLOAT_ATTR(input->window_params.window_source_size[0], attr, "src_width", 1, 1.0f);
        XML_GET_FLOAT_ATTR(input->window_params.window_source_size[1], attr, "src_height", 1, 1.0f);
        XML_GET_FLOAT_ATTR(input->window_params.window_source_pos[0], attr, "src_x", 1, 0.0f);
        XML_GET_FLOAT_ATTR(input->window_params.window_source_pos[1], attr, "src_y", 1, 0.0f);
        XML_GET_INT_ATTR(input->window_params.zorder, attr, "zorder", 1, int, 0);
        XML_GET_INT_ATTR(input->window_params.pipeline_id, attr, "pipeline", 1, int, -1);
        XML_GET_INT_ATTR(input->window_params.n_buffers_display, attr, "nbufs", 1, int, 5);
        XML_GET_STRING_ATTR(format, attr, "format", 1, "uyvy");

        input->window_params.format = translate_format_string(format);
    }
    else if (!strncmp(name, "output_setting", strlen("output_setting")))
    {
        XML_GET_INT_ATTR(input->output_params.n_buffers, attr, "nbufs", 1, int, 5);
        XML_GET_INT_ATTR(input->output_params.width, attr, "width", 1, int, -1);
        XML_GET_INT_ATTR(input->output_params.height, attr, "height", 1, int, -1);
        XML_GET_INT_ATTR(input->output_params.stride, attr, "stride", 1, int, -1);
        XML_GET_INT_ATTR(input->output_params.format, attr, "format", 1, qcarcam_color_fmt_t, -1);
        XML_GET_INT_ATTR(input->output_params.frame_rate_config.frame_drop_mode, attr, "framedrop_mode", 1, qcarcam_frame_drop_mode_t, QCARCAM_KEEP_ALL_FRAMES);
        XML_GET_INT_ATTR(input->output_params.frame_rate_config.frame_drop_period, attr, "framedrop_period", 1, char, 0);
        XML_GET_INT_ATTR(input->output_params.frame_rate_config.frame_drop_pattern, attr, "framedrop_pattern", 1, int, 0);
        XML_GET_INT_ATTR(input->output_params.num_batch_frames, attr, "batch_frames", 1, int, 1);
        XML_GET_INT_ATTR(input->output_params.frame_increment, cur, "frame_increment", 1, int, 0);
    }
#ifdef ENABLE_INJECTION_SUPPORT
    else if (!strncmp(name, "inject_setting", strlen("inject_setting")))
    {
        XML_GET_INT_ATTR(input->inject_params.framedrop_pattern, attr, "framedrop_pattern", 0, qcarcam_color_pattern_t, 0);
        XML_GET_INT_ATTR(input->inject_params.bitdepth, attr, "bitdepth", 0, qcarcam_color_bitdepth_t, 0);
        XML_GET_INT_ATTR(input->inject_params.pack, attr, "pack", 0, qcarcam_color_pack_t, 0);
        XML_GET_INT_ATTR(input->inject_params.buffer_size[0], attr, "width", 0, int, 0);
        XML_GET_INT_ATTR(input->inject_params.buffer_size[1], attr, "height", 1, int, 0);
        XML_GET_INT_ATTR(input->inject_params.stride, attr, "stride", 0, int, 0);
        XML_GET_INT_ATTR(input->inject_params.n_buffers, attr, "nbufs", 1, int, 1);
        XML_GET_STRING_ATTR(input->inject_params.filename, attr, "filename", 1, "bayer_input.raw");
    }
#endif
    else if (!strncmp(name, "exposure_setting", strlen("exposure_setting")))
    {
        XML_GET_FLOAT_ATTR(input->exp_params.exposure_time, attr, "exp_time", 1, 31.147f);
        XML_GET_FLOAT_ATTR(input->exp_params.gain, attr, "gain", 1, 1.5f);
        XML_GET_INT_ATTR(input->exp_params.manual_exp, attr, "enable_manual", 1, int, -1);
    }
}

static void open_element(void *data, const char *name, const char **attr)
{
    xml_data *p_data = (xml_data *)data;

    if (!strncmp(name, "qcarcam_inputs", strlen("qcarcam_inputs")))
    {
        p_data->root_found = 1;
    }

    if (p_data->root_found)
    {
        xml_read_element(name, attr, &p_data->inputs[p_data->numInputs]);
    }
    else
    {
        QCARCAM_ERRORMSG("Document not parsed successfully");
        XML_StopParser(p_data->parser, 0);
    }
}

static void close_element(void *data, const char *name)
{
    xml_data *p_data = (xml_data *)data;

    if (!strncmp(name, "input_device", strlen("input_device")))
    {
        p_data->numInputs++;
    }
}

int test_util_parse_xml_config_file(const char* filename, test_util_xml_input_t* xml_inputs, unsigned int max_num_inputs)
{
    FILE *fp = NULL;
    xml_data data = {};
    char read_buffer[MAX_READ_BUFF_SIZE] = {0};
    size_t length = 0;

    data.inputs = xml_inputs;

    data.parser = XML_ParserCreate(NULL);
    XML_SetUserData(data.parser, &data);
    XML_SetElementHandler(data.parser, open_element, close_element);

    fp = fopen(filename, "rb");

    if (fp != NULL)
    {
        QCARCAM_INFOMSG("Reading config file : ​%s", filename);

        length = fread(&read_buffer, sizeof(char), MAX_READ_BUFF_SIZE, fp);

        if (!XML_Parse(data.parser, read_buffer, length, 1))
        {
            QCARCAM_ERRORMSG("%s at line %lu\n", XML_ErrorString(XML_GetErrorCode(data.parser)), XML_GetCurrentLineNumber(data.parser));
            return 0;
        }

        fclose(fp);
    }
    else
    {
        if (!strncmp(filename, "/ghs_test/Camera/config/qcarcam_config_single_cvbs.xml",
                     strlen("/ghs_test/Camera/config/qcarcam_config_single_cvbs.xml")))
        {
            length = static_cast<size_t>(sizeof(xml_dump_cvbs)/sizeof(xml_dump_cvbs[0]));
            if (!XML_Parse(data.parser, xml_dump_cvbs, length, 1))
            {
                QCARCAM_ERRORMSG("%s at line %lu\n", XML_ErrorString(XML_GetErrorCode(data.parser)), XML_GetCurrentLineNumber(data.parser));
                return 0;
            }
        }
        else
        {
            length = static_cast<size_t>(sizeof(xml_dump)/sizeof(xml_dump[0]));
            if (!XML_Parse(data.parser, xml_dump, length, 1))
            {
                QCARCAM_ERRORMSG("%s at line %lu\n", XML_ErrorString(XML_GetErrorCode(data.parser)), XML_GetCurrentLineNumber(data.parser));
                return 0;
            }
        }
    }

    XML_ParserFree(data.parser);
    return data.numInputs;
}
#endif

///////////////////////////////////////////////////////////////////////////////
/// test_util_get_time
///
/// @brief Get current time
///
/// @param pTime          Pointer to current time
///
/// @return QCARCAM_RET_OK if successful
///////////////////////////////////////////////////////////////////////////////
int test_util_get_time(unsigned long long *pTime)
{
    uint64 tm = 0;
    CameraGetTime(&tm);
    if( pTime ){
        *pTime = tm/1000000;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// test_util_kpi_render_text
///
/// @brief render text to bufferId on pWindow
///
/// @param text          render text
///
/// @param pWindow       rendered window
///
/// @param bufferId      the bufferId on pWindow
///
/// @param buffers       qcarcam_buffers_t: to get buffer information
///////////////////////////////////////////////////////////////////////////////
void test_util_kpi_render_text(const char* text, test_util_window_t* pWindow, unsigned int bufferId, qcarcam_buffers_t* buffers)
{
    test_util_buf_ptr_t buf_ptr;
    int i, j;
    unsigned char* buf;


    buf_ptr.buf_idx = bufferId;
    test_util_get_buf_ptr(pWindow, &buf_ptr);

    QCARCAM_DBGMSG("buf_ptr -- idx: %u, p_va[0]: %p, stride: %u", buf_ptr.buf_idx, buf_ptr.p_va[0]);

    buf = (unsigned char* )buf_ptr.p_va[0];

    for (i = 0; i < strlen(text); i++)
    {
        unsigned char* digital_matrix;
        unsigned int digit;

        if (text[i] >= '0' && text[i] <= '9')
            digit = (unsigned int)(text[i] - '0');
        else if (text[i] == '.')
            digit = DOT;
        else
            digit = SPACE;

        switch (buffers->color_fmt)
        {
            case QCARCAM_FMT_UYVY_8:
            {
                digital_matrix = gDigitalMatrixUyvy[digit];

                for (j = 0; j < DIGITAL_HEIGHT; j++)
                {
                    unsigned int stride = buffers->buffers[bufferId].planes[0].stride;
                    memcpy((void* )&buf[j*stride], (void* )&digital_matrix[j*DIGITAL_STRIDE_UYVY], DIGITAL_STRIDE_UYVY);
                }

                buf += DIGITAL_STRIDE_UYVY;
                break;
            }
            case QCARCAM_FMT_NV12:
            {
                digital_matrix = gDigitalMatrixNv12[digit];

                for (j = 0; j < DIGITAL_HEIGHT_NV12_PLANE0; j++)
                {
                    unsigned int stride0 = buffers->buffers[bufferId].planes[0].stride;
                    memcpy((void* )&buf[j*stride0], (void* )&digital_matrix[j*DIGITAL_STRIDE_NV12_PLANE0], DIGITAL_STRIDE_NV12_PLANE0);
                }

                for (j = 0; j < DIGITAL_HEIGHT_NV12_PLANE1; j++)
                {
                    unsigned int stride1 = buffers->buffers[bufferId].planes[1].stride;
                    unsigned int size0 = buffers->buffers[bufferId].planes[0].size;
                    memcpy((void* )&(buf+size0)[j*stride1], (void* )&(digital_matrix+DIGITAL_SIZE_NV12_PLANE0)[j*DIGITAL_STRIDE_NV12_PLANE1], DIGITAL_STRIDE_NV12_PLANE1);
                }

                buf += DIGITAL_STRIDE_NV12_PLANE0;
                break;
            }
            case QCARCAM_FMT_YU12:
            {
                digital_matrix = gDigitalMatrixYu12[digit];

                for (j = 0; j < DIGITAL_HEIGHT_YU12_PLANE0; j++)
                {
                    unsigned int stride0 = buffers->buffers[bufferId].planes[0].stride;
                    memcpy((void* )&buf[j * stride0], (void* )&digital_matrix[j * DIGITAL_STRIDE_YU12_PLANE0], DIGITAL_STRIDE_YU12_PLANE0);
                }

                for (j = 0; j < DIGITAL_HEIGHT_YU12_PLANE1; j++)
                {
                    unsigned int stride1 = buffers->buffers[bufferId].planes[1].stride;
                    unsigned int size0 = buffers->buffers[bufferId].planes[0].size;
                    memcpy((void* )&(buf + size0)[j * stride1], (void* )&(digital_matrix + DIGITAL_SIZE_YU12_PLANE0)[j * DIGITAL_STRIDE_NV12_PLANE1], DIGITAL_STRIDE_NV12_PLANE1);
                }

                for (j = 0; j < DIGITAL_HEIGHT_YU12_PLANE2; j++)
                {
                    unsigned int stride2 = buffers->buffers[bufferId].planes[2].stride;
                    unsigned int size0 = buffers->buffers[bufferId].planes[0].size;
                    unsigned int size1 = buffers->buffers[bufferId].planes[1].size;
                    memcpy((void* )&(buf + size0 + size1)[j*stride2], (void* )&(digital_matrix + DIGITAL_SIZE_YU12_PLANE0 + DIGITAL_SIZE_YU12_PLANE1)[j * DIGITAL_STRIDE_YU12_PLANE2], DIGITAL_STRIDE_YU12_PLANE2);
                }

                buf += DIGITAL_STRIDE_YU12_PLANE0;
                break;
            }
            case QCARCAM_FMT_RGB_888:
            {
                digital_matrix = gDigitalMatrixRgb888[digit];

                for (j = 0; j < DIGITAL_HEIGHT; j++)
                {
                    unsigned int stride = buffers->buffers[bufferId].planes[0].stride;
                    memcpy((void* )&buf[j*stride], (void* )&digital_matrix[j*DIGITAL_STRIDE_RGB888], DIGITAL_STRIDE_RGB888);
                }

                buf += DIGITAL_STRIDE_RGB888;
                break;
            }
            default:
                QCARCAM_ERRORMSG("unsupported format");
                break;
        }
    }
}

