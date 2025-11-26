/* ===========================================================================
 * Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 =========================================================================== */
#include "ais_v4l2_proxy_util.h"

#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#define XML_GET_INT_ATTR(_var_, _node_, _attr_, _opt_, _type_, _default_) \
    do { \
        xmlChar* _p_ = xmlGetProp(_node_, (const xmlChar *)_attr_); \
        if (_p_) { \
            _var_ = (_type_)atoi((const char *)_p_); \
            xmlFree(_p_); \
        } else if (_opt_) {\
            _var_ = (_type_)_default_; \
        } else  { \
            VPROXY_ERROR("could not get attribute " _attr_); \
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
            VPROXY_ERROR("could not get attribute " _attr_); \
            return -1; \
        } \
    } while(0)


static unsigned int translate_to_qcarcam_format(char* format)
{
    struct qcarcam_format {
        const char* name;
        uint32 uiformat;
    };

    static qcarcam_format formats[] =
        {
            {"QCARCAM_FMT_MIPIRAW_8",  QCARCAM_FMT_MIPIRAW_8},
            {"QCARCAM_FMT_MIPIRAW_10",  QCARCAM_FMT_MIPIRAW_10},
            {"QCARCAM_FMT_MIPIRAW_12",  QCARCAM_FMT_MIPIRAW_12},
            {"QCARCAM_FMT_MIPIRAW_14",  QCARCAM_FMT_MIPIRAW_14},
            {"QCARCAM_FMT_MIPIRAW_16",  QCARCAM_FMT_MIPIRAW_16},
            {"QCARCAM_FMT_MIPIRAW_20",  QCARCAM_FMT_MIPIRAW_20},

            {"QCARCAM_FMT_PLAIN16_10",  QCARCAM_FMT_PLAIN16_10},
            {"QCARCAM_FMT_PLAIN16_12",  QCARCAM_FMT_PLAIN16_12},
            {"QCARCAM_FMT_PLAIN16_14",  QCARCAM_FMT_PLAIN16_14},
            {"QCARCAM_FMT_PLAIN16_16",  QCARCAM_FMT_PLAIN16_16},
            {"QCARCAM_FMT_PLAIN32_20",  QCARCAM_FMT_PLAIN32_20},

            {"QCARCAM_FMT_RGB_888",  QCARCAM_FMT_RGB_888},
            {"QCARCAM_FMT_UYVY_8",  QCARCAM_FMT_UYVY_8},
            {"QCARCAM_FMT_UYVY_10",  QCARCAM_FMT_UYVY_10},
            {"QCARCAM_FMT_UYVY_12",  QCARCAM_FMT_UYVY_12},
            {"QCARCAM_FMT_YUYV_8",  QCARCAM_FMT_YUYV_8},
            {"QCARCAM_FMT_YUYV_10",  QCARCAM_FMT_YUYV_10},
            {"QCARCAM_FMT_YUYV_12",  QCARCAM_FMT_YUYV_12},

            {"QCARCAM_FMT_NV12",  QCARCAM_FMT_NV12},
            {"QCARCAM_FMT_NV21",  QCARCAM_FMT_NV21},

            {"QCARCAM_FMT_MIPIUYVY_10",  QCARCAM_FMT_MIPIUYVY_10},

        };

    uint32 size = sizeof(formats)/sizeof(qcarcam_format);

    for (uint32 i = 0; i < size; ++i)
    {
        if (formats[i].name && !strncmp(format, formats[i].name, strlen(formats[i].name)))
        {
            VPROXY_INFO("config format %x", formats[i].uiformat);
            return formats[i].uiformat;
        }
    }

    return 0;
}

static unsigned int translate_to_v4l2_format(char* format)
{
    struct v4l2_format {
        const char* name;
        uint32 uiformat;
    };

    static v4l2_format formats[] =
        {
            {"V4L2_PIX_FMT_UYVY",  V4L2_PIX_FMT_UYVY},
            {"V4L2_PIX_FMT_NV12",  V4L2_PIX_FMT_NV12},
            {"V4L2_PIX_FMT_YUV420",  V4L2_PIX_FMT_YUV420},
            {"V4L2_PIX_FMT_RGB565",  V4L2_PIX_FMT_RGB565},
            {"V4L2_PIX_FMT_RGB24",  V4L2_PIX_FMT_RGB24},
            {"V4L2_PIX_FMT_BGR24",  V4L2_PIX_FMT_BGR24},
            {"V4L2_PIX_FMT_RGB32",  V4L2_PIX_FMT_RGB32},
            {"V4L2_PIX_FMT_BGR32",  V4L2_PIX_FMT_BGR32},
            {"V4L2_PIX_FMT_GREY", V4L2_PIX_FMT_GREY},
        };

    uint32 size = sizeof(formats)/sizeof(v4l2_format);

    for (uint32 i = 0; i < size; ++i)
    {
        if (formats[i].name && !strncmp(format, formats[i].name, strlen(formats[i].name)))
        {
            VPROXY_INFO("config format %x", formats[i].uiformat);
            return formats[i].uiformat;
        }
    }

    return 0;
}

