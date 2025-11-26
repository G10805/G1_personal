#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.

import argparse
import configdefine
import logging
import os
import processdefine
import sys
import time


# load the configuration module
def prepare_package(define):
    configdefine.check_def(define)
    processdefine.pack_process()


# Check the input
def check_input():
    argp = argparse.ArgumentParser()
    argp.add_argument(
        '-d',
        '--define',
        required=True,
        help='The definition file')
    return argp.parse_args()


# The load process
def procmain():
    load = time.time()
    logging_format = '%(asctime)s - %(filename)s - %(levelname)-8s: %(message)s'
    logging.basicConfig(level=logging.INFO, format=logging_format, datefmt='%Y-%m-%d %H:%M:%S')
    args = check_input()
    config = None

    if args.define:
        config = os.path.expanduser(args.define)

    if config:
        prepare_package(config)
    else:
        raise ValueError(
            'Please provide define option.')
    duration = time.time() - load
    h_result, more_result = divmod(duration, 3600)
    m_result, s_result = divmod(more_result, 60)
    logging.info('Rawdata Package generate time : %dh%dm%.3fs' % (h_result, m_result, s_result))


# Enter of the process
if __name__ == '__main__':
    procmain()
