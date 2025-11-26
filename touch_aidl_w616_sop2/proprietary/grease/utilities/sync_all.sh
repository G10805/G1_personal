#!/bin/bash

# Assume user is in a) empty dir or b) dir with sub dirs "caf-manifest",
# "grease-manifest", and either "b2g" or "android"
# If (a), clone/repo init as necessary to get proper structure
# If (b), just fetch, checkout, combine, and repo sync

set -o errexit

usage() {
  echo "Usage: `readlink -f $0` <-u grease_username> <-p grease_password> <-a au|-m crm_label> <-b branch>"
  echo -n "Example 1: `readlink -f $0` -u bob -p bob_pass -a AU_LINUX_GECKO_ICS_CHOCOLATE.01.00.00.05.005 -b CDR004/ics_chocolate"
  echo " -j 4 -Q -C"
  echo -n "Example 2: `readlink -f $0` -u bob -p bob_pass -m LA.BR.1.2.3-07210-8x09.0 -b CDR005/LA.BR.1.2.3_rb1 -j 4 -Q -C"
  echo ""
  exit 1
}

quiet=""
sync_c=""
shallow_clone=false
grease_server="grease.qualcomm.com"
jobs="2"
while getopts s:u:p:a:b:c:g:l:r:n:m:j:CSQ o
do
  case "$o" in
  s) grease_server="$OPTARG";;
  u) grease_user="$OPTARG";;
  p) grease_pass="$OPTARG";;
  a) full_au="$OPTARG";;
  m) crm_label="$OPTARG";;
  b) branch="$OPTARG";;
  c) caf_manifest_repo="$OPTARG";;
  g) grease_manifest_repo="$OPTARG";;
  l) caf_server_url="$OPTARG";;
  r) repo_url="$OPTARG";;
  n) repo_branch="$OPTARG";;
  C) sync_c="-c";;
  S) shallow_clone=true;;
  Q) quiet="-q";;
  j) jobs="$OPTARG";;
  [?]) usage ;;
  esac
done


if [ -z "$grease_user" ]; then
  echo "Please supply your grease username."
  usage
fi

if [ -z "$grease_pass" ]; then
  echo "Please supply your grease password."
  usage
fi

if ! [ -n "$full_au" -o -n "$crm_label" ]; then
  echo "Please supply the full AU name or CRM label."
  usage
fi

if [ -z "$branch" ]; then
  echo "Please supply the branch name."
  usage
fi

if $shallow_clone; then
    if ! repo init --help | grep -q '\-\-depth'; then
        echo "Current version of repo doesn't support shallow clone."
        echo "Please upgrade repo to use this option"
        usage
    fi
fi

platform=android
if [ -z "$caf_manifest_repo" ]; then
  caf_manifest_repo=platform/manifest
fi

if [ -z "$grease_manifest_repo" ]; then
  grease_manifest_repo=platform/manifest
fi

if [ -z "$caf_server_url" ]; then
  caf_server_url="git://codeaurora.org"
fi

if [ -z "$repo_url" ]; then
  repo_url="git://codeaurora.org/tools/repo.git"
fi

if [ -z "$repo_branch" ]; then
  repo_branch="caf-stable"
fi


if [[ "$full_au" =~ "GECKO" ]] || [[ "$crm_label" =~ LF ]]; then
    platform=b2g
    grease_manifest_repo=b2g/manifest
    caf_manifest_repo=quic/b2g/manifest
fi

cdr_customer=$(dirname ${branch})
grease_fetch_url="ssh://${grease_user}@${grease_server}:29418/"
grease_mf_url="${grease_fetch_url}${grease_manifest_repo}"
caf_mf_url="$caf_server_url/$caf_manifest_repo"

if [ -n "$full_au" ]; then
    # Extract the pattern "dd.dd.dd.dd.ddd", where d is a digit, in the AU name
    au="$(echo $full_au | \
          sed 's/^.*\([[:digit:]]\{2\}\.[[:digit:]]\{2\}\.[[:digit:]]\{2\}\.[[:digit:]]\{2,\}\.[[:digit:]]\{3\}\).*/\1/')"

    # Grease manifests are named like: 04.00.03.00.061.xml
    grease_mf_name="${au}.xml"
    # CAF manifests are named like: caf_AU_LINUX_GECKO_ICS_CHOCOLATE.01.00.00.05.005.xml
    if [[ "$caf_server_url" =~ "codelinaro" ]];then
       caf_mf_name="${full_au}.xml"
    else
       caf_mf_name="caf_${full_au}.xml"
    fi
    tar_file_xz="prebuilt_${cdr_customer}_${full_au}.tar.xz"
    tar_file_gz="prebuilt_${cdr_customer}_${full_au}.tar.gz"
    tar_file_old="prebuilt_${au}.tar.gz"
