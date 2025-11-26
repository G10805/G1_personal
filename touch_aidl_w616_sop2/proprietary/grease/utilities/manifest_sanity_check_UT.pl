#!/usr/bin/env bash
#
# manifest_sanity_check_UT.pl - Run unit tests for manifest_sanity_check.pl.
#
# usage: $0

# NOTE: This script creates a temporary dir
#       that it does not clean up.

# The following checks are performed.
# 1. Check mainlines for Android, Firefox OS, MDM and Tizen.
# 2. Check AU_LINUX_ANDROID_JB_REL_2.0.3.04.01.01.21.012
#    and expect for it to fail.
# 3. Check AU_LINUX_ANDROID_ICS.04.00.04.00.264
#    and expect for it to fail.

git_server_url='git://git.quicinc.com'
exit_code=0
sanity_check_script="$(dirname $(readlink -f $0))/manifest_sanity_check.pl"

sync_grease_config() {
    local gc_dir=$(dirname $1)
    mkdir -p $gc_dir
    pushd $gc_dir > /dev/null
    git init
    git fetch git://git.quicinc.com/grease/distribution refs/heads/master
    git checkout FETCH_HEAD
    popd > /dev/null
}

run_sanity_check() {
    local mf_project=$1
    local mf_rev=$2
    local expected_outcome=$3
    echo ------------------------------------------------------------
    echo Running manifest_sanity_check.pl for $mf_project $mf_rev
    echo ------------------------------------------------------------
    mkdir -p $work/$mf_project/$mf_rev
    pushd $work/$mf_project/$mf_rev > /dev/null
    git init
    git fetch $git_server_url/$mf_project $mf_rev
    git checkout FETCH_HEAD
    $sanity_check_script $grease_config ./default.xml
    if [ "$?" == "$expected_outcome" ]; then
        echo PASSED manifest_sanity_check.pl for $mf_project $mf_rev
    else
        echo FAILED manifest_sanity_check.pl for $mf_project $mf_rev
        exit_code=1
    fi
    popd > /dev/null
}


work=$(mktemp -d)
pushd $work > /dev/null
echo "working directory is $work"
grease_config=$work/grease/distribution/grease_config.xml

sync_grease_config $grease_config

# Check 1.
run_sanity_check platform/manifest refs/heads/jb 0
run_sanity_check b2g/manifest refs/heads/ics_strawberry 0
run_sanity_check mdm/manifest refs/heads/master 0
run_sanity_check mdm/manifest refs/heads/sawtooth 0

# Check 2.
run_sanity_check platform/manifest \
    refs/tags/AU_LINUX_ANDROID_JB_REL_2.0.3.04.01.01.21.012 1

# Check 3.
run_sanity_check platform/manifest \
    refs/tags/AU_LINUX_ANDROID_ICS.04.00.04.00.264 1

popd > /dev/null

exit $exit_code
