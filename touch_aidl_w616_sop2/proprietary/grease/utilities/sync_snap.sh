#!/bin/bash
#Copyright (c) 2020 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.

#Possible location of sourcecode/prebuilts
#Code/prebuilt bins	Private CAF	Public CAF	Grease	Chip code
#QSSI SI (OSS)		Yes		Yes		NA	NA
#QSSSI (Prop)		NA		NA		Yes	Yes
#Vendor SI (OSS)	Yes		Yes		NA	NA
#Vendor SI(Prop)	NA		NA		Yes	Yes

#QSSI Tree
#No need to sync vendor sources/prebuilts
#QSSI OSS - can be private/public CAF
#QSSI prop(sources and prebuilts) - Grease/Chpcode

#Vendor Tree
#Vendor SI OSS - can be private CAF/public CAF
#Vendor SI prop(Sources/Prebuilts) - Grease/Chipcode
#QSSI OSS - can be private CAF/Public CAF
#QSSI prop(sources) - Grease/Chipcode
#QSSI prop(prebuilts) - NA

#Vendor standalone Tree
#Vendor SI OSS - can be private CAF/public CAF
#Vendor SI prop(Sources/Prebuilts) - Grease/Chipcode

#Single Tree
#Vendor SI OSS - can be private CAF/public CAF
#Vendor SI prop(Sources/Prebuilts) - Grease/Chipcode
#QSSI OSS - can be private CAF/Public CAF
#QSSI prop(sources and prebuilts)- Grease/Chipcode

#Manifest Management
#QSSI Tree - single manaifest - no need of any local manifest(init CAF manifest and Grease manifest in seperate directories)
#Vendor Tree - init Vendor and QSSI SI manifests(Vendor CAF, Vendor Grease, QSSI CAF and QSSI Grease) - ignore grease manifest incase of Chipcode
#Single Tree - init Vendor and QSSI SI manifests(Vendor CAF, Vendor Grease, QSSI CAF and QSSI Grease) - ignore grease manifest incase of Chipcode

#Unsupported combo(expectation is both Vendor SI and QSSI SI prop content is sourced from same(Either grease or Chipcode)
#Vendor prop from Chipcode +  QSSI prop from Grease
#QSSI prop from Chipcode   +  Vendor prop from Grease

set -o errexit  -o pipefail

m1=$1 ;


# Gets a value for a default name and inserts where that name is not
# currently defined.
remove_default() { # name < xml > xml
  local val name=$1 xml=$(cat)

  val=$(echo "$xml" | xmlstarlet sel -t -v "/manifest/default/@$name")
  if [ -n "$val" ]; then
    echo "$xml" | xmlstarlet ed -i "/manifest/*[not(self::remote|@$name)]"\
                                   -t attr -n "$name" -v "$val"
  fi
}

# Gets each default and inserts it where none is currently defined.
# Deletes all the defaults.
remove_defaults() { # < xml > xml
  local xml=$(cat)

  xml=$(echo "$xml" | remove_default "remote")
  xml=$(echo "$xml" | remove_default "revision")
  # xml=$(echo "$xml" | remove_default "sync-j") # doesn't like the '-'

  echo "$xml" | xmlstarlet ed -d "/manifest/default"
}

# Uses the original remote name to lookup and then replaces it with the
# conflict free remote name.
#   You can't insert if it already exists, update instead.
#   Ensure that we update all node types, not just projects.
update_remote() { # orig new < xml > xml
  local orig new

  orig=$1
  new=$2
  xmlstarlet ed -u "/manifest/remote[@name='$orig']/@name" -v "$new"\
                -u "/manifest/*[@remote='$orig']/@remote" -v "$new"
}



usage() {
  echo ""
  echo "USAGE:"
  echo "TBD"
  echo ""
  exit 1
}

qssi_caf_args_check () {

  if [ "$tree_type" == $VENDOR_TREE_STANDALONE ]; then
    return 0
  fi

  echo "In QSSI CAF param check"

  if [ -z "$qssi_caf_manifest_repo" ]; then
    echo "Pls supply QSSI CAF manifest Repo info"
    usage
  fi

  if [ -z "$qssi_caf_server_url" ]; then
    echo "Pls supply QSSI CAF server url"
    usage
  fi

 return 0
}

vendor_caf_args_check () {

  echo "In Vendor CAF param check"

  if [ -z "$vendor_caf_manifest_repo" ]; then
    echo "Pls supply Vendor CAF manifest Repo info"
    usage
  fi

  if [ -z "$vendor_caf_server_url" ]; then
    echo "Pls supply Vendor CAF server url"
    usage
  fi

  if [ "$tree_type" == $VENDOR_TREE_STANDALONE ]; then
    return 0
  fi

  if [ -z "$qssi_caf_manifest_repo" ]; then
    echo "Pls supply QSSI CAF manifest Repo info"
    usage
  fi

  if [ -z "$qssi_caf_server_url" ]; then
    echo "Pls supply QSSI CAF server url"
    usage
  fi

  return 0
}