static qcarcam_opmode_type translate_operation_mode(char* format)
{
    if (!strncmp(format, "RAW_DUMP", strlen("RAW_DUMP")))
    {
        return QCARCAM_OPMODE_RAW_DUMP;
    }
    else if (!strncmp(format, "SHDR", strlen("SHDR")))
    {
        return QCARCAM_OPMODE_SHDR;
    }
    else if (!strncmp(format, "INJECT", strlen("INJECT")))
    {
        return QCARCAM_OPMODE_INJECT;
    }
    else if (!strncmp(format, "PAIRED_INPUT", strlen("PAIRED_INPUT")))
    {
        return QCARCAM_OPMODE_PAIRED_INPUT;
    }
    else if (!strncmp(format, "TRANSFORMER", strlen("TRANSFORMER")))
    {
        return QCARCAM_OPMODE_TRANSFORMER;
    }
    else if (!strncmp(format, "FUSED_SHDR", strlen("FUSED_SHDR")))
    {
        return QCARCAM_OPMODE_FUSED_SHDR;
    }
    else
    {
        VPROXY_ERROR("Sensor output format %s not found; defaulting to QCARCAM_OPMODE_RAW_DUMP", format);
        return QCARCAM_OPMODE_RAW_DUMP;
    }
}

static int parse_isp_instance_element(xmlDocPtr doc, xmlNodePtr cur, qcarcam_isp_usecase_config_t* isp_config)
{
    CAM_UNUSED(doc);

    XML_GET_INT_ATTR(isp_config->id, cur, "id", 0, unsigned int, 0);
    XML_GET_INT_ATTR(isp_config->camera_id, cur, "camera_id", 0, unsigned int, 0);
    XML_GET_INT_ATTR(isp_config->use_case, cur, "use_case", 0, qcarcam_isp_usecase_t, 0);

    return 0;
}

static int parse_input_element (xmlDocPtr doc, xmlNodePtr parent, ais_proxy_xml_input_t* input)
{
    xmlNodePtr cur = parent->xmlChildrenNode;
    char format[64];
    char opmode[128];
    char align[32];

    input->pixformat = 0;
    input->width = -1;
    input->height = -1;
    input->is_buf_align = true;

    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"properties"))) {
            XML_GET_INT_ATTR(input->qcarcam_input_id, cur, "input_id", 0, qcarcam_input_desc_t, 0);
            XML_GET_STRING_ATTR(input->v4l2_node, cur, "node", 1, "/dev/video1");
            XML_GET_STRING_ATTR(opmode, cur, "opMode", 1, "RAW_DUMP");
            input->opmode = translate_operation_mode(opmode);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"output_setting")))
        {
            XML_GET_INT_ATTR(input->width, cur, "width", 1, int, -1);
            XML_GET_INT_ATTR(input->height, cur, "height", 1, int, -1);
            XML_GET_STRING_ATTR(format, cur, "pixelformat", 1, "UNKNOWN");
            input->pixformat = translate_to_qcarcam_format(format);

            XML_GET_STRING_ATTR(format, cur, "v4l2format", 1, "UNKNOWN");
            input->v4l2_format = translate_to_v4l2_format(format);

            XML_GET_STRING_ATTR(align, cur, "is_buf_align", 1, "true");
            if (!strncmp(align, "true", strlen("true")))
            {
                input->is_buf_align = true;
            }
            else
            {
                input->is_buf_align = false;
            }

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

static int parse_latency_mode(xmlDocPtr doc, xmlNodePtr parent, ais_proxy_gconfig *config)
{
    xmlNodePtr cur = parent->xmlChildrenNode;
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"properties"))) {
            XML_GET_INT_ATTR(config->latency_measurement_mode, cur, "latency_mode", 0, CameraLatencyMeasurementModeType, 0);
        }
        cur = cur->next;
    }

    return 0;
}

int parse_xml_config_file(const char *filename, ais_proxy_xml_input_t *xml_inputs, unsigned int max_num_inputs, ais_proxy_gconfig *config)
{
    int rc = 0, numInputs = 0;

    xmlDocPtr doc;
    xmlNodePtr cur;

    doc = xmlParseFile(filename);

    if (doc == NULL)
    {
        VPROXY_ERROR("Document not parsed successfully");
        return -1;
    }

    cur = xmlDocGetRootElement(doc);

    if (cur == NULL)
    {
        VPROXY_ERROR("Empty config file");
        xmlFreeDoc(doc);
        return -1;
    }

    if (xmlStrcmp(cur->name, (const xmlChar *) "qcarcam_inputs"))
    {
        VPROXY_ERROR("Wrong config file format, root node != qcarcam_inputs");
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
                VPROXY_ERROR("Missing parameter in config file");
            }
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *) "extra_mode")))
        {
            rc = parse_latency_mode(doc, cur, config);
            if (rc)
            {
                VPROXY_ERROR("Missing parameter in config file");
            }
        }
        cur = cur->next;
    }

    xmlFreeDoc(doc);
    return numInputs;
}

