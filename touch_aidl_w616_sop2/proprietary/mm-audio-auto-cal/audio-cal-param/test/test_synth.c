/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <log/log.h>

#include "../inc/audcalparam_api.h"
#include "../src/audcalparam_api_priv.h"
#include "../src/audcalparam_serv_con.h"

/*******************************************************************************
 * Macro Definitions
 ******************************************************************************/
#define MAX_VAR_NUM 6
#define MAX_STEP_NUM 6
#define MAX_VOICE_NUM 256

enum
{
    AUDIO_DEVICE_CAL_TYPE = 0,
    AUDIO_STREAM_CAL_TYPE,
};

#define CAPI_V2_MODULE_ID_SYNTH 0x123A5000

#define CAPI_V2_PARAM_ID_SYNTH_ENABLE 0x123A5001
#define CAPI_V2_PARAM_ID_SYNTH_WAVE_TABLES 0x123A5002
#define CAPI_V2_PARAM_ID_SYNTH_ENVELOPE_TABLES 0x123A5003
#define CAPI_V2_PARAM_ID_SYNTH_VOICE_CONFIG 0x123A5004

#define CAPI_V2_PARAM_ID_SYNTH_INPUT_GAINS 0x123A5005
#define CAPI_V2_PARAM_ID_SYNTH_GAINS 0x123A5006

#define CAPI_V2_PARAM_ID_SYNTH_PATTERN 0x123A5008
#define CAPI_V2_PARAM_ID_SYNTH_VARIABLES 0x123A5009
#define CAPI_V2_PARAM_ID_SYNTH_TRIGGER 0x123A5010

/*******************************************************************************
 * Typedef
 ******************************************************************************/
/**
  @brief synth pp enable structure.
*/
struct synth_pp_enable
{
    uint32_t enable_flag; /*{"Disable"=0; "Enable"=1}*/
};
/* Type definition for #synth_pp_enable_t. */
typedef struct synth_pp_enable synth_pp_enable_t;

/**
  @brief synth pp trigger structure.
*/
struct synth_pp_trigger
{
    int32_t repeats; /** write number of desired repetitions, see SYNTH_REPEATS_* mentioned above*/
};
/* Type definition for #synth_pp_trigger_t. */
typedef struct synth_pp_trigger synth_pp_trigger_t;

/**
  @brief per per channel input gains - smoothly ramped
*/
struct synth_pp_input_gains
{
    uint16_t ramp_time_ms; /*ramping time in ms*/
    uint16_t num_gains;    /*number of gains, can be 1 or number of channels*/
    uint16_t *gains;       /*Linear gain values Q14, Single value for all channels
                       or N-Channel values*/
};
/* Type definition for #synth_pp_input_gains_t */
typedef struct synth_pp_input_gains synth_pp_input_gains_t;

/**
  @brief per per channel output gains - smoothly ramped
*/
struct synth_pp_output_gains
{
    uint16_t ramp_time_ms; /*ramping time in ms*/
    uint16_t num_gains;    /*number of gains, can be 1 or number of channels*/
    uint16_t *gains;       /*Linear gain values Q14, Single value for all channels
                       or N-Channel values*/
};
/* Type definition for #synth_pp_output_gains_t */
typedef struct synth_pp_output_gains synth_pp_output_gains_t;

/**
  @brief pattern step
  VARIABLE_IDs: all values of synth_pstep_t fields gain,freq or ticks
  can refer to an according variable value
  values are interpreted as VARIABLE_ID when (value AND 0xFFC0) == 0xFFC0
  the variable index is VARIABLE_ID AND 0x3F (0..64)
  A variable is evaluated whenever a pattern step is executed so that a tone
  pattern can be dynamically adjusted in its gains, frequencies and timings}
*/
struct synth_pp_pstep
{
    uint8_t op;     /* pattern operation
                   NONE=0;START=1;STOP=2;WAIT=3;SETSUSTAIN=4
                 */
    uint8_t index;  /* voice or variable index*/
    uint16_t gain;  /* step gain */
    uint16_t freq;  /*frequency, depending on fundamental_freq value :
                    fundamental_freq==0 -> absolute cent = log2(freq[Hz])*1200
                    fundamental_freq>0  -> multiple of fundamental_freq
                    see also ##VARIABLE_ID##} */
    uint16_t ticks; /*description  {number of ticks until next step,
                      result will be rounded to full samples} */
};
/* Type definition for #synth_pp_pstep_t */
typedef struct synth_pp_pstep synth_pp_pstep_t;