qssi_chipcode_args_check() {

  if [ "$tree_type" == $VENDOR_TREE_STANDALONE ]; then
    return 0
  fi

  echo "In QSSI Chipcode param check"

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     echo "Proprietary content will be downloaded/copied from 'Chipcode'"

     if [ -z "$qssi_chipcode_path" ]; then
        echo "Please supply the QSSI SI chipcode path"
        usage
     fi
     echo $qssi_chipcode_path
  fi

  return 0
}

vendor_chipcode_args_check() {
  echo "In Vendor Chipcode param check"

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     echo "Proprietary content will be downloaded/copied from 'Chipcode'"

     if [ -z "$vendor_chipcode_path" ]; then
        echo "Please supply the Vendor SI chipcode content path."
        usage
     fi

    if [ "$tree_type" == $VENDOR_TREE_STANDALONE ]; then
      return 0
    fi

     echo $vendor_chipcode_path
     if [ -z "$qssi_chipcode_path" ]; then
        echo "Please supply the QSSI SI chipcode content path"
        usage
     fi

     echo $qssi_chipcode_path

  fi

  return 0
}

qssi_grease_args_check() {

  if [ "$tree_type" == $VENDOR_TREE_STANDALONE ]; then
    return 0
  fi

  echo "In QSSI Grease param check"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     echo "Proprietary content will be downloaded from 'Grease'"

     if [ -z "$qssi_grease_user" ]; then
        qssi_grease_user=$vendor_grease_user
     fi

     if [ -z "$qssi_grease_pass" ]; then
        qssi_grease_pass=$vendor_grease_pass
     fi

     if [ "$tree_type" == $QSSI_TREE ]; then
       if ! [ -n "$qssi_au_tag" -o -n "$qssi_crm_label" ]; then
           echo "\nPlease supply the full AU TAG name or CRM label.\n"
           usage
       fi
     fi

     if [ -z "$qssi_branch" ]; then
        qssi_branch=$vendor_branch
     fi

     if [ -z "$qssi_grease_server" ]; then
      qssi_grease_server=$vendor_grease_server
     fi
  fi

  return 0
}

vendor_grease_args_check() {

  echo "In Vendor Grease param check"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     echo "Proprietary content will be downloaded from 'Grease'"

     if [ -z "$vendor_grease_user" ]; then
        echo "Please supply your grease username."
        usage
     fi

     if [ -z "$vendor_grease_pass" ]; then
        echo "Please supply your grease password."
        usage
     fi

     if ! [ -n "$vendor_au_tag" -o -n "$vendor_crm_label" ]; then
         echo "Please supply the full AU TAG name or CRM label."
         usage
     fi

     if [ -z "$vendor_branch" ]; then
        echo "Please supply the Vendor SI branch name."
        usage
     fi

     if [ -z "$vendor_branch" ]; then
        echo "Please supply the vendor SI branch name."
        usage
     fi
  fi
  return 0
}

#when groups are passed remove projects fetched
#from chipcode which do not belong to correct grp
remove_common_sys_prjs() {
  echo "Removing common sys projects..."
  file_name="common_sys_projects.txt"
  if [ ! -e "${qssi_chipcode_path}/${file_name}" ]; then
     echo "${qssi_chipcode_path}/${file_name} not found!"
     exit 1
  fi
  cp -f ${qssi_chipcode_path}/${file_name} $ROOT_PWD/
  for path in $(cat ${ROOT_PWD}/${file_name}); do
     echo "Deleting ${ROOT_PWD}/$path "
     rm -rf ${ROOT_PWD}/$path
  done
  rm -rf ${ROOT_PWD}/${file_name}
}


