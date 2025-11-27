/*
* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/

#include <math.h>
#include "zalloc.h"
#include "module_adr_algo_helper.h"

//sentences modified by algo, such as GGA----ACCURACY, PMTK840, need return to mnld
int algo_to_adr2mnld_msg(char *srcBuf, char *dstBuf)
{
    char *start = NULL;
    char *end = NULL;
    int len1, len2;

    if(!srcBuf){
        LOG_ERROR("src buffer is NULL");
        abort();
    }

    if(!dstBuf){
        LOG_ERROR("dst buffer is NULL");
        abort();
    }

    start = strstr(srcBuf, "GNACCURACY");
    if(!start){
        LOG_ERROR("not find GNACCURACY, the srcBuf :%s", srcBuf);
        abort();
    }
    end = strstr(start,"\n");
    if(!end){
        LOG_ERROR("find GNACCURACY end char failed, the srcBuf :%s", srcBuf);
        abort();
    }
    len1 = end - srcBuf + 1;
    strncpy(dstBuf, srcBuf, len1);

    start = strstr(srcBuf, "$PMTK840");
    if(!start){
        LOG_ERROR("not find $PMTK840, the srcBuf :%s", srcBuf);
        abort();
    }
    end = strstr(start,"\n");
    if(!end){
        LOG_ERROR("find $PMTK840 end char failed, the srcBuf :%s", srcBuf);
        abort();
    }
    len2 = end - start + 1;
    strncpy(dstBuf + len1, start, len2);

    return 0;
}

/*update PMTK840: position error parameter*/
int update_pmtk_position(char *srcBuf,char *outputbuf, double epx,
        double epy, double epv, double climb, double epd, double eps, double epc)
{
    char *pmtk_start = NULL;
    char *checksum_start = NULL;
    char buf[200] = {0};
    int len1 = 0;
    int len2 = 0;
    int len3 = 0;

    pmtk_start = strstr(srcBuf, "PMTK840");
    if (NULL == pmtk_start)
    {
        LOG_ERROR("Can't find PMTK840");
        return -1;
    }

    checksum_start = strstr(pmtk_start, "*");
    if (NULL == checksum_start)
    {
        LOG_ERROR("Can't find PMTK840");
        return -1;
    }

    sprintf(buf, "PMTK840,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f", epx,
        epy, epv, climb, epd, eps, epc);

    len1 = pmtk_start - srcBuf;
    strncpy(outputbuf, srcBuf, len1);
    len2 = strlen(buf);
    strncpy(outputbuf + len1, buf, len2);
    len3 = strlen(checksum_start);
    strncpy(outputbuf + len1 + len2, checksum_start, len3);
    *(outputbuf + len1 + len2 + len3) = '\0';

    return 1;
}

char *update_gsa_fix_status(char *srcBuf,char *outputbuf,char *GSA, char *last_end,int *len,char *fixbuf)
{
    int checklen;
    int newlen = *len;
    char *GSA_start = NULL;
    char *GSA_end = NULL;
    unsigned char crc;
    ProtoTokenizer  tzer[1];
    char checksum[3] = {0};
    LOG_INFO("update %s fix status", GSA);

    GSA_start = strstr(srcBuf, GSA);
    if (NULL == GSA_start)
    {
        LOG_ERROR("Can't find GPGSA");
        return last_end;
    }
    GSA_end = strstr(GSA_start, "\n");
    if (NULL == GSA_end)
    {
        LOG_ERROR("Can't find GPGSA");
        return last_end;
    }

    strncpy((char*)outputbuf + newlen, last_end, GSA_start - last_end);
    newlen = newlen + GSA_start - last_end;
    proto_tokenizer_init(tzer, GSA_start, GSA_end);
    Token tok_fix_type = proto_tokenizer_get(tzer, 2);

    int gsastartpos = newlen;
    strncpy((char*)outputbuf + newlen, GSA_start, tok_fix_type.p - GSA_start);
    newlen += tok_fix_type.p - GSA_start;
    strncpy((char*)outputbuf + newlen, fixbuf, 1);
    newlen = newlen + 1;
    strncpy((char*)outputbuf + newlen, tok_fix_type.end, GSA_end - tok_fix_type.end + 1);
    newlen += GSA_end - tok_fix_type.end + 1;
    checklen = GSA_end - GSA_start - 4;

    crc = sentence_checksum_calc(GSA_start, checklen);
    crc = sentence_checksum_calc((char*)outputbuf + gsastartpos, checklen);
    snprintf(checksum, 3, "%02X", crc);
    strncpy((char*)outputbuf + newlen - 4, checksum, 2);
    *len = newlen;

    return GSA_end;
}

