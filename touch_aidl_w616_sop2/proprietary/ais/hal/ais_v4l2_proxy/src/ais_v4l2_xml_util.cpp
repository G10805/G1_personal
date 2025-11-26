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


static unsigned int translate_to_v4l2_format(char* format)
{
    if (!strncmp(format, "V4L2_PIX_FMT_UYVY", strlen("V4L2_PIX_FMT_UYVY")))
    {
        return V4L2_PIX_FMT_UYVY; /* 16  YUV 4:2:2     */
    }
    else if (!strncmp(format, "V4L2_PIX_FMT_NV12", strlen("V4L2_PIX_FMT_NV12")))
    {
        return V4L2_PIX_FMT_NV12; /* 12  Y/CbCr 4:2:0  */
    }
    else if (!strncmp(format, "V4L2_PIX_FMT_YUV420", strlen("V4L2_PIX_FMT_YUV420")))
    {
        return V4L2_PIX_FMT_YUV420;
    }
    else
    {
        QCARCAM_ERRORMSG("XML color format %s not found default to V4L2_PIX_FMT_UYVY", format);
        return V4L2_PIX_FMT_UYVY;
    }
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
    else
    {
        QCARCAM_ERRORMSG("Sensor output format %s not found; defaulting to QCARCAM_OPMODE_RAW_DUMP", format);
        return QCARCAM_OPMODE_RAW_DUMP;
    }
}

static int parse_input_element (xmlDocPtr doc, xmlNodePtr parent, ais_proxy_xml_input_t* input)
{
    xmlNodePtr cur = parent->xmlChildrenNode;
    char format[64];
    char opmode[128];

    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"properties"))) {
            XML_GET_INT_ATTR(input->qcarcam_input_id, cur, "input_id", 0, qcarcam_input_desc_t, 0);
            XML_GET_STRING_ATTR(input->v4l2_node, cur, "node", 1, "/dev/video1");
            XML_GET_STRING_ATTR(opmode, cur, "opMode", 1, "RAW_DUMP");
            input->opmode = translate_operation_mode(opmode);
            XML_GET_INT_ATTR(input->recovery, cur, "recovery", 1, int, 0);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"output_setting")))
        {
            XML_GET_INT_ATTR(input->width, cur, "width", 1, int, -1);
            XML_GET_INT_ATTR(input->height, cur, "height", 1, int, -1);
            XML_GET_INT_ATTR(input->crop_top, cur, "crop_top", 1, unsigned int, 0);
            XML_GET_INT_ATTR(input->crop_left, cur, "crop_left", 1, unsigned int, 0);
            XML_GET_INT_ATTR(input->crop_right, cur, "crop_right", 1, unsigned int, 0);
            XML_GET_INT_ATTR(input->crop_bottom, cur, "crop_bottom", 1, unsigned int, 0);
            XML_GET_STRING_ATTR(format, cur, "pixelformat", 1, "V4L2_PIX_FMT_UYVY");
            input->pixformat = translate_to_v4l2_format(format);
        }
        cur = cur->next;
    }

    return 0;
}

int parse_xml_config_file(const char* filename, ais_proxy_xml_input_t* xml_inputs, unsigned int max_num_inputs)
{
    int rc = 0, numInputs = 0;

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
        }
        cur = cur->next;
    }

    xmlFreeDoc(doc);
    return numInputs;
}