COMBINED_MANIFEST=".repo/manifests/snap_combined_manifest.xml"
PREBUILT_HY11_PATH="vendor/qcom/proprietary/prebuilt_HY11"
PREBUILT_GREASE_PATH="vendor/qcom/proprietary/prebuilt_grease"
REPO_MANIFESTS=".repo/manifests/"
REPO_MANIFESTS_GIT=".repo/manifests.git"
PROP_PATH="vendor/qcom/proprietary/"
REPO_BACKUP_PATH=".repo_bak"
VENDOR_CAF_DIR_NAME="vendor_caf"
VENDOR_PROP_DIR_NAME="vendor_prop"
QSSI_CAF_DIR_NAME="qssi_caf"
QSSI_PROP_DIR_NAME="qssi_prop"
QSSI_TREE="qt"
SINGLE_TREE="st"
SINGLE_TREE_GROUPS="sg"
VENDOR_TREE_STANDALONE="vt"
PROP_DEST_GREASE="gr"
PROP_DEST_CHIPCODE="ch"
quiet=""
sync_c=""
shallow_clone=false
groups=""
while getopts D:T:R:C:L:G:U:P:A:M:B:H:c:l:g:u:p:a:m:b:h:j:r:n:s:YSQ o
do
  case "$o" in
  D) tree_dest_path="$OPTARG";;
  T) tree_type="$OPTARG";;
  R) prop_dest="$OPTARG";;
  C) vendor_caf_manifest_repo="$OPTARG";;
  L) vendor_caf_server_url="$OPTARG";;
  G) vendor_grease_server="$OPTARG";;
  U) vendor_grease_user="$OPTARG";;
  P) vendor_grease_pass="$OPTARG";;
  A) vendor_au_tag="$OPTARG";;
  M) vendor_crm_label="$OPTARG";;
  B) vendor_branch="$OPTARG";;
  H) vendor_chipcode_path="$OPTARG";;
  c) qssi_caf_manifest_repo="$OPTARG";;
  l) qssi_caf_server_url="$OPTARG";;
  g) qssi_grease_server="$OPTARG";;
  u) qssi_grease_user="$OPTARG";;
  p) qssi_grease_pass="$OPTARG";;
  a) qssi_au_tag="$OPTARG";;
  m) qssi_crm_label="$OPTARG";;
  b) qssi_branch="$OPTARG";;
  h) qssi_chipcode_path="$OPTARG";;
  r) repo_url="$OPTARG";;
  n) repo_branch="$OPTARG";;
  s) groups="$OPTARG";;
  Y) sync_c="-c";;
  S) shallow_clone=true;;
  Q) quiet="-q";;
  j) jobs="$OPTARG";;
  [?]) usage ;;
  esac
done


if [ -z "$tree_type" ]; then
  if [ -z "$prop_dest" ]; then
    if [ -z "$tree_dest_path" ]; then
      echo "Falling back to legacy sync model"
      if [ -z "$vendor_caf_manifest_repo" ]; then
        echo "legacy sync: public CAF"
        $PWD/sync_all.sh -a $vendor_au_tag -b $vendor_branch -u $vendor_grease_user -p $vendor_grease_pass -s $vendor_grease_server
      else
        echo "legacy sync: private CAF"
        $PWD/sync_all.sh -a $vendor_au_tag -b $vendor_branch -u $vendor_grease_user -p $vendor_grease_pass -s $vendor_grease_server -c $vendor_caf_manifest_repo -l $vendor_caf_server_url
      fi
      exit 0
    fi
  fi
fi

echo "using AIM sync model"

if [ -z "$tree_type" ]; then
  echo "Please input tree type(vendor Tree(vt)/QSSI Tree(qt)/Single Tree(st))"
  usage
fi

if [ -z "$prop_dest" ]; then
  echo "Please input source code/prebuilt location(Grease(gr)/Chipcode(ch))"
  usage
fi

#Pick up the arguments based on Tree type
#check, if req is to sync QSSI Tree
if [ "$tree_type" == $QSSI_TREE ]; then
  echo "Selected tree type is 'QSSI Tree'"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     if qssi_grease_args_check
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     if qssi_chipcode_args_check
        then echo "success"
        else echo " failure"
     fi
  fi

  if qssi_caf_args_check
     then echo "success"
     else echo " failure"
  fi

fi

#check, if the req is to sync Vendor Tree
if [ "$tree_type" == $SINGLE_TREE ] || [ "$tree_type" == $SINGLE_TREE_GROUPS ] || [ "$tree_type" == $VENDOR_TREE_STANDALONE ]
then
  echo "Selected tree type is 'SINGLE_TREE/SINGLE_TREE_GROUPS/VENDOR_STANDALONE'"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     if qssi_grease_args_check
        then
        if vendor_grease_args_check
          then
          echo "QSSI and Vendor Grease args check success"
          else echo "Vendor Grease args check failure"
        fi
        else echo "QSSI Grease args check failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     if qssi_chipcode_args_check
        then
        if vendor_chipcode_args_check
          then
          echo "QSSI and Vendor Grease args check success"
          else echo "Vendor Grease args check failure"
        fi
        else echo "QSSI Grease args check failure"
     fi
  fi

  if qssi_caf_args_check
     then
     if vendor_caf_args_check
       then
       echo "QSSI and Vendor CAF args check success"
       else echo "Vendor CAF args check failure"
     fi
     else echo "QSSI CAF args check failure"
  fi
fi

if $shallow_clone; then
    if ! repo init --help | grep -q '\-\-depth'; then
        echo "Current version of repo doesn't support shallow clone."
        echo "Please upgrade repo to use this option"
        usage
    fi
fi

platform=android
if [ -z "$vendor_caf_manifest_repo" ]; then
  vendor_caf_manifest_repo=la/vendor/manifest
fi

