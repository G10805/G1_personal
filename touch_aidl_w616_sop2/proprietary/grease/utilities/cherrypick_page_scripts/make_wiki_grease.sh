# ============================================================================
#
#    Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
#    Qualcomm Technologies Proprietary and Confidential.
#
# ============================================================================

#!/bin/bash
cd $1
AU_RE=$2
MAIL_ID=$3
PATH=$PATH:`pwd`/bin
export PATH
start=`date`
for dir in `ls branches`;
do
    here=`pwd`
    branch_name=`echo $dir | tr [a-z] [A-Z]a`
    echo Checking $branch_name
    (cd branches/$dir;
        if [ ! -f in_progress ];
        then
            touch in_progress
            echo `date`:$branch_name syncing
#            repo sync --jobs=4 -q
            echo `date`:$branch_name wiki creation
            PYTHONPATH=$here python $here/create_wiki.py $branch_name $AU_RE
            echo `date`:$branch_name wiki creation done
            rm -f in_progress
        else
            echo `date`:$branch_name build still in progress
            ls -l in_progress
        fi
    ) > $1/tmp/$branch_name 2>&1 # Don't run in parallel &
done
# wait # uncomment to run in parallel
(
echo Started at $start
for dir in `ls branches`;
do
    branch_name=`echo $dir | tr [a-z] [A-Z]a`
    echo "=== $branch_name ==="
    cat $1/tmp/$branch_name
    rm -f $1/tmp/$branch_name
done
echo Ended at `date`
#) | mail -s 'Cherry pick page generation output'  $MAIL_ID
) >> CPwiki.log