/**
  @brief synthersize tone pattern
*/
struct synth_pp_pattern
{
    uint32_t fundamental_freq; /*fundamental frequency as nano Hertz or 0 to use cent scale*/
    uint32_t tick_len_ns;      /*tick length in ns*/
    uint16_t sustain_enable;   /*enable tone sustaining
                             default value can be overwritten with OP set-sustain*/
    uint16_t reserved;
    uint32_t num_entries;    /*number of pattern steps*/
    synth_pp_pstep_t *steps; /*pattern steps arrary*/
};
/* Type definition for #synth_pp_pattern_t */
typedef struct synth_pp_pattern synth_pp_pattern_t;

/**
  @brief list of variables optionally referenced by pattern values
*/
struct synth_pp_variable
{
    uint32_t num_vars;   /*number of variables */
    uint16_t *variables; /* variables arrays*/
};
/* Type definition for #synth_pp_variable_t */
typedef struct synth_pp_variable synth_pp_variable_t;

/**
  @brief  parameters to open synth module and by default synth module
          will support cache mechanism.
*/
struct synth_pp_open_param
{
    bool cache;
    uint32_t sample_rate; /*sample rate for copp, it could be 8k/16k/48k */
    uint32_t module_id;   /**< Module ID */
    uint32_t instance_id; /*for multi instance, default 0 */
    uint32_t param_id;    /**< Parameter ID */
};
/* Type definition for #synth_pp_common_ctl_t */
typedef struct synth_pp_open_param synth_pp_open_param_t;

struct synth_beeper_pattern
{
    uint32_t fundamental_freq;
    uint32_t tick_len_ns;
    uint16_t sustain_enable;
    uint16_t reserved;
    uint32_t num_entries;
    synth_pp_pstep_t step[MAX_STEP_NUM];
};

/**
  @brief single voice definition
*/
struct synth_pp_voicedef
{
    int16_t wave_table_id; /*built in or custom wave id, built in waves has negative IDs
                        Sine=-1, Square=-2, Saw=-3*/
    int16_t envelope_id;   /*built in or custom envelope id, built in envelopes have negative
                        IDs, beeper=-1 */
};
/* Type definition for #synth_pp_voicedef_t */
typedef struct synth_pp_voicedef synth_pp_voicedef_t;

/**
  @brief define per voice envelope and waveform
*/
struct synth_pp_voiceconfig
{
    uint32_t num_voices;            /*number of voices */
    synth_pp_voicedef_t *voice_def; /*array of structures num_voices*/
};
/* Type definition for #synth_pp_voiceconfig_t */
typedef struct synth_pp_voiceconfig synth_pp_voiceconfig_t;

struct synth_beeper_voice
{
    uint32_t num_voices;
    synth_pp_voicedef_t voicedef[MAX_VOICE_NUM];
};

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
struct synth_beeper_voice voice_demo;
struct synth_beeper_pattern pattern_demo;
uint16_t vars[MAX_VAR_NUM];
audcalparam_session_t *synth_handle;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
static int synth_pp_set_voiceconfig(synth_pp_open_param_t *open_args,
                                    synth_pp_voiceconfig_t *param,
                                    size_t length);
static int synth_pp_set_repeats(synth_pp_open_param_t *open_args,
                                synth_pp_trigger_t *param,
                                size_t length);
static int synth_pp_set_enable(synth_pp_open_param_t *open_args,
                               synth_pp_enable_t *param,
                               size_t length);
