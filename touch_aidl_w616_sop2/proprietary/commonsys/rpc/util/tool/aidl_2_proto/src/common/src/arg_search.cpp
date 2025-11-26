/*****************************************************************************
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*****************************************************************************/

#include "arg_search.h"

static uint32_t localArgc = 0;
static char **localArgv = NULL;

void ArgSearchInit(uint32_t argc, char *argv[])
{
    localArgc = argc;
    localArgv = argv;
}

bool ArgSearchValidate(void)
{
    bool param = false;
    uint32_t argi;
    for (argi = 1; argi < localArgc; ++argi)
    {
        if (strncmp("--", localArgv[argi], 2) == 0)
        {
            param = true;
        }
        else
        {
            if (param)
            {
                param = false;
            }
            else
            {
                return false;
            }
        }
    }
    return true;
}

char *ArgSearch(uint32_t *argi, const char *prefix, char **parameter, char **value)
{
    bool exact = false;
    uint32_t index = 1;
    size_t prefixLen = strlen(prefix);

    if (argi == NULL)
    {
        exact = true;
        argi = &index;
    }

    for ( ; *argi < localArgc; ++*argi)
    {
        if ((exact && (strcmp(prefix, localArgv[*argi]) == 0)) ||
            (!exact && (strncmp(prefix, localArgv[*argi], prefixLen) == 0)))
        {
            if (parameter != NULL)
            {
                *parameter = localArgv[*argi];
            }
            ++*argi;
            if ((*argi < localArgc) && (strncmp("--", localArgv[*argi], 2) != 0))
            {
                if (value != NULL)
                {
                    *value = localArgv[*argi];
                }
                ++*argi;
                return localArgv[*argi - 1];
            }
            if (value != NULL)
            {
                *value = NULL;
            }
            return localArgv[*argi - 1];
        }
    }
    return NULL;
}
