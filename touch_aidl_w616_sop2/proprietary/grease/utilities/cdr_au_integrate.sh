#!/bin/bash

#Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
#Qualcomm Technologies Proprietary and Confidential.

export PS4="+ [\t] "
set -o errexit -o pipefail

usage() {
cat <<USAGE

Usage: $(readlink -f $0)
    < -d|--sync_cdr_dir         workspace-dir >
    < -t|--sync_au_dir          synced-au-dir >
    < -r|--cdr_mainline_branch  cdr-mainline-branch>
    < -a|--au_name              au-tag >
    < -k|--log_dir              log-dir >
    [ -l|--cdr_server           cdr-server-url ]
    [ -u|--grease_username      grease-username ]
    [ -w|--grease_password      grease-password ]
    [ -b|--grease_branch        cdr-branch ]
    [ -v|--verbose              verbose-level ]
    [ -m|--manifest_file        manifest-file ]
    [ -p|--cdr_manifest_proj    cdr-manifest project ]
    [ -j|--num_jobs             num-jobs ]
    [ -e|--push_to_cdr_server]
    [ -g|--sync_quic_au]
    [ -c|--sync_cdr_tree]
    [ -y|--dry_run]
    [ -s|--skip_validation_errexit]

Syncs CDR tip and AU to be merged on top of it. Then
applies the delta beteen the two on top on the CDR
tip. In case of successfull merge each project in
CDR is tagged and pushed to CDR server. Also tag
is pushed to manifest project at the CDR server.

Required arguments:

-d|--sync_cdr_dir           Workspace location to sync CDR tree.
-t|--sync_au_dir            Directory to sync the AU to.
-a|--au_name                QuIC AU full name.
-r|--cdr_mainline_branch    Mainline branch of the CDR.
-k|--log_dir                Log directory.

Required arguments only when sync_quic_au enabled:

-u|--grease_username        Grease user id of the CDR customer.
-w|--grease_password        Grease access password for CDR customer.
-b|--grease_branch          Grease branch name for CDR customer.

Required arguments when push_to_cdr_server or sync_cdr_tree enabled:

-l|--cdr_server             CDR server name

Optional arguments:

-v|--verbose                Integer representing verbose level
                            (Use 2 for full trace).
-m|--manifest_file          Manifest file for the CDR tree.
-p|--cdr_manifest_proj      CDR manifest project name.
-j|--num_jobs               thread count for reposyncing the
                            CDR tree.
-e|--push_to_cdr_server     Push option will be enable.
-g|--sync_quic_au           Sync quic AU from CAF and grease sever.
-c|--sync_cdr_tree          Sync CDR tree.
-y|--dry_run                Dry run option will be enable.
-s|--skip_validation_errexit Skip exit on validation failure.
USAGE

[ $# -gt 0 ] && echo "Usage Error - $1"
exit

}

traperror () {
    local errcode=$?
    local lineno="$1"
    local funcstack="$2"
    local linecallfunc="$3"

    echo "ERROR: line ${lineno} - command exited with status: ${errcode}"
    if [[ "${funcstack}" != "" ]]; then
        echo -n "Error at function ${funcstack[0]}() "
        if [[ "${linecallfunc}" != "" ]]; then
            echo -n "called at line ${linecallfunc}"
        fi
        echo
    fi
}

function au_sync {
    trap 'traperror ${LINENO} ${FUNCNAME} ${BASH_LINENO}' ERR
    local workspace_dir="$1"

    pushd "${workspace_dir}"

    "${BASE_DIR}"/sync_all.sh \
        -u "${GREASE_USERNAME}" \
        -p "${GREASE_PASSWORD}" \
        -a "${AU_TAG}" \
        -b "${GREASE_BRANCH}"

    popd
    trap - ERR
}

function cdr_sync {
    trap 'traperror ${LINENO} ${FUNCNAME} ${BASH_LINENO}' ERR
    local workspace_dir="$1"
    local manifest_proj_url="$2"
    local branch="$3"
    local manifest_file="$4"

    pushd "${workspace_dir}"

    repo init \
        -u "${manifest_proj_url}" \
        -b "${branch}" \
        -m "${manifest_file}"
    repo sync \
        -j "${NUM_JOBS}"

    popd
    trap - ERR
}

function get_manifest_projects {
    trap 'traperror ${LINENO} ${FUNCNAME} ${BASH_LINENO}' ERR
    local manifest_file="$1"

    xmlstarlet sel -t -m "//project" -o " " -v \
        "@name" -o " " "${manifest_file}"
    trap - ERR
}

function get_projects {
    trap 'traperror ${LINENO} ${FUNCNAME} ${BASH_LINENO}' ERR
    local manifest="$1"
    local projects_list="$2"

    xmlstarlet sel -t -m \
        "//project[contains('${projects_list}' , concat(' ',@name,' '))]" \
        -v "@name" -n < "${manifest}"
    trap - ERR
}

function check_projects {
    local proj_list1="$1"
    local proj_list2="$2"
    local log_dir="$3"

    for i in ${proj_list1}; do
        j="\\b$i\\b"
        if ! [[  "${proj_list2}" =~ ${j} ]]; then
            absent_projects="${absent_projects}\n$i"
        fi
    done

    echo -e "${absent_projects}" > "$log_dir"/missing_projects.txt

    if [[ -z "${SKIP_VALIDATION_ERREXIT}" ]]; then
        if [[ -n "${absent_projects}" ]]; then
            echo -n "Following QuIC projects are not present in CDR."
            echo -e "${absent_projects}"
            echo "Follow the below steps before running the script again"
            echo "1. Create the absent projects."
            echo "2. Add the projects to the manifest project."
            echo "3. Run the script."
            exit 1
        fi
    fi
}

function validate_tagged_heads {
    local workspace_dir="$1"
    local untagged_head_prjs

    pushd "${workspace_dir}"

    untagged_head_prjs=$(repo forall -c "if ! git describe --tags \
        --exact-match >/dev/null; then echo \${REPO_PROJECT} ;  fi")
    if [[ -n "${untagged_head_prjs}" ]]; then
        echo "HEAD of following projects aren't tagged."
        echo "${untagged_head_prjs}"
        exit 1
    fi

    popd
}

function add_fetch_merge_remote {
    trap 'traperror ${LINENO} ${FUNCNAME} ${BASH_LINENO}' ERR
    local workspace_dir="$1"
    local projects="$2"
    local remote_path="$3"
    local remote_branch="$4"
    local log_dir="$5"

    pushd "${workspace_dir}"

    repo forall ${projects} -c "git remote add quic_au \
        ${remote_path}/\${REPO_PATH}"

    repo forall ${projects} -c 'git fetch quic_au'

    trap - ERR

    set +e
    repo forall ${projects} -p -c \
        "git merge remotes/quic_au/${remote_branch} \
        --no-ff" 2>&1 | tee merge.log
    return_status="$?"
    set -e

    log_merge_results \
        "${workspace_dir}" "${log_dir}"

    if [[ "${return_status}" -ne "0" ]]; then
        exit 0
    fi

    popd
}

function log_merge_results {
    local dir="$1"
    local log_dir="$2"

    export log_dir
    pushd "$dir"
    repo forall -c \
        'short=$(git status -s)
         if [[ -n "$short" ]] ; then
             echo $REPO_PROJECT >>"$log_dir"/conflicts.txt
             echo -e "PROJECT_NAME=$REPO_PROJECT"\\n"$short" >>"$log_dir"/short_status.txt
             echo -e "PROJECT_NAME=$REPO_PROJECT"\\n"$(git diff)" >>"$log_dir"/diff_status.txt
         else
             echo $REPO_PROJECT >>"$log_dir"/clean.txt
         fi
         echo -e "PROJECT_NAME=$REPO_PROJECT"\\n"$(git status)" >>"$log_dir"/status.txt'
    popd
}

function push_projects {
    trap 'traperror ${LINENO} ${FUNCNAME} ${BASH_LINENO}' ERR
    local workspace_dir="$1"
    local remote_url="$2"
    local remote_branch="$3"

    pushd "${workspace_dir}"

    repo forall -c "git push --dry-run \
        ${remote_url}/\${REPO_PROJECT} HEAD:${remote_branch}"

    if [[ -z "${DRY_RUN}" ]]; then
        repo forall -c "git push \
            ${remote_url}/\${REPO_PROJECT} HEAD:${remote_branch}"
    fi

    popd
    trap - ERR
}

function push_tags_to_projects {
    trap 'traperror ${LINENO} ${FUNCNAME} ${BASH_LINENO}' ERR
    local workspace_dir="$1"
    local remote_url="$2"
    local remote_branch="$3"

    pushd "${workspace_dir}"

    repo forall -c "git tag -a "${TAG_NAME}" -m "${TAG_NAME}" "

    repo forall -c "git push --dry-run --tags \
        ${remote_url}/\${REPO_PROJECT} HEAD:${remote_branch}"

    if [[ -z "${DRY_RUN}" ]]; then
        repo forall -c "git push --tags \
            ${remote_url}/\${REPO_PROJECT} HEAD:${remote_branch}"
    fi

    popd
    trap - ERR
}

function push_tags_to_manifest_project {
    trap 'traperror ${LINENO} ${FUNCNAME} ${BASH_LINENO}' ERR
    local manifest_prj_dir="$1"
    local remote_manifest_prj_url="$2"
    local remote_manifest_prj_branch="$3"

    pushd "${manifest_prj_dir}"

    git tag -a "${TAG_NAME}" -m "${TAG_NAME}"

    git push --dry-run --tags \
        "${remote_manifest_prj_url}" \
        HEAD:"${remote_manifest_prj_branch}"

    if [[ -z "${DRY_RUN}" ]]; then
        git push --tags \
            "${remote_manifest_prj_url}" \
            HEAD:"${remote_manifest_prj_branch}"
    fi

    popd
    trap - ERR
}

VERBOSE="0"

LONG_OPTS="au_name:,grease_branch:,sync_cdr_tree,sync_cdr_dir:, \
            push_to_cdr_server,sync_quic_au,num_jobs:,log_dir:, \
            cdr_server:,manifest_file:,cdr_manifest_proj:, \
            cdr_mainline_branch:,skip_validation_errexit, \
            sync_au_dir:,grease_username:,verbose:,grease_password:, \
            dry_run"

GETOPT_CMD=$(getopt -o a:b:cd:egj:k:l:m:p:r:st:u:v:w:y \
            --long "${LONG_OPTS}" -n $(basename "$0") -- "$@")

if [ $? != 0 ]; then
    echo "getopt failed"
fi

eval set -- "${GETOPT_CMD}"

while true ; do
    case "$1" in
    -a|--au_name)
        AU_TAG="$2"
        shift
        ;;
    -b|--grease_branch)
        GREASE_BRANCH="$2"
        shift
        ;;
    -c|--sync_cdr_tree)
        SYNC_CDR_TREE=1
        ;;
    -d|--sync_cdr_dir)
        CDR_WORKSPACE_DIR="$2"
        shift
        ;;
    -e|--push_to_cdr_server)
        PUSH_TO_CDR_SERVER=1
        ;;
    -g|--sync_quic_au)
        SYNC_QUIC_AU=1
        ;;
    -j|--num_jobs)
        NUM_JOBS="$2"
        shift
        ;;
    -k|--log_dir)
        LOG_DIR="$2"
        shift
        ;;
    -l|--cdr_server)
        CDR_SERVER="$2"
        shift
        ;;
    -m|--manifest_file)
        MANIFEST_FILE="$2"
        shift
        ;;
    -p|--cdr_manifest_proj)
        CDR_MANIFEST_PROJ="$2"
        shift
        ;;
    -r|--cdr_mainline_branch)
        CDR_MAINLINE_BRANCH="$2"
        shift
        ;;
    -s|--skip_validation_errexit)
        SKIP_VALIDATION_ERREXIT=1
        ;;
    -t|--sync_au_dir)
        SYNC_AU_DIR="$2"
        shift
        ;;
    -u|--grease_username)
        GREASE_USERNAME="$2"
        shift
        ;;
    -v|--verbose)
        VERBOSE="$2"
        shift
        ;;
    -w|--grease_password)
        GREASE_PASSWORD="$2"
        shift
        ;;
    -y|--dry_run)
        DRY_RUN=1
        ;;
    --) shift
        break
        ;;
    *) usage
    esac
    shift