int update_gga_rmc_position(char *srcBuf, char *dstBuf,
        uint64_t ts_sec, int ts_millisec, char *latlonbuf, int fixmode, char *speedbuf, float alt)
{
    char *GGA_start = NULL;
    char *GGA_end = NULL;
    char *RMC_start = NULL;
    char *RMC_end = NULL;
    ProtoTokenizer  tzer[1];
    char fixbuf[2] = {0};
    char *outputbuf = NULL;
    unsigned char crc;
    char checksum[3] = {0};
    int newlen = 0;
    int checklen = 0;
    char hhmmsssms[16] = {0};//064526.810
    char dmy[8] = {0};      //day,month,year:140917
    uint32_t year, month, day, hour, minute, second, millisec;
    if ((NULL == srcBuf) || (NULL == dstBuf))
    {
        return -1;
    }
    outputbuf = dstBuf;
    GGA_start = strstr(srcBuf,"GNGGA");
    if (NULL == GGA_start )
    {
        LOG_ERROR("Can't find GGA");
        return -1;
    }

    year = ((uint64_t)(ts_sec / pow(10, 10)))%10000;
    month = ((uint64_t)(ts_sec / pow(10, 8)))%100;
    day = ((uint64_t)(ts_sec / pow(10, 6)))%100;
    hour = ((uint64_t)(ts_sec / pow(10, 4)))%100;
    minute = ((uint64_t)(ts_sec / pow(10, 2)))%100;
    second = ts_sec%100;
    millisec = ts_millisec;
    //snprintf(dmy, 8, "%02d%02d%02d", day, month, (year - 2000));
    year = year%100;
    snprintf(dmy, 8, "%02d%02d%02d", day, month, year);
    snprintf(hhmmsssms, 16, "%02d%02d%02d.%03d", hour, minute, second, millisec);

    int ggastartpos = GGA_start - srcBuf ;
    GGA_end = strstr(GGA_start, "\n");
    proto_tokenizer_init(tzer, GGA_start, GGA_end);
    Token tok_date_hmsms = proto_tokenizer_get(tzer, 1);
    Token tok_latitude = proto_tokenizer_get(tzer, 2);
    Token tok_fixstate = proto_tokenizer_get(tzer, 6);
    Token tok_altitude = proto_tokenizer_get(tzer, 9);
    Token tok_height = proto_tokenizer_get(tzer, 11);

    char fix_quality = *tok_fixstate.p;

    unsigned int newlatlonlen = strlen(latlonbuf);
    unsigned int newspeedlen = strlen(speedbuf);

    newlen = tok_date_hmsms.p - srcBuf;
    strncpy((char*)outputbuf, srcBuf, newlen);

    strncpy((char*)outputbuf + newlen, hhmmsssms, strlen(hhmmsssms));
    newlen = newlen + strlen(hhmmsssms);
    strncpy((char*)outputbuf + newlen, tok_date_hmsms.end, (tok_latitude.p - tok_date_hmsms.end));
    newlen += (tok_latitude.p - tok_date_hmsms.end);

    strncpy((char*)outputbuf + newlen, latlonbuf, newlatlonlen);
    newlen = newlen + newlatlonlen;

    // Modify
    // if(fix_quality != '0')
        // snprintf(fixbuf, 2, "%1d", 1);
    // else
        // snprintf(fixbuf, 2, "%1d", 0);

    // if (fixmode ==2 || fixmode ==6)
        // snprintf(fixbuf, 2, "%1d", fixmode);

    if (fix_quality == '0' && (fixmode == 2 || fixmode == 6))
        fixbuf[0] = '6';
    else
        fixbuf[0] = fix_quality;

    strncpy((char*)outputbuf + newlen, fixbuf, 1);
    newlen = newlen + 1;
    strncpy((char*)outputbuf + newlen , tok_fixstate.end, tok_height.p - tok_fixstate.end);
    newlen += tok_height.p - tok_fixstate.end;

    int tmp_len = tok_altitude.end - tok_altitude.p + 1;
    char * tmp = zalloc(sizeof(char) * tmp_len);
    memset(tmp, '\0', tmp_len);
    strncpy(tmp, tok_altitude.p, tmp_len - 1);
    double double_diff = alt - atof(tmp);
    char str_diff[10];
    sprintf(str_diff, "%0.2f", double_diff);
    /*gcvt(double_diff, 32, str_diff);*/

    int str_diff_len = strlen(str_diff);

    strncpy((char*)outputbuf + newlen , str_diff, str_diff_len);
    newlen += str_diff_len;
    //len need to add last char
    strncpy((char*)outputbuf + newlen , tok_height.end, GGA_end - tok_height.end + 1);
    newlen += GGA_end - tok_height.end +1;

    /*checksum from $ end * */
    checklen = newlen - ggastartpos - 5;
    crc = sentence_checksum_calc(GGA_start, GGA_end - GGA_start - 4);
    crc = sentence_checksum_calc(outputbuf + ggastartpos, checklen);
    snprintf(checksum,3,"%02X", crc);
    strncpy((char *)outputbuf + newlen - 4, checksum , 2);
    free(tmp);
    tmp = NULL;

    //GNRMC
    RMC_start = strstr(srcBuf,"GNRMC");
    if (NULL == RMC_start )
    {
        LOG_ERROR("Can't find RMC");
        return -1;
    }

    RMC_end = strstr(RMC_start, "\n");
    if (NULL == RMC_end )
    {
        LOG_ERROR("Can't find RMC");
        return -1;
    }

    // ADD
    char fixRMCbuf[2] = {0};

    strncpy((char*)outputbuf + newlen, GGA_end + 1, RMC_start - GGA_end - 1);
    newlen = newlen + RMC_start - GGA_end - 1;
    proto_tokenizer_init(tzer, RMC_start, RMC_end);
    tok_date_hmsms = proto_tokenizer_get(tzer, 1);
    
    // ADD
    Token tok_status = proto_tokenizer_get(tzer, 2);
    char fix_rmc = *tok_status.p;

    tok_latitude = proto_tokenizer_get(tzer, 3);
    Token tok_bearing = proto_tokenizer_get(tzer, 8);
    Token tok_date_dmy = proto_tokenizer_get(tzer, 9);

    int rmcstartpos = newlen;
    strncpy((char*)outputbuf + newlen, RMC_start, tok_date_hmsms.p - RMC_start);
    newlen += tok_date_hmsms.p - RMC_start;
    checklen += tok_date_hmsms.p - RMC_start;
    strncpy((char*)outputbuf + newlen, hhmmsssms, strlen(hhmmsssms));
    newlen += strlen(hhmmsssms);

    // ADD
    if (fix_rmc == 'V' && (fixmode == 2 || fixmode == 6)) {
        fixRMCbuf[0] = 'A';
    } else {
        fixRMCbuf[0] = fix_rmc;
    }
    strncpy((char*)outputbuf + newlen, tok_date_hmsms.end, (tok_status.p - tok_date_hmsms.end));
    newlen += (tok_status.p - tok_date_hmsms.end);
    strncpy((char*)outputbuf + newlen, fixRMCbuf, 1);
    newlen = newlen + 1;
    strncpy((char*)outputbuf + newlen, tok_status.end, (tok_latitude.p - tok_status.end));
    newlen += (tok_latitude.p - tok_status.end);
    strncpy((char*)outputbuf + newlen, latlonbuf, newlatlonlen);
    newlen += newlatlonlen;
    strncpy((char*)outputbuf + newlen, speedbuf, newspeedlen);
    newlen += newspeedlen;
    strncpy((char*)outputbuf + newlen, tok_bearing.end, (tok_date_dmy.p - tok_bearing.end));
    newlen += (tok_date_dmy.p - tok_bearing.end);
    strncpy((char*)outputbuf + newlen, dmy, strlen(dmy));
    newlen += strlen(dmy);
    strncpy((char*)outputbuf + newlen, tok_date_dmy.end, RMC_end - tok_date_dmy.end + 1);
    newlen += RMC_end - tok_date_dmy.end + 1;

    /* *xx\r\n five chars */
    checklen = newlen - rmcstartpos - 5;
    crc = sentence_checksum_calc(RMC_start, RMC_end - RMC_start - 4);
    crc = sentence_checksum_calc((char*)outputbuf + rmcstartpos, checklen);
    LOG_DEBUG("RMC checklen:%d string:%s", checklen, (outputbuf + rmcstartpos));
    snprintf(checksum,3,"%02X", crc);
    strncpy((char*)outputbuf + newlen - 4, checksum, 2);

    strncpy((char*)outputbuf + newlen, (RMC_end + 1), strlen(RMC_end + 1));
    newlen += strlen(RMC_end + 1);
    *(outputbuf + newlen) = '\0';
    return 0;
}