elif [ -n "$crm_label" ]; then
    grease_mf_name="${crm_label}.xml"
    caf_mf_name="${crm_label}.xml"
    tar_file_xz="prebuilt_${cdr_customer}_${crm_label}.tar.xz"
    tar_file_gz="prebuilt_${cdr_customer}_${crm_label}.tar.gz"
    tar_file_old="$tar_file_gz"
fi

# Test Grease Gerrit server connection
set +e
ssh -p 29418 ${grease_user}@${grease_server}
if [ $? -ne 127 ]; then
  echo "Please check your grease user name and password for correctness."
  usage
fi
set -e

parent_branch=`echo ${branch} | cut -d '/' -f2`
# Obtain the binary components
tar_file_on_server_xz="https://${grease_server}/binaries/outgoing/${cdr_customer}/${parent_branch}/${tar_file_xz}"
tar_file_on_server_gz="https://${grease_server}/binaries/outgoing/${cdr_customer}/${parent_branch}/${tar_file_gz}"
tar_file_on_server_old_layout="https://${grease_server}/binaries/outgoing/${parent_branch}/${au}/${tar_file_old}"
( wget --continue --no-check-certificate --user=$grease_user \
       --password=$grease_pass $tar_file_on_server_xz ||
  wget --continue --no-check-certificate --user=$grease_user \
       --password=$grease_pass $tar_file_on_server_gz ||
  wget --continue --no-check-certificate --user=$grease_user \
       --password=$grease_pass $tar_file_on_server_old_layout )

# Get the caf manifest project
fetch_branch=release

mkdir -p caf-manifest
pushd caf-manifest
git init
git fetch $caf_mf_url refs/heads/$fetch_branch
git checkout -f FETCH_HEAD
caf_prjs="$(xmlstarlet sel -t -m "//project[not(contains(@groups, 'notdefault'))]" -v @name -o ' ' $caf_mf_name)"
popd

# Get the grease manifest project
mkdir -p grease-manifest
pushd grease-manifest
git init
git fetch $grease_mf_url refs/heads/$branch
git checkout -f FETCH_HEAD
temp="tmp_manifest.xml"
xmlstarlet ed -P -S -u "/manifest/remote/@fetch" -v "${grease_fetch_url}" \
    "$grease_mf_name" > "$temp" && mv "$temp" "$grease_mf_name"
grease_prjs="$(xmlstarlet sel -t -m "//project[not(contains(@groups, 'notdefault'))]" -v @name -o ' ' $grease_mf_name)"
popd

# Produce a combined CAF/Grease manifest file
# Use join_manifests.sh from the same location
#+ as this script.
SCRIPT_DIR=$(dirname $(readlink -f "$0"))
join_manifests_cmd="$SCRIPT_DIR/join_manifests.sh"
if [ ! -e "$join_manifests_cmd" ]; then
    join_manifests_cmd=grease-manifest/bin/join_manifests.sh
fi
"$join_manifests_cmd" caf-manifest/$caf_mf_name grease-manifest/$grease_mf_name > manifest.xml

# Initialize a combined tree
args=''
if $shallow_clone; then
    args+=' --depth=1'
fi
mkdir -p $platform
pushd $platform
repo init -u $grease_mf_url \
          -m $grease_mf_name \
          -b $branch \
          --repo-url=$repo_url \
          --repo-branch=$repo_branch \
          $args

# Replace the grease-only manifest with the generated combined manifest
rm -f .repo/manifest.xml
cp ../manifest.xml .repo/manifest.xml

# Sync from CAF and from Grease
# Use 'jobs' for syncing caf projects over network
# Sync -j2 for syncing grease projects over the network only
repo sync -n -j2 ${sync_c} ${quiet} $grease_prjs
repo sync -n -j$jobs ${sync_c} ${quiet} $caf_prjs
# Sync -j8 for local disk and detach the head (increase the -j if desired)
repo sync -d -l -j8 ${sync_c} ${quiet}

# Move old prebuilt dirs
prebuilt_dirs="vendor/qcom/proprietary/prebuilt_HY11 vendor/qcom/proprietary/prebuilt_grease"
datetime="$(date +%F_%H.%M.%S)"
for dir in $prebuilt_dirs; do
    if [ -d "$dir" ]; then
        mv $dir ${dir}.$datetime
    fi
done

# Extract the binary component archive
if [ -e "../$tar_file_xz" ]; then
    tar xf ../$tar_file_xz
elif [ -e "../$tar_file_gz" ]; then
    tar xf ../$tar_file_gz
else
    tar xf ../$tar_file_old
fi