done

[[ "${VERBOSE}" -ge 1 ]] && echo "Verbose enabled"
[[ "${VERBOSE}" -ge 2 ]] && set -o xtrace

[[ -z "${CDR_WORKSPACE_DIR}" ]]     && usage "Please supply CDR directory."
[[ -z "${SYNC_AU_DIR}" ]]           && usage "Please supply AU sync directory."
[[ -z "${AU_TAG}" ]]                && usage "Please supply AU name."
[[ -z "${CDR_MAINLINE_BRANCH}" ]]   && usage "Please supply mainline-branch."
[[ -z "${LOG_DIR}" ]]               && usage "Please supply log directory."

if [[ -n "${SYNC_QUIC_AU}" ]]; then
    [[ -z "${GREASE_USERNAME}" ]]       && usage "Please supply grease user id."
    [[ -z "${GREASE_PASSWORD}" ]]       && usage "Please supply grease password."
    [[ -z "${GREASE_BRANCH}" ]]         && usage "Please supply grease branch name."
fi

if [[ -n "${SYNC_CDR_TREE}" || "${PUSH_TO_CDR_SERVER}" ]]; then
    [[ -z "${CDR_SERVER}" ]] && usage "Please supply CDR server name."

    CDR_SERVER_URL="git://${CDR_SERVER}"
    [[ -z "${CDR_MANIFEST_PROJ}" ]] && \
        CDR_MANIFEST_PROJ="platform/manifest"
    CDR_MANIFEST_PROJ_URL="${CDR_SERVER_URL}/${CDR_MANIFEST_PROJ}"