if [ -z "$qssi_caf_manifest_repo" ]; then
  qssi_caf_manifest_repo=la/system/manifest
fi

if [ -z "$vendor_grease_manifest_repo" ]; then
  vendor_grease_manifest_repo=la/vendor/manifest
fi

if [ -z "$qssi_grease_manifest_repo" ]; then
  qssi_grease_manifest_repo=la/system/manifest
fi

if [ -z "$vendor_caf_server_url" ]; then
  vendor_caf_server_url="https://git.codelinaro.org"
fi

if [ -z "$qssi_caf_server_url" ]; then
  qssi_caf_server_url="https://git.codelinaro.org"
fi

if [ -z "$repo_url" ]; then
  repo_url="https://git.codelinaro.org/clo/la/tools/repo.git"
fi

if [ -z "$repo_branch" ]; then
  repo_branch="clo-stable"
fi

#clean up the repo folders
if [ -z "$tree_dest_path" ]; then
  echo "Please supply the workspace PATH"
      exit 1
fi

echo "Current directory $PWD"
echo "Dest path $tree_dest_path"
if [ ! -d "$tree_dest_path" ]
then
    echo "Directory $tree_dest_path DOES NOT exists,creating the destination path"
    mkdir -p $tree_dest_path
fi

ROOT_PWD=$tree_dest_path

if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
  if [ "$tree_type" == $QSSI_TREE ] || [ "$tree_type" == $SINGLE_TREE ] || [ "$tree_type" == $SINGLE_TREE_GROUPS ]
    then
    if [ ! -d "$qssi_chipcode_path" ]
    then
      echo "QSSI SI chipcode PATH qssi_chipcode_path DOES NOT exists"
      exit 1
    fi
  fi
  if [ "$tree_type" == $SINGLE_TREE ] || [ "$tree_type" == $SINGLE_TREE_GROUPS ] || [ "$tree_type" == $VENDOR_TREE_STANDALONE ]
    then
    if [ ! -d "$vendor_chipcode_path" ]
      then
      echo "Vendor SI chipcode PATH vendor_chipcode_path DOES NOT exists"
      exit 1
    fi
  fi
fi

if [ -z "$jobs" ]; then
  echo "No input for jobs, defaulting to 8"
  jobs=8
fi

mkdir -p $ROOT_PWD
rm -rf $ROOT_PWD/.repo/local_manifests
rm -rf $ROOT_PWD/$QSSI_CAF_DIR_NAME $ROOT_PWD/$QSSI_PROP_DIR_NAME $ROOT_PWD/$VENDOR_CAF_DIR_NAME $ROOT_PWD/$VENDOR_PROP_DIR_NAME
mkdir -p $ROOT_PWD/$REPO_BACKUP_PATH
pushd $ROOT_PWD
rm -rf $ROOT_PWD/prebuilt_*



if [ -d $ROOT_PWD/.repo ]; then
  pushd $ROOT_PWD/$REPO_MANIFESTS
  git reset --hard
  pushd $ROOT_PWD
fi

if [ -d $ROOT_PWD/$REPO_BACKUP_PATH/.repo ]; then
  pushd $ROOT_PWD/$REPO_BACKUP_PATH/$REPO_MANIFESTS
  git reset --hard
  mv $ROOT_PWD/$REPO_BACKUP_PATH/.repo $ROOT_PWD/
  pushd $ROOT_PWD
fi


#check, if the req is to sync QSSI Tree
if [ "$tree_type" == $QSSI_TREE ]; then
  qssi_grease_fetch_url="ssh://${qssi_grease_user}@${qssi_grease_server}:29418/"
  qssi_grease_mf_url="${qssi_grease_fetch_url}${qssi_grease_manifest_repo}"
  qssi_au="$(echo $qssi_au_tag | \
            sed 's/^.*\([[:digit:]]\{2\}\.[[:digit:]]\{2\}\.[[:digit:]]\{2\}\.[[:digit:]]\{2,\}\.[[:digit:]]\{3\}\).*/\1/')"

  mkdir -p $ROOT_PWD/$QSSI_CAF_DIR_NAME
  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    mkdir -p $ROOT_PWD/$QSSI_PROP_DIR_NAME
  fi
  pushd $ROOT_PWD
  echo " repo init QSSI OSS"
  echo "repo init command = repo init -u $qssi_caf_server_url/$qssi_caf_manifest_repo.git -b release -m caf_$qssi_au_tag.xml --repo-url=$repo_url --repo-branch=$repo_branch "
  repo init -u $qssi_caf_server_url/$qssi_caf_manifest_repo.git -b release -m caf_$qssi_au_tag.xml --repo-url=$repo_url --repo-branch=$repo_branch
  echo "QSSI SI OSS init done"

if [ -d $ROOT_PWD/.repo ]; then
  rm -rf $ROOT_PWD/$REPO_BACKUP_PATH/.repo
  mv $ROOT_PWD/.repo $ROOT_PWD/$REPO_BACKUP_PATH/