static int synth_pp_set_variable(synth_pp_open_param_t *open_args,
                                 synth_pp_variable_t *param,
                                 size_t length);
static int synth_pp_set_pattern(synth_pp_open_param_t *open_args,
                                synth_pp_pattern_t *param,
                                size_t length);
static int synth_pp_set_input_gains(synth_pp_open_param_t *open_args,
                                    synth_pp_input_gains_t *param,
                                    size_t length);
static int synth_pp_set_output_gains(synth_pp_open_param_t *open_args,
                                     synth_pp_output_gains_t *param,
                                     size_t length);
static int test_pp_enable_sumx();

/*******************************************************************************
 * Main Function
 ******************************************************************************/
int main(int argc, char *argv[])
{
    int c, rc = 0;
    uint32_t repeats = 0;
    uint32_t repeats_set_flag = 0;
    uint32_t enable = 0;
    uint32_t enable_set_flag = 0;
    uint32_t pattern_set_flag = 0;
    uint32_t output_gain_value = 0;
    uint32_t output_gain_set_flag = 0;
    uint32_t input_gain_value = 0;
    uint32_t input_gain_set_flag = 0;
    uint32_t vars_set_flag = 0;
    uint32_t num_vars = 0;
    uint32_t voice_set_flag = 0;
    uint32_t i = 0, read_bytes;
    int option_index;
    char *fname = "/etc/audcalparam_commands.cfg";
    char *snd_card_name = "sa8155-adp-star-snd-card"; // snd card name!
    struct stat fstatus;

    synth_pp_open_param_t open_args = {
        .cache = 0,
    };

    struct option long_opts[] = {
        {"repeats", required_argument, 0, 0},
        {"enable", required_argument, 0, 0},
        {"pattern", required_argument, 0, 0},
        {"step", required_argument, 0, 0},
        {"output_gain", required_argument, 0, 0},
        {"input_gain", required_argument, 0, 0},
        {"vars", required_argument, 0, 0},
        {"voices", required_argument, 0, 0},
        {0, 0, 0, 0},
    };

    if (stat(fname, &fstatus) != 0)
    {
        ALOGE("Cfg file %s not found", fname);
        return 1;
    }

    if (audcalparam_session_init(&synth_handle, fname, snd_card_name))
    {
        ALOGE("session init failed!!!");
        goto exit;
    }

    ALOGD("Starting synth_test...");

    while ((c = getopt_long(argc, argv, "", long_opts, &option_index)) != -1)
    {
        switch (c)
        {
        case 0:
        {
            if (!strcmp("repeats", long_opts[option_index].name) && optarg)
            {
                repeats = strtol(optarg, NULL, 0);
                repeats_set_flag = 1;
                ALOGD("option \"%s\" set to %d",
                      long_opts[option_index].name, repeats);
            }
            else if (!strcmp("enable", long_opts[option_index].name) && optarg)
            {
                enable = strtol(optarg, NULL, 0);
                enable_set_flag = 1;
                ALOGD("option \"%s\" set to %d",
                      long_opts[option_index].name, enable);
            }
            else if (!strcmp("pattern", long_opts[option_index].name) && optarg)
            {
                sscanf(optarg, "%u,%u,%hu,%u", &pattern_demo.fundamental_freq,
                       &pattern_demo.tick_len_ns,
                       &pattern_demo.sustain_enable,
                       &pattern_demo.num_entries);
                pattern_set_flag = 1;
                ALOGD("option \"%s\" set to fundamental_freq %d tick_len_ns %d sustain_enable %d num_entries %d",
                      long_opts[option_index].name,
                      pattern_demo.fundamental_freq,
                      pattern_demo.tick_len_ns,
                      pattern_demo.sustain_enable,
                      pattern_demo.num_entries);
            }
            else if (!strcmp("vars", long_opts[option_index].name) && optarg)
            {
                i = 0;
                read_bytes = 0;
                while (1)
                {
                    rc = sscanf(optarg, "%hu %n", &vars[i], &read_bytes);
                    if (rc == 1)
                    {
                        ALOGD("option \"%s\" set to var[%d] = %d read_byte %u",
                              long_opts[option_index].name,
                              i,
                              vars[i],
                              read_bytes);
                        optarg += read_bytes + 1;
                        i++;
                        if (i == MAX_VAR_NUM)
                            break;
                    }
                    else
                    {
                        break;
                    }
                }
                num_vars = i;
                if (num_vars < MAX_VAR_NUM)
                    vars_set_flag = 1;
                else
                    ALOGD("option \"%s\" invalid parameters cnt, maximum support 6 variable\n",
                          long_opts[option_index].name);
            }
            else if (!strcmp("voices", long_opts[option_index].name) && optarg)
            {
                i = 0;
                read_bytes = 0;
                while (1)
                {
                    rc = sscanf(optarg, "%hu,%hu %n", &voice_demo.voicedef[i].wave_table_id,
                                &voice_demo.voicedef[i].envelope_id,
                                &read_bytes);
                    if (rc == 2)
                    {
                        ALOGD("option \"%s\" set to voice[%d] = waveID %d envelopID %d read_byte %u",
                              long_opts[option_index].name,
                              i,
                              voice_demo.voicedef[i].wave_table_id,
                              voice_demo.voicedef[i].envelope_id,
                              read_bytes);
                        optarg += read_bytes + 1;
                        i++;
                        if (i == MAX_VOICE_NUM)
                            break;
                    }
                    else
                    {
                        break;
                    }
                }
                voice_demo.num_voices = i;
                if (voice_demo.num_voices < MAX_VOICE_NUM)
                    voice_set_flag = 1;
                else
                    ALOGD("option \"%s\" invalid parameters cnt, maximum support 256 voices\n",
                          long_opts[option_index].name);
            }
            else if (!strcmp("step", long_opts[option_index].name) && optarg)
            {
                i = 0;
                read_bytes = 0;
                while (1)
                {
                    rc = sscanf(optarg, "%u,%u,%u,%u,%u %n",
                                (uint32_t *)&pattern_demo.step[i].op,
                                (uint32_t *)&pattern_demo.step[i].index,
                                (uint32_t *)&pattern_demo.step[i].gain,
                                (uint32_t *)&pattern_demo.step[i].freq,
                                (uint32_t *)&pattern_demo.step[i].ticks,
                                &read_bytes);
                    if (rc == 5)
                    {
                        ALOGD("option \"%s\" set to step %d op %u, index %u,gain %u, freq %u, tick%u read_bytes %u",
                              long_opts[option_index].name,
                              i,
                              pattern_demo.step[i].op,
                              pattern_demo.step[i].index,
                              pattern_demo.step[i].gain,
                              pattern_demo.step[i].freq,
                              pattern_demo.step[i].ticks,
                              read_bytes);
                        optarg += read_bytes + 1;
                        i++;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else if (!strcmp("output_gain", long_opts[option_index].name) && optarg)
            {
                output_gain_value = strtol(optarg, NULL, 0);
                output_gain_set_flag = 1;
                ALOGD("option \"%s\" set to %d",
                      long_opts[option_index].name, output_gain_value);
            }
            else if (!strcmp("input_gain", long_opts[option_index].name) && optarg)
            {
                input_gain_value = strtol(optarg, NULL, 0);
                input_gain_set_flag = 1;
                ALOGD("option \"%s\" set to %d",
                      long_opts[option_index].name, input_gain_value);
            }
            break;
        }
        default:
        {
            ALOGD("invalid command line argument");
            break;
        }
        }
    }

    if (enable_set_flag)
    {
        synth_pp_enable_t enable_param;
        enable_param.enable_flag = enable;
        rc = synth_pp_set_enable(&open_args, &enable_param, sizeof(enable_param));
        if (rc)
        {
            ALOGE("synth_test set enable failure");
            return -1;
        }
    }
    if (pattern_set_flag)
    {
        synth_pp_pattern_t pattern_param;
        pattern_param.fundamental_freq = pattern_demo.fundamental_freq;
        pattern_param.tick_len_ns = pattern_demo.tick_len_ns;
        pattern_param.sustain_enable = pattern_demo.sustain_enable;
        pattern_param.num_entries = pattern_demo.num_entries;
        pattern_param.steps = pattern_demo.step;
        rc = synth_pp_set_pattern(&open_args, &pattern_param, sizeof(pattern_param));
        if (rc)
        {
            ALOGE("synth_test set pattern failed");
            return -1;
        }
    }
    if (output_gain_set_flag)
    {
        uint16_t gains[8];
        gains[0] = output_gain_value;
        synth_pp_output_gains_t gain_param;
        gain_param.ramp_time_ms = 20;
        gain_param.num_gains = 1;
        gain_param.gains = gains;
        rc = synth_pp_set_output_gains(&open_args, &gain_param, sizeof(gain_param));
        if (rc)
        {
            ALOGE("synth_test set output gains failure");
            return -1;
        }
    }

    if (input_gain_set_flag)
    {
        uint16_t gains[8];
        gains[0] = input_gain_value;
        synth_pp_input_gains_t gain_param;
        gain_param.ramp_time_ms = 20;
        gain_param.num_gains = 1;
        gain_param.gains = gains;
        rc = synth_pp_set_input_gains(&open_args, &gain_param, sizeof(gain_param));
        if (rc)
        {
            ALOGE("synth_test set input gains failure");
            return -1;
        }
    }
    if (vars_set_flag)
    {
        synth_pp_variable_t var_param;
        var_param.num_vars = num_vars;
        var_param.variables = vars;
        rc = synth_pp_set_variable(&open_args, &var_param, sizeof(var_param));
        if (rc)
        {
            ALOGE("synth_test set variable failure");
            return -1;
        }
    }

    if (repeats_set_flag)
    {
        synth_pp_trigger_t trigger;
        trigger.repeats = repeats;
        rc = synth_pp_set_repeats(&open_args, &trigger, sizeof(trigger));
        if (rc)
        {
            ALOGE("synth_test set repeats failed");
            return -1;
        }
    }

    if (voice_set_flag)
    {
        synth_pp_voiceconfig_t voice_param;
        voice_param.num_voices = voice_demo.num_voices;
        voice_param.voice_def = voice_demo.voicedef;
        rc = synth_pp_set_voiceconfig(&open_args, &voice_param, sizeof(voice_param));
        if (rc)
        {
            ALOGE("synth_test set voice failed");
            return -1;
        }
    }
    //    rc = test_pp_enable_sumx();

exit:
    rc = audcalparam_session_deinit(synth_handle);
    ALOGD("synth_test Exit, de-init session ret %d", rc);
    return rc;
}

static int synth_pp_set_voiceconfig(synth_pp_open_param_t *open_args,
                                    synth_pp_voiceconfig_t *param,
                                    size_t length)
{
    int32_t rc = 0;
    void *cmd_buf = NULL;
    size_t total_size = 0;
    uint32_t *tmp_ptr = NULL;
    audcalparam_cmd_module_param_cfg_t mpcfg;

    if (length != sizeof(synth_pp_voiceconfig_t))
    {
        ALOGE("%s invalid parameter", __func__);
        return -1;
    }

    total_size = sizeof(param->num_voices) + param->num_voices * sizeof(synth_pp_voicedef_t);
    cmd_buf = malloc(total_size);
    if (!cmd_buf)
    {
        ALOGE("out of memory");
        return -1;
    }
    memset(cmd_buf, 0, total_size);

    mpcfg.module_id = CAPI_V2_MODULE_ID_SYNTH;
    mpcfg.param_id = CAPI_V2_PARAM_ID_SYNTH_VOICE_CONFIG;
    mpcfg.base_cfg.sampling_rate = open_args->sample_rate;
    mpcfg.base_cfg.cache = open_args->cache;
    mpcfg.instance_id = open_args->instance_id;

    tmp_ptr = (uint32_t *)cmd_buf;
    tmp_ptr[0] = param->num_voices;
    memcpy(&tmp_ptr[1], param->voice_def, param->num_voices * sizeof(synth_pp_voicedef_t));
    rc = audcalparam_cmd_module_param(synth_handle, "copp_synth_voice_config", AUDCALPARAM_SET, (uint8_t *)cmd_buf, &total_size, &mpcfg);
    ALOGD("AUDCALPARAM SET_CONFIG returned %d (0x%x)", rc, rc);

    free(cmd_buf);
    return rc;
}

static int synth_pp_set_repeats(synth_pp_open_param_t *open_args,
                                synth_pp_trigger_t *param,
                                size_t length)
{
    int32_t rc = 0;
    void *cmd_buf = NULL;
    size_t total_size = 0;
    audcalparam_cmd_module_param_cfg_t mpcfg;

    if (length != sizeof(synth_pp_trigger_t))
    {
        ALOGE("%s invalid parameter", __func__);
        return -1;
    }

    total_size = sizeof(synth_pp_trigger_t);
    cmd_buf = malloc(total_size);
    if (!cmd_buf)
    {
        ALOGE("out of memory");
        return -1;
    }

    memset(cmd_buf, 0, total_size);

    mpcfg.module_id = CAPI_V2_MODULE_ID_SYNTH;
    mpcfg.param_id = CAPI_V2_PARAM_ID_SYNTH_TRIGGER;
    mpcfg.base_cfg.sampling_rate = open_args->sample_rate;
    mpcfg.base_cfg.cache = open_args->cache;
    mpcfg.instance_id = open_args->instance_id;

    memcpy(cmd_buf, param, sizeof(synth_pp_trigger_t));
    rc = audcalparam_cmd_module_param(synth_handle, "copp_synth_trigger", AUDCALPARAM_SET, (uint8_t *)cmd_buf, &total_size, &mpcfg);
    ALOGD("AUDCALPARAM SET_CONFIG returned %d (0x%x)", rc, rc);

    free(cmd_buf);
    return rc;
}

static int synth_pp_set_enable(synth_pp_open_param_t *open_args,
                               synth_pp_enable_t *param,
                               size_t length)
{
    int32_t rc = 0;
    void *cmd_buf = NULL;
    uint32_t *tmp_ptr = NULL;
    size_t total_size = 0;
    audcalparam_cmd_module_param_cfg_t mpcfg;

    if (length != sizeof(synth_pp_enable_t))
    {
        ALOGE("%s invalid parameter", __func__);
        return -1;
    }

    total_size = sizeof(synth_pp_enable_t);
    cmd_buf = malloc(total_size);
    if (!cmd_buf)
    {
        ALOGE("out of memory");
        return -1;
    }

    memset(cmd_buf, 0, total_size);

    mpcfg.module_id = CAPI_V2_MODULE_ID_SYNTH;
    mpcfg.param_id = CAPI_V2_PARAM_ID_SYNTH_ENABLE;
    mpcfg.base_cfg.sampling_rate = open_args->sample_rate;
    mpcfg.base_cfg.cache = open_args->cache;
    mpcfg.instance_id = open_args->instance_id;

    tmp_ptr = (uint32_t *)cmd_buf;
    tmp_ptr[0] = param->enable_flag;
    rc = audcalparam_cmd_module_param(synth_handle, "copp_synth_enable", AUDCALPARAM_SET, (uint8_t *)cmd_buf, &total_size, &mpcfg);
    ALOGD("AUDCALPARAM synth_enable (%d) returned %d", param->enable_flag, rc);

    free(cmd_buf);
    return rc;
}

static int synth_pp_set_variable(synth_pp_open_param_t *open_args,
                                 synth_pp_variable_t *param,
                                 size_t length)
{
    int32_t rc = 0;
    void *cmd_buf = NULL;
    size_t total_size = 0;
    uint32_t *tmp_ptr = NULL;
    audcalparam_cmd_module_param_cfg_t mpcfg;

    if (length != sizeof(synth_pp_variable_t))
    {
        ALOGE("%s invalid parameter", __func__);
        return -1;
    }

    total_size = sizeof(param->num_vars) + param->num_vars * sizeof(uint16_t);
    cmd_buf = malloc(total_size);
    if (!cmd_buf)
    {
        ALOGE("out of memory");
        return -1;
    }

    memset(cmd_buf, 0, total_size);

    mpcfg.module_id = CAPI_V2_MODULE_ID_SYNTH;
    mpcfg.param_id = CAPI_V2_PARAM_ID_SYNTH_VARIABLES;
    mpcfg.base_cfg.sampling_rate = open_args->sample_rate;
    mpcfg.base_cfg.cache = open_args->cache;
    mpcfg.instance_id = open_args->instance_id;

    tmp_ptr = (uint32_t *)cmd_buf;
    tmp_ptr[0] = param->num_vars;
    memcpy(&tmp_ptr[1], param->variables, param->num_vars * sizeof(uint16_t));
    rc = audcalparam_cmd_module_param(synth_handle, "copp_synth_set_variable", AUDCALPARAM_SET, (uint8_t *)cmd_buf, &total_size, &mpcfg);
    ALOGD("AUDCALPARAM SET_CONFIG returned %d (0x%x)", rc, rc);

    free(cmd_buf);
    return rc;
}

static int synth_pp_set_pattern(synth_pp_open_param_t *open_args,
                                synth_pp_pattern_t *param,
                                size_t length)
{
    int32_t rc = 0;
    void *cmd_buf = NULL;
    size_t total_size = 0;
    size_t header_size = 0;
    audcalparam_cmd_module_param_cfg_t mpcfg;
    uint8_t *tmp_ptr = NULL;

    if (length != sizeof(synth_pp_pattern_t))
    {
        ALOGE("%s invalid parameter", __func__);
        return -1;
    }

    header_size = sizeof(param->fundamental_freq) + sizeof(param->tick_len_ns) + sizeof(param->sustain_enable) + sizeof(param->reserved) + sizeof(param->num_entries);

    total_size = header_size + param->num_entries * sizeof(synth_pp_pstep_t);
    cmd_buf = malloc(total_size);
    if (!cmd_buf)
    {
        ALOGE("out of memory");
        return -1;
    }

    memset(cmd_buf, 0, total_size);

    mpcfg.module_id = CAPI_V2_MODULE_ID_SYNTH;
    mpcfg.param_id = CAPI_V2_PARAM_ID_SYNTH_PATTERN;
    mpcfg.base_cfg.sampling_rate = open_args->sample_rate;
    mpcfg.base_cfg.cache = open_args->cache;
    mpcfg.instance_id = open_args->instance_id;

    tmp_ptr = (uint8_t *)cmd_buf;
    memcpy(tmp_ptr, (void *)param, header_size);
    tmp_ptr += header_size;
    memcpy(tmp_ptr, param->steps, param->num_entries * sizeof(synth_pp_pstep_t));
    rc = audcalparam_cmd_module_param(synth_handle, "copp_synth_set_pattern", AUDCALPARAM_SET, (uint8_t *)cmd_buf, &total_size, &mpcfg);
    ALOGD("AUDCALPARAM SET_CONFIG returned %d (0x%x)", rc, rc);

    free(cmd_buf);
    return rc;
}

static int synth_pp_set_input_gains(synth_pp_open_param_t *open_args,
                                    synth_pp_input_gains_t *param,
                                    size_t length)
{
    int32_t rc = 0;
    void *cmd_buf = NULL;
    size_t total_size = 0;
    size_t header_size = 0;
    audcalparam_cmd_module_param_cfg_t mpcfg;
    uint16_t *tmp_ptr = NULL;

    if (length != sizeof(synth_pp_input_gains_t))
    {
        ALOGE("%s invalid parameter", __func__);
        return -1;
    }

    header_size = sizeof(param->ramp_time_ms) + sizeof(param->num_gains);
    total_size = header_size + param->num_gains * sizeof(uint16_t);
    cmd_buf = malloc(total_size);
    if (!cmd_buf)
    {
        ALOGE("out of memory");
        return -1;
    }

    memset(cmd_buf, 0, total_size);

    mpcfg.module_id = CAPI_V2_MODULE_ID_SYNTH;
    mpcfg.param_id = CAPI_V2_PARAM_ID_SYNTH_INPUT_GAINS;
    mpcfg.base_cfg.sampling_rate = open_args->sample_rate;
    mpcfg.base_cfg.cache = open_args->cache;
    mpcfg.instance_id = open_args->instance_id;

    tmp_ptr = (uint16_t *)cmd_buf;
    tmp_ptr[0] = param->ramp_time_ms;
    tmp_ptr[1] = param->num_gains;
    memcpy(&tmp_ptr[2], param->gains, param->num_gains * sizeof(uint16_t));
    rc = audcalparam_cmd_module_param(synth_handle, "copp_synth_set_input_gain", AUDCALPARAM_SET, (uint8_t *)cmd_buf, &total_size, &mpcfg);
    ALOGD("AUDCALPARAM SET_CONFIG returned %d (0x%x)", rc, rc);

    free(cmd_buf);
    return rc;
}

static int synth_pp_set_output_gains(synth_pp_open_param_t *open_args,
                                     synth_pp_output_gains_t *param,
                                     size_t length)
{
    int32_t rc = 0;
    void *cmd_buf = NULL;
    size_t total_size = 0;
    size_t header_size = 0;
    audcalparam_cmd_module_param_cfg_t mpcfg;
    uint16_t *tmp_ptr = NULL;

    if (length != sizeof(synth_pp_output_gains_t))
    {
        ALOGE("%s invalid parameter", __func__);
        return -1;
    }

    header_size = sizeof(param->ramp_time_ms) + sizeof(param->num_gains);
    total_size = header_size + param->num_gains * sizeof(uint16_t);
    cmd_buf = malloc(total_size);
    if (!cmd_buf)
    {
        ALOGE("out of memory");
        return -1;
    }

    memset(cmd_buf, 0, total_size);

    mpcfg.module_id = CAPI_V2_MODULE_ID_SYNTH;
    mpcfg.param_id = CAPI_V2_PARAM_ID_SYNTH_GAINS;
    mpcfg.base_cfg.sampling_rate = open_args->sample_rate;
    mpcfg.base_cfg.cache = open_args->cache;
    mpcfg.instance_id = open_args->instance_id;

    tmp_ptr = (uint16_t *)cmd_buf;
    tmp_ptr[0] = param->ramp_time_ms;
    tmp_ptr[1] = param->num_gains;
    memcpy(&tmp_ptr[2], param->gains, param->num_gains * sizeof(uint16_t));
    rc = audcalparam_cmd_module_param(synth_handle, "copp_synth_set_output_gain", AUDCALPARAM_SET, (uint8_t *)cmd_buf, &total_size, &mpcfg);
    ALOGD("AUDCALPARAM SET_CONFIG returned %d (0x%x)", rc, rc);

    free(cmd_buf);
    return rc;
}

static int test_pp_enable_sumx()
{
    int32_t rc = 0;
    int32_t len = 64;
    uint8_t buf[64];
    uint8_t *pbuf = buf;
    audcalparam_cmd_module_param_cfg_t mpcfg;

    mpcfg.module_id = 0x123A0000;
    mpcfg.param_id = 0x123A0001;
    mpcfg.base_cfg.sampling_rate = 48000;
    mpcfg.base_cfg.cache = 0;
    mpcfg.instance_id = 0;

    *(uint32_t *)pbuf = 1;
    rc = audcalparam_cmd_module_param(synth_handle, "sumx_en_copp_auto_amp_general_playback", AUDCALPARAM_SET, pbuf, &len, &mpcfg);
    ALOGD("AUDCALPARAM SET_CONFIG returned %d (0x%x)", rc, rc);
    return rc;
}