fi

BASE_DIR=$(readlink -f "$(dirname "$0")")
CDR_WORKSPACE_DIR=$(readlink -e "${CDR_WORKSPACE_DIR}")
SYNC_AU_DIR=$(readlink -e "${SYNC_AU_DIR}")
LOG_DIR=$(readlink -e "${LOG_DIR}")

MANIFEST_DIR="${CDR_WORKSPACE_DIR}/.repo/manifests"

[[ -z "${MANIFEST_FILE}" ]] && MANIFEST_FILE="default.xml"
[[ -z "${NUM_JOBS}" ]] && NUM_JOBS="2"

PLATFORM="android"
if (echo "${AU_TAG}" | grep -q GECKO); then
    PLATFORM="b2g"
fi

if [[ -n "${SYNC_QUIC_AU}" ]]; then
    # Sync AU
    au_sync "${SYNC_AU_DIR}"
fi

# Start local branch on synced AU repository
pushd "${SYNC_AU_DIR}/${PLATFORM}"
# Delete old branches
set +e
repo start default --all
repo forall -c "git branch -D ${CDR_MAINLINE_BRANCH}"
set -e

repo start "${CDR_MAINLINE_BRANCH}" --all
popd

if [[ -n "${SYNC_CDR_TREE}" ]]; then
    # Sync CDR tip
    cdr_sync "${CDR_WORKSPACE_DIR}" "${CDR_MANIFEST_PROJ_URL}" \
        "${CDR_MAINLINE_BRANCH}" "${MANIFEST_FILE}"