fi

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    pushd $ROOT_PWD/$QSSI_PROP_DIR_NAME
    echo " repo init QSSI SI PROP"
    qssi_grease_mf_name="${qssi_au_tag}.xml"
    echo " repo init -u $qssi_grease_mf_url \
            -m $qssi_grease_mf_name \
            -b $qssi_branch \
            --repo-url=$repo_url \
            --repo-branch=$repo_branch"

    repo init -u $qssi_grease_mf_url \
            -m $qssi_grease_mf_name \
            -b $qssi_branch \
            --repo-url=$repo_url \
            --repo-branch=$repo_branch
    echo "QSSI SI PROP init done"
  fi
  pushd $ROOT_PWD
fi

#check, if the req is to sync Vendor/Vendor standalone Tree
if [ "$tree_type" == $SINGLE_TREE ] || [ "$tree_type" == $SINGLE_TREE_GROUPS ] || [ "$tree_type" == $VENDOR_TREE_STANDALONE ]
 then
    mkdir -p $ROOT_PWD/$QSSI_CAF_DIR_NAME
    mkdir -p $ROOT_PWD/$VENDOR_CAF_DIR_NAME
  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    mkdir -p $ROOT_PWD/$VENDOR_PROP_DIR_NAME
    mkdir -p $ROOT_PWD/$QSSI_PROP_DIR_NAME
  fi

  pushd $ROOT_PWD
  echo " repo init Vendor SI OSS"
  ARGS="-u $vendor_caf_server_url/$vendor_caf_manifest_repo.git -b release -m caf_$vendor_au_tag.xml --repo-url=$repo_url --repo-branch=$repo_branch"
  if [ ! -z $groups ] && [ "$tree_type" == $SINGLE_TREE_GROUPS ]; then
        ARGS="$ARGS -g $groups"
  fi
  echo "repo init $ARGS"
  repo init $ARGS
  echo "Vendor SI OSS init done"
if [ -d $ROOT_PWD/.repo ]; then
  rm -rf $ROOT_PWD/$REPO_BACKUP_PATH/.repo
  mv $ROOT_PWD/.repo $ROOT_PWD/$REPO_BACKUP_PATH/
fi
  echo "Get QSSI AU_TAG"
  qssi_au_tag=$(xmlstarlet sel -T -t -m  "//refs/image[@project='la/system/manifest']" -v "@tag" -nl $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/caf_$vendor_au_tag.xml)
  echo "QSSI_AU_TAG=$qssi_au_tag"
  vendor_grease_fetch_url="ssh://${vendor_grease_user}@${vendor_grease_server}:29418/"
  vendor_grease_mf_url="${vendor_grease_fetch_url}${vendor_grease_manifest_repo}"
  vendor_au="$(echo $vendor_au_tag | \
            sed 's/^.*\([[:digit:]]\{2\}\.[[:digit:]]\{2\}\.[[:digit:]]\{2\}\.[[:digit:]]\{2,\}\.[[:digit:]]\{3\}\).*/\1/')"

  if [ "$tree_type" == $SINGLE_TREE ] || [ "$tree_type" == $SINGLE_TREE_GROUPS ]
  then
    qssi_grease_fetch_url="ssh://${qssi_grease_user}@${qssi_grease_server}:29418/"
    qssi_grease_mf_url="${qssi_grease_fetch_url}${qssi_grease_manifest_repo}"
    qssi_au="$(echo $qssi_au_tag | \
              sed 's/^.*\([[:digit:]]\{2\}\.[[:digit:]]\{2\}\.[[:digit:]]\{2\}\.[[:digit:]]\{2,\}\.[[:digit:]]\{3\}\).*/\1/')"
    echo "QSSI_AU_TAG=$qssi_au_tag"
    echo "QSSI_AU=$qssi_au"

    pushd $ROOT_PWD/$QSSI_CAF_DIR_NAME
    echo " repo init QSSI OSS"
    echo "repo init command = repo init -u $qssi_caf_server_url/$qssi_caf_manifest_repo.git -b release -m caf_$qssi_au_tag.xml --repo-url=$repo_url --repo-branch=$repo_branch "
    ARGS="-u $qssi_caf_server_url/$qssi_caf_manifest_repo.git -b release -m caf_$qssi_au_tag.xml --repo-url=$repo_url --repo-branch=$repo_branch"
    if [ ! -z $groups ] && [ "$tree_type" == $SINGLE_TREE_GROUPS ]; then
        ARGS="$ARGS -g $groups"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "QSSI OSS init done"
  fi

    if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
      pushd $ROOT_PWD/$VENDOR_PROP_DIR_NAME
      echo " repo init Vendor SI PROP"
      vendor_grease_mf_name="${vendor_au_tag}.xml"
      ARGS="-u $vendor_grease_mf_url \
            -m $vendor_grease_mf_name \
            -b $vendor_branch \
            --repo-url=$repo_url \
            --repo-branch=$repo_branch"
      if [ ! -z $groups ] && [ "$tree_type" == $SINGLE_TREE_GROUPS ]; then
          ARGS="$ARGS -g $groups"
      fi
      echo "repo init $ARGS"
      repo init $ARGS
      echo "Vendor SI PROP init done"

  if [ "$tree_type" == $SINGLE_TREE ] || [ "$tree_type" == $SINGLE_TREE_GROUPS ]
  then
      pushd $ROOT_PWD/$QSSI_PROP_DIR_NAME
      echo " repo init QSSI SI PROP"
      qssi_grease_mf_name="${qssi_au_tag}.xml"
      ARGS="-u $qssi_grease_mf_url \
            -m $qssi_grease_mf_name \
            -b $qssi_branch \
            --repo-url=$repo_url \
            --repo-branch=$repo_branch"
      if [ ! -z $groups ] && [ "$tree_type" == $SINGLE_TREE_GROUPS ]; then
          ARGS="$ARGS -g $groups"
      fi
      echo "repo init $ARGS"
      repo init $ARGS
      echo "QSSI SI PROP init done"
    fi
  fi
  pushd $ROOT_PWD