fi

# Start local branch on synced CDR repository
pushd "${CDR_WORKSPACE_DIR}"

# Delete old branches
set +e
repo start default --all
repo forall -c "git branch -D quic_au_merge"
set -e

repo start quic_au_merge --all
popd

PROJECTS_LIST=$(get_manifest_projects \
    "${SYNC_AU_DIR}/${PLATFORM}/.repo/manifest.xml")

PROJECTS=$(get_projects \
    "${MANIFEST_DIR}/${MANIFEST_FILE}" "${PROJECTS_LIST}")

# Perform check to verify that all projects in QuIC
#+ are present in CDR

check_projects "${PROJECTS_LIST}" "${PROJECTS}" "${LOG_DIR}"

if [[ -z "${SKIP_VALIDATION_ERREXIT}" ]]; then
     validate_tagged_heads "${CDR_WORKSPACE_DIR}"
fi

add_fetch_merge_remote "${CDR_WORKSPACE_DIR}" "${PROJECTS}" \
    "${SYNC_AU_DIR}/${PLATFORM}" "${CDR_MAINLINE_BRANCH}" \
    "${LOG_DIR}"

if [[ -n "${PUSH_TO_CDR_SERVER}" ]]; then
    push_projects "${CDR_WORKSPACE_DIR}" "${CDR_SERVER_URL}" \
        "${CDR_MAINLINE_BRANCH}"

    AU_REV=$(echo "${AU_TAG}" | sed \
        's/^.*\([0-9]\{2\}\.[0-9]\{2\}\.[0-9]\{2\}\.[0-9]\{2,\}\.[0-9]\{3\}\)$/\1/')

    TAG_NAME="${AU_REV}_CDR000_000"

    push_tags_to_projects "${CDR_WORKSPACE_DIR}" \
        "${CDR_SERVER_URL}" "${CDR_MAINLINE_BRANCH}"

    push_tags_to_manifest_project "${MANIFEST_DIR}" \
        "${CDR_MANIFEST_PROJ_URL}" "${CDR_MAINLINE_BRANCH}"
fi