fi

if [ "$tree_type" == $QSSI_TREE ] || [ "$tree_type" == $SINGLE_TREE ] || [ "$tree_type" == $SINGLE_TREE_GROUPS ]
then
  # Since the defaults will likely conflict (like remote), delete them.
  # Must remove the defaults before updating so that the default remote
  # is updated correctly.
  pushd $ROOT_PWD
  if [ "$tree_type" == $QSSI_TREE ]
  then
    if [ -d $ROOT_PWD/$REPO_BACKUP_PATH/.repo ];then
      mv $ROOT_PWD/$REPO_BACKUP_PATH/.repo $ROOT_PWD/
    fi
  fi
  echo "calling remove defaults"

  if [ "$tree_type" == $SINGLE_TREE ] || [ "$tree_type" == $SINGLE_TREE_GROUPS ]
  then
    qssi_caf_manifest=$ROOT_PWD/$QSSI_CAF_DIR_NAME/.repo/manifests/caf_$qssi_au_tag.xml

    qremote="cafq"
    echo "qssi CAF manifest = $qssi_caf_manifest, qremote = $qremote"
    xml1=$(cat "$qssi_caf_manifest" | remove_defaults)

    echo "Getting remotes"
    m1remotes=($(cat "$qssi_caf_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
    echo "replacing remotes"
    for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
    done
    rm -rf $ROOT_PWD/qcafmanifest.xml
    {
      echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

      echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
      echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

      echo '</manifest>'
    } |  xmlstarlet fo >> $ROOT_PWD/qcafmanifest.xml

    echo " qssi caf manifest processing is done"
    mv $ROOT_PWD/qcafmanifest.xml $ROOT_PWD/$QSSI_CAF_DIR_NAME/.repo/manifests/caf_${qssi_au_tag}.xml
  fi

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    qssi_grease_manifest=$ROOT_PWD/$QSSI_PROP_DIR_NAME/.repo/manifests/${qssi_au_tag}.xml
    qremote="greaseq"
    echo "Vendor Grease manifest = $qssi_grease_manifest, qremote = $qremote"
    xml1=$(cat "$qssi_grease_manifest" | remove_defaults)

    echo "Getting remotes"
    m1remotes=($(cat "$qssi_grease_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
    echo "replacing remotes"
    for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
    done
    rm -rf $ROOT_PWD/qgreasemanifest.xml
    {
      echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

      echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
      echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

      echo '</manifest>'
    } |  xmlstarlet fo >> $ROOT_PWD/qgreasemanifest.xml

    echo " qssi grease manifest processing is done"
    mv $ROOT_PWD/qgreasemanifest.xml $ROOT_PWD/$QSSI_PROP_DIR_NAME/.repo/manifests/${qssi_au_tag}.xml

    echo "getting QSSI prebuilts"
    # Test Grease Gerrit server connection
    set +e
    ssh -p 29418 ${qssi_grease_user}@${qssi_grease_server}
    if [ $? -ne 127 ]; then
      echo "Please check your grease user name and password for correctness."
      usage
    fi
    set -e
    pushd $ROOT_PWD
    qssi_cdr_customer=$(dirname ${qssi_branch})
    qssi_tar_file_gz="prebuilt_${qssi_cdr_customer}_${qssi_au_tag}.tar.gz"
    qssi_tar_file_xz="prebuilt_${qssi_cdr_customer}_${qssi_au_tag}.tar.xz"
    qssi_parent_branch=`echo ${qssi_branch} | cut -d '/' -f2`
    qssi_tar_file_on_server_gz="https://${qssi_grease_server}/binaries/outgoing/${qssi_cdr_customer}/${qssi_parent_branch}/${qssi_tar_file_gz}"
    qssi_tar_file_on_server_xz="https://${qssi_grease_server}/binaries/outgoing/${qssi_cdr_customer}/${qssi_parent_branch}/${qssi_tar_file_xz}"
   ( wget --continue --no-check-certificate --user=$qssi_grease_user \
       --password=$qssi_grease_pass $qssi_tar_file_on_server_gz ||
     wget --continue --no-check-certificate --user=$qssi_grease_user \
       --password=$qssi_grease_pass $qssi_tar_file_on_server_xz)
  fi
fi #IF NOT VENDOR_TREE_STANDALONE

if [ "$tree_type" == $SINGLE_TREE ] || [ "$tree_type" == $SINGLE_TREE_GROUPS ] || [ "$tree_type" == $VENDOR_TREE_STANDALONE ]
then
  if [ -d $ROOT_PWD/$REPO_BACKUP_PATH/.repo ];then
    mv $ROOT_PWD/$REPO_BACKUP_PATH/.repo $ROOT_PWD/
  fi

  echo "Initializing local manifest"
  mkdir -p $ROOT_PWD/.repo/local_manifests
  if [ "$tree_type" == $SINGLE_TREE ] || [ "$tree_type" == $SINGLE_TREE_GROUPS ]
  then
    cp $ROOT_PWD/$QSSI_CAF_DIR_NAME/.repo/manifests/caf_${qssi_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
    echo "QSSSI CAF Manifest copied as local manifest to Vendor"
  fi

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    vendor_grease_manifest=$ROOT_PWD/$VENDOR_PROP_DIR_NAME/.repo/manifests/${vendor_au_tag}.xml
    qremote="greasev"
    echo "Vendor Grease manifest = $vendor_grease_manifest, qremote = $qremote"
    xml1=$(cat "$vendor_grease_manifest" | remove_defaults)

    echo "Getting remotes"
    m1remotes=($(cat "$vendor_grease_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
    echo "replacing remotes"
    for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
    done
    rm -rf $ROOT_PWD/vgreasemanifest.xml
    {
      echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

      echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
      echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

      echo '</manifest>'
    } |  xmlstarlet fo >> $ROOT_PWD/vgreasemanifest.xml

    echo " vendor grease manifest processing is done"
    mv $ROOT_PWD/vgreasemanifest.xml $ROOT_PWD/$VENDOR_PROP_DIR_NAME/.repo/manifests/${vendor_au_tag}.xml

    cp $ROOT_PWD/$VENDOR_PROP_DIR_NAME/.repo/manifests/${vendor_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
    echo "Vendor Grease Manifest copied as local manifest to Vendor"

    if [ "$tree_type" == $SINGLE_TREE ] || [ "$tree_type" == $SINGLE_TREE_GROUPS ]
    then
      cp $ROOT_PWD/$QSSI_PROP_DIR_NAME/.repo/manifests/${qssi_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
      echo "QSSI Grease Manifest copied as local manifest to Vendor"
    fi

    echo "getting prebuilts"
    # Test Grease Gerrit server connection
    set +e
    ssh -p 29418 ${vendor_grease_user}@${vendor_grease_server}
    if [ $? -ne 127 ]; then
      echo "Please check your grease user name and password for correctness."
      usage
    fi
    set -e
    pushd $ROOT_PWD
    vendor_cdr_customer=$(dirname ${vendor_branch})
    vendor_tar_file_gz="prebuilt_${vendor_cdr_customer}_${vendor_au_tag}.tar.gz"
    vendor_tar_file_xz="prebuilt_${vendor_cdr_customer}_${vendor_au_tag}.tar.xz"
    vendor_parent_branch=`echo ${vendor_branch} | cut -d '/' -f2`
    vendor_tar_file_on_server_gz="https://${vendor_grease_server}/binaries/outgoing/${vendor_cdr_customer}/${vendor_parent_branch}/${vendor_tar_file_gz}"
    vendor_tar_file_on_server_xz="https://${vendor_grease_server}/binaries/outgoing/${vendor_cdr_customer}/${vendor_parent_branch}/${vendor_tar_file_xz}"
   ( wget --continue --no-check-certificate --user=$vendor_grease_user \
       --password=$vendor_grease_pass $vendor_tar_file_on_server_gz ||
     wget --continue --no-check-certificate --user=$vendor_grease_user \
       --password=$vendor_grease_pass $vendor_tar_file_on_server_xz)
  fi

  # Move old prebuilt dirs
  prebuilt_dirs="$ROOT_PWD/$PREBUILT_HY11_PATH $ROOT_PWD/$PREBUILT_GREASE_PATH"
  datetime="$(date +%F_%H.%M.%S)"
  for dir in $prebuilt_dirs; do
      if [ -d "$dir" ]; then
          mv $dir ${dir}.$datetime
      fi
  done

  pushd $ROOT_PWD
  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
    echo "Getting chipcode content, takes several minutes"
    mkdir -p  $ROOT_PWD/vendor/qcom
    cp -r $vendor_chipcode_path/$PROP_PATH $ROOT_PWD/vendor/qcom/
    if [ "$tree_type" == $SINGLE_TREE ] || [ "$tree_type" == $SINGLE_TREE_GROUPS ]
    then
      cp -r $qssi_chipcode_path/$PROP_PATH/* $ROOT_PWD/$PROP_PATH/
      if [ ! -z $groups ] && [ "$tree_type" == $SINGLE_TREE_GROUPS ]; then
          remove_common_sys_prjs
      fi
    fi
  fi
  rm -rf $ROOT_PWD/$VENDOR_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$VENDOR_CAF_DIR_NAME
  rm -rf $ROOT_PWD/$QSSI_CAF_DIR_NAME
  rm -rf $ROOT_PWD/$QSSI_PROP_DIR_NAME
  echo "Tree sync initiated"
  pushd $ROOT_PWD
  sleep 2
  repo manifest > $COMBINED_MANIFEST
  echo "Combined Manifest generated"
  echo "repo sync -j${jobs}"
  repo sync -j${jobs}
  echo "repo sync successful"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    echo "extracting prebuilts"
    if [ -e prebuilt_${vendor_cdr_customer}_${vendor_au_tag}.tar.gz ]; then
      tar -zxvf prebuilt_${vendor_cdr_customer}_${vendor_au_tag}.tar.gz
    else
      tar -xvf prebuilt_${vendor_cdr_customer}_${vendor_au_tag}.tar.xz
    fi
    if [ "$tree_type" == $SINGLE_TREE ] || [ "$tree_type" == $SINGLE_TREE_GROUPS ]
    then
      if [ -e prebuilt_${qssi_cdr_customer}_${qssi_au_tag}.tar.gz ]; then
        tar -zxvf prebuilt_${qssi_cdr_customer}_${qssi_au_tag}.tar.gz
      else
        tar -xvf prebuilt_${qssi_cdr_customer}_${qssi_au_tag}.tar.xz
      fi
    fi
  fi

  if [ "$tree_type" == $SINGLE_TREE ] || [ "$tree_type" == $SINGLE_TREE_GROUPS ]
  then
    if [ "$tree_type" == $SINGLE_TREE ]; then
      echo "SINGLE_TREE is ready"
    fi
    if [ "$tree_type" == $SINGLE_TREE_GROUPS ]; then
      echo "SINGLE_TREE with groups option is ready"
    fi
  else
    echo "VENDOR_STANDALONE_TREE is ready"
  fi

fi #"$tree_type" = $SINGLE_TREE

if [ "$tree_type" == $QSSI_TREE ]; then
  mkdir -p $ROOT_PWD/.repo/local_manifests
  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then

    cp $ROOT_PWD/$QSSI_PROP_DIR_NAME/.repo/manifests/${qssi_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
    echo "QSSI Grease Manifest copied as local manifest to Vendor"
  fi

  pushd $ROOT_PWD
  # Move old prebuilt dirs
  prebuilt_dirs="$ROOT_PWD/$PREBUILT_HY11_PATH $ROOT_PWD/$PREBUILT_GREASE_PATH"
  datetime="$(date +%F_%H.%M.%S)"
  for dir in $prebuilt_dirs; do
      if [ -d "$dir" ]; then
          mv $dir ${dir}.$datetime
      fi
  done

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
    echo "Getting chipcode content, takes several minutes"
    mkdir -p  $ROOT_PWD/vendor/qcom
    cp -r $qssi_chipcode_path/$PROP_PATH/ $ROOT_PWD/vendor/qcom/
  fi
  echo "QSSI_tree code sync initiated"
  pushd $ROOT_PWD
  sleep 2
  repo manifest > $COMBINED_MANIFEST
  echo "Combined Manifest generated"
  echo "repo sync -j${jobs}"
  repo sync -j${jobs}
  echo "repo sync successful"


  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    echo "extracting prebuilts"
    if [ -e prebuilt_${qssi_cdr_customer}_${qssi_au_tag}.tar.gz ]; then
      tar -zxvf prebuilt_${qssi_cdr_customer}_${qssi_au_tag}.tar.gz
    else
      tar -xvf prebuilt_${qssi_cdr_customer}_${qssi_au_tag}.tar.xz
    fi
  fi
  rm -rf $ROOT_PWD/$QSSI_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$QSSI_CAF_DIR_NAME
  echo "QSSI_STANDALONE_TREE is ready"
fi #"$tree_type" = $QSSI_TREE
