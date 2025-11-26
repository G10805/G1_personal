#!/bin/bash
#Copyright (c) 2021 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.

#Possible location of sourcecode/prebuilts
#Code/prebuilt bins	Private CAF/CLO	 Public CAF/CLO	 Grease/chipcodeHF 	Chip code
#QSSI SI (OSS)		Yes		 Yes		 NA	 		NA
#QSSI (Prop)		NA		 NA		 Yes	 		Yes
#Vendor SI (OSS)	Yes		 Yes		 NA	 		NA
#Vendor SI(Prop)	NA		 NA		 Yes	 		Yes

#QSSI Tree
#No need to sync vendor sources/prebuilts
#QSSI OSS - can be private/public CAF/CLO
#QSSI prop(sources and prebuilts) - Grease/Chipcode/ChipcodeHF

#Vendor Tree
#Vendor SI OSS - can be private/public CAF/CLO
#Vendor SI prop(Sources/Prebuilts) - Grease/Chipcode/ChipcodeHF
#QSSI OSS - can be private/Public CAF/CLO
#QSSI prop(sources) - Grease/Chipcode/chipcodeHF
#QSSI prop(prebuilts) - NA

#Vendor standalone Tree
#Vendor SI OSS - can be private/public CAF/CLO
#Vendor SI prop(Sources/Prebuilts) - Grease/Chipcode/ChipcodeHF

#Single Tree
#Vendor SI OSS - can be private/public CAF/CLO
#Vendor SI prop(Sources/Prebuilts) - Grease/Chipcode/ChipcodeHF
#QSSI OSS - can be private/Public CAF/CLO
#QSSI prop(sources and prebuilts)- Grease/Chipcode/chipcodeHF

#Manifest Management
#QSSI Tree - single manaifest - no need of any local manifest(init CAF/CLO manifest and Grease/ChipcodeHF manifest in seperate directories)
#Vendor Tree - init Vendor and QSSI SI manifests(Vendor CAF/CLO, Vendor Grease/ChipcodeHF, QSSI CAF/CLO and QSSI Grease/ChipcodeHF) - ignore grease manifest incase of Chipcode
#Single Tree - init Vendor and QSSI SI manifests(Vendor CAF/CLO, Vendor Grease/chipcodeHF, QSSI CAF/CLO and QSSI Grease/ChipcodeHF) - ignore grease manifest incase of Chipcode

#Unsupported combo(expectation is both Vendor SI and QSSI SI prop content is sourced from same(Either grease/ChipcodeHF or Chipcode)
#Vendor prop from Chipcode +  QSSI prop from Grease/ChipcodeHF
#QSSI prop from Chipcode   +  Vendor prop from Grease/ChipcodeHF

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

remove_defaults_update_remotes() {
  image_manifest=$1
  qremote=$2
  echo "image_manifest = $image_manifest, qremote = $qremote"
  xml1=$(cat "$image_manifest" | remove_defaults)

  echo "Getting remotes"
  m1remotes=($(cat "$image_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
  echo "replacing remotes"
  if [[ $3 =~ "LE" ]];then
    for m1remote in "${m1remotes[@]}"; do
      echo "m1remote = "$m1remote""
      if [[ "$m1remote" == "caf" || "$m1remote" == "clo" ]]; then
        xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      fi
    done
    else
    for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
    done
  fi
  rm -rf $ROOT_PWD/qimagemanifest.xml
  {
    echo '<?xml version="1.0" encoding="UTF-8"?>
        <manifest>'

    echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
    echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

    echo '</manifest>'
  } |  xmlstarlet fo >> $ROOT_PWD/qimagemanifest.xml

  echo "Image manifest processing is done"
}

usage() {
  echo ""
  echo "USAGE:"
  echo "TBD"
  echo ""
  exit 1
}

oss_args_check () {

  local si_type="$1"
  local oss_manifest_git="$2"
  local oss_url="$3"

  echo "In $si_type oss param check"

  if [ -z "$oss_manifest_git" ]; then
    echo "Pls supply $si_type oss manifest Repo info"
    usage
  fi
  echo "###################"
  echo $oss_url
  if [ -z "$oss_url" ]; then
    echo "Pls supply $si_type oss server url"
    usage
  fi

 return 0
}

chipcode_hf_args_check() {

  local si_type="$1"
  local chipcode_hf_server="$2"
  local chipcode_hf_manifest_git="$3"
  local chipcode_hf_manifest_branch="$4"

  echo "In $si_type Chipcode History Format param check"

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
     echo "Proprietary content will be downloaded from 'Chipcode History Format'"

     if [ -z "$chipcode_hf_server" ]; then
        echo "Please supply $si_type ChipCode History Format Server."
        usage
     fi

     if [ -z "$chipcode_hf_manifest_git" ]; then
        echo "Please supply $si_type Manifest Git hosted on ChipCode History Format Server."
        usage
     fi

     if [ -z "$chipcode_hf_manifest_branch" ]; then
        echo "Please supply $si_type Manifest Branch hosted on ChipCode History Format Server."
        usage
     fi

     echo "$si_type Chipcode History Format Server: $chipcode_hf_server"
     echo "$si_type Manifest Git hosted on Chipcode History Format Server: $chipcode_hf_manifest_git"
     echo "$si_type Manifest Git branch hosted on Chipcode History Format Server: $chipcode_hf_manifest_branch"
  fi

  return 0
}

chipcode_args_check() {

  local si_type="$1"
  local chipcode_path="$2"

  echo "In QSSI Chipcode param check"

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     echo "Proprietary content will be downloaded/copied from 'Chipcode'"

     if [ -z "$chipcode_path" ]; then
          echo "Please supply $si_type SI chipcode path"
          usage
     fi
     echo "$si_type Chipcode Path: $chipcode_path"
  fi

  return 0
}

grease_args_check() {

  local si_type="$1"
  local grease_server="$2"
  local grease_branch="$3"

  echo "In $si_type Grease param check"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     echo "Proprietary content will be downloaded from 'Grease'"

     if [ -z "$grease_userid" ]; then
        grease_user=$grease_userid
     fi

     if [ -z "$grease_pass" ]; then
        grease_pass=$grease_pass
     fi

     if [ -z "$grease_branch" ]; then
        ${si_type}_grease_branch=$grease_branch
     fi

     if [ -z "$grease_server" ]; then
        grease_server=$grease_server
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


kernel_au_fnd_str="KERNEL.PLATFORM"
found_kernel_au=false
display_au_fnd_str="TECHPACK_DISPLAY"
camera_au_fnd_str="TECHPACK_CAMERA"
audio_au_fnd_str="TECHPACK_AUDIO"
video_au_fnd_str="TECHPACK_VIDEO"
sensors_au_fnd_str="TECHPACK_SENSOR"
cv_au_fnd_str="TECHPACK_CV"
graphics_au_fnd_str="TECHPACK_GRAPHICS"
xr_au_fnd_str="TECHPACK_XR"
found_display_au=false
found_camera_au=false
found_video_au=false
found_audio_au=false
found_sensor_au=false
found_cv_au=false
found_graphics_au=false
found_xr_au=false
qssi_au_fnd_str="LA.QSSI"
COMBINED_MANIFEST=".repo/manifests/snap_combined_manifest.xml"
PREBUILT_HY11_PATH="vendor/qcom/proprietary/prebuilt_HY11"
PREBUILT_GREASE_PATH="vendor/qcom/proprietary/prebuilt_grease"
REPO_MANIFESTS=".repo/manifests/"
REPO_MANIFESTS_GIT=".repo/manifests.git"
PROP_PATH="vendor/qcom/proprietary/"
REPO_BACKUP_PATH=".repo_bak"
IMAGE_OSS_DIR_NAME="image_oss"
IMAGE_PROP_DIR_NAME="image_prop"
VENDOR_OSS_DIR_NAME="vendor_oss"
VENDOR_PROP_DIR_NAME="vendor_prop"
QSSI_OSS_DIR_NAME="qssi_oss"
QSSI_PROP_DIR_NAME="qssi_prop"
KERNEL_OSS_DIR_NAME="kernel_oss"
LE_OSS_DIR_NAME="le_oss"
KERNEL_PROP_DIR_NAME="kernel_prop"
DISPLAY_OSS_DIR_NAME="display_oss"
DISPLAY_PROP_DIR_NAME="display_prop"
CAMERA_OSS_DIR_NAME="camera_oss"
CAMERA_PROP_DIR_NAME="camera_prop"
VIDEO_OSS_DIR_NAME="video_oss"
VIDEO_PROP_DIR_NAME="video_prop"
AUDIO_OSS_DIR_NAME="audio_oss"
AUDIO_PROP_DIR_NAME="audio_prop"
SENSOR_OSS_DIR_NAME="sensor_oss"
SENSOR_PROP_DIR_NAME="sensor_prop"
CV_OSS_DIR_NAME="cv_oss"
CV_PROP_DIR_NAME="cv_prop"
GRAPHICS_OSS_DIR_NAME="graphics_oss"
GRAPHICS_PROP_DIR_NAME="graphics_prop"
XR_OSS_DIR_NAME="xr_oss"
XR_PROP_DIR_NAME="xr_prop"
LE_STANDALONE="le_standalone"
LE_COMBINED="le_tree"
LA_COMBINED_TREE="la_tree"
LA_VENDOR_TECHPACK_TREE="la_vendor_techpack"
LE_APPS_PROC_PATH="apps_proc"
LA_COMBINED_TREE_GROUPS="la_tree_groups"
QSSI_TREE_STANDALONE="la_qssi"
VENDOR_TREE_STANDALONE="la_vendor"
KERNEL_TREE_STANDALONE="kernel_platform"
DISPLAY_TREE_STANDALONE="techpack_display"
CAMERA_TREE_STANDALONE="techpack_camera"
VIDEO_TREE_STANDALONE="techpack_video"
AUDIO_TREE_STANDALONE="techpack_audio"
SENSOR_TREE_STANDALONE="techpack_sensor"
CV_TREE_STANDALONE="techpack_cv"
GRAPHICS_TREE_STANDALONE="techpack_graphics"
XR_TREE_STANDALONE="techpack_xr"
PROP_DEST_GREASE="grease"
PROP_DEST_CHIPCODE="chipcode"
PROP_DEST_CHIPCODE_HF="chipcode_hf"
KERNEL_PLATFORM_PATH="kernel_platform"
LE_IMAGE="le"
LA_IMAGE="la"
quiet=""
shallow_clone=true
groups=""
OPTIONAL_SYNC_ARG="--current-branch --no-tags --force-sync"
le_ksrc_path="src/kernel-5.10"

INPUT_V21_OPTS=optional_sync_arg,image_type:,workspace_path:,tree_type:,prop_opt:,repo_url:,repo_branch:,groups:,shallow_clone:,quiet:,jobs:,snap_release:,chipcode_customer_id:,nhprop_chipcode_path:

INPUT_V2_OPTS=optional_sync_arg,image_type:,workspace_path:,tree_type:,prop_opt:,common_oss_url:,vendor_oss_manifest_git:,vendor_oss_url:,grease_server:,grease_userid:,grease_pass:,vendor_au_tag:,vendor_crm_label:,vendor_grease_branch:,vendor_chipcode_path:,qssi_oss_manifest_git:,qssi_oss_url:,qssi_au_tag:,qssi_crm_label:,qssi_grease_branch:,qssi_chipcode_path:,le_crmid:,le_oss_url:,le_oss_manifest_git:,le_chipcode_path:,kernel_oss_url:,kernel_oss_manifest_git:,kernel_au_tag:,kernel_grease_branch:,kernel_chipcode_path:,graphics_oss_url:,graphics_oss_manifest_git:,graphics_au_tag:,graphics_crm_label:,graphics_grease_branch:,graphics_chipcode_path:,xr_oss_url:,xr_oss_manifest_git:,xr_au_tag:,xr_crm_label:,xr_grease_branch:,xr_chipcode_path:,camera_oss_url:,camera_oss_manifest_git:,camera_au_tag:,camera_crm_label:,camera_grease_branch:,camera_chipcode_path:,sensor_oss_url:,sensor_oss_manifest_git:,sensor_au_tag:,sensor_crm_label:,sensor_grease_branch:,sensor_chipcode_path:,cv_oss_url:,cv_oss_manifest_git:,cv_au_tag:,cv_crm_label:,cv_grease_branch:,cv_chipcode_path:,audio_oss_url:,audio_oss_manifest_git:,audio_au_tag:,audio_crm_label:,audio_grease_branch:,audio_chipcode_path:,video_oss_url:,video_oss_manifest_git:,video_au_tag:,video_crm_label:,video_grease_branch:,video_chipcode_path:,display_oss_url:,display_oss_manifest_git:,display_au_tag:,display_crm_label:,display_grease_branch:,display_chipcode_path:,common_chipcode_hf_server:,vendor_chipcode_hf_server:,vendor_chipcode_hf_manifest_git:,vendor_chipcode_hf_manifest_branch:,qssi_chipcode_hf_server:,qssi_chipcode_hf_manifest_git:,qssi_chipcode_hf_manifest_branch:,kernel_chipcode_hf_server:,kernel_chipcode_hf_manifest_git:,kernel_chipcode_hf_manifest_branch:,display_chipcode_hf_server:,display_chipcode_hf_manifest_git:,display_chipcode_hf_manifest_branch:,camera_chipcode_hf_server:,camera_chipcode_hf_manifest_git:,camera_chipcode_hf_manifest_branch:,xr_chipcode_hf_server:,xr_chipcode_hf_manifest_git:,xr_chipcode_hf_manifest_branch:,sensor_chipcode_hf_server:,sensor_chipcode_hf_manifest_git:,sensor_chipcode_hf_manifest_branch:,graphics_chipcode_hf_server:,graphics_chipcode_hf_manifest_git:,graphics_chipcode_hf_manifest_branch:,cv_chipcode_hf_server:,cv_chipcode_hf_manifest_git:,cv_chipcode_hf_manifest_branch:,audio_chipcode_hf_server:,audio_chipcode_hf_manifest_git:,audio_chipcode_hf_manifest_branch:,video_chipcode_hf_server:,video_chipcode_hf_manifest_git:,video_chipcode_hf_manifest_branch:,repo_url:,repo_branch:,groups:,shallow_clone:,quiet:,jobs:

echo "arg = $@"
if [ $1 == "-D" ] || [  $1 == "-T" ] || [  $1 == "-R" ] || [  $1 == "-R" ] || [  $1 == "-c" ] || [  $1 == "-A" ] || [  $1 == "-L" ]; then
  DIR="$(cd "$(dirname "$0")" && pwd)"
  echo "Using V1 sync method,calling sync_snap.sh from $DIR"
  $DIR/sync_snap.sh $@
else
  if [[ $1 == *"--snap_release"* ]] || [[ $2 == *"--snap_release"* ]] || [[ $3 == *"--snap_release"* ]] || [[ $4 == *"--snap_release"* ]] || [[ $5 == *"--snap_release"* ]] || [[ $6 == *"--snap_release"* ]] || [[ $7 == *"--snap_release"* ]]; then
    echo "Using V2.1 sync method"
    OPTIONS=$(getopt  --long $INPUT_V21_OPTS -o '' -- "$@")
    eval set -- "$OPTIONS"
    while true ; do
      case "$1" in
       --workspace_path ) tree_dest_path="$2"; shift 2;;
       --image_type ) image_type="$2"; shift 2;;
       --tree_type ) tree_type=$2; shift 2;;
       --prop_opt ) prop_dest=$2; shift 2;;
       --snap_release ) snap_release_xml=$2; shift 2;;
       --chipcode_customer_id ) hf_customer_id=$2; shift 2;;
       --nhprop_chipcode_path ) nhprop_chipcode_path=$2; shift 2;;
       --repo_url ) repo_url=$2; shift 2;;
       --repo_branch ) repo_branch=$2; shift 2;;
       --groups ) groups=$2; shift 2;;
       --shallow_clone ) shallow_clone=$2; shift 2;;
       --jobs ) jobs=$2; shift 2;;
       --optional_sync_arg ) optional_sync_arg=$OPTIONAL_SYNC_ARG; shift;;
       -- ) shift; break;;
       *)echo "Error!!!";exit 1;;
      esac
    done

    echo "tree_dest_path = $tree_dest_path"
    echo "snap_release input = $snap_release_xml"
    echo "chipcode_customer_id = $hf_customer_id"
    echo "tree_type = $tree_type"
    sync_standalone_tree=true
    MASTER_ROOT_PWD=$tree_dest_path
    MASTER_REPO_PATH="$MASTER_ROOT_PWD"
    master_repo_flag=true


    software_image_combo=$(xmlstarlet sel -t -v "//snap_release/image/@${tree_type}" -nl $snap_release_xml)
    echo "Requested Image Combination = $software_image_combo"
    sw_image_count=$(echo "$software_image_combo" | wc --word)
    if [ $sw_image_count -gt 1 ];then
      echo "sw_image_count = $sw_image_count, setting standalone sync to false"
      sync_standalone_tree=false
    fi

    if [[ $tree_type =~ "comb" ]] || [[ $tree_type =~ "software_image_combo" ]] || [[ $tree_type =~ "software_image" ]]
    then
      echo "setting standalone sync to false"
      sync_standalone_tree=false
    fi

    if [ "$sync_standalone_tree" = false ]
    then
      echo " combined tree request"
      software_image_combo=$(xmlstarlet sel -t -v "//snap_release/image/@${tree_type}" -nl $snap_release_xml)
      echo "software_image_combo = $software_image_combo"
      software_image_combo=$software_image_combo
      for token in ${software_image_combo}
      do
        echo "software_image = $token"
      done
    else software_image_combo=$tree_type
    fi

    echo "sync_standalone_tree = $sync_standalone_tree"
    echo "MASTER_ROOT_PWD = $MASTER_ROOT_PWD"
    rm -rf $tree_dest_path/.repo/local_manifests

    no_oss_iteration=true

    for token in ${software_image_combo}
    do
      rm -rf $tree_dest_path/$token
        ROOT_PWD=$tree_dest_path
      mkdir -p $ROOT_PWD/$REPO_BACKUP_PATH
      if [ -d $ROOT_PWD/.repo ]; then
        pushd $ROOT_PWD/$REPO_MANIFESTS
        git reset --hard
        pushd $ROOT_PWD
      fi

        if [ -d $ROOT_PWD/.repo ]; then
          rm -rf $ROOT_PWD/$REPO_BACKUP_PATH/.repo
	  rm -rf $ROOT_PWD/.repo/manifest.xml
          mv $ROOT_PWD/.repo $ROOT_PWD/$REPO_BACKUP_PATH/
        fi
    for software_image in $(xmlstarlet sel -t -v "//snap_release/image/@software_image" -nl $snap_release_xml)
    do
     echo "====================================================================="
     si_has_oss=true
     echo "In Image node"
     echo "software_image = $software_image"
     #snap_release_tree_type=$(xmlstarlet sel -T -t -m "//snap_release/image[@software_image='$software_image']" -v "@image_type" -nl $snap_release_xml)
     #echo snap_release_tree_type=$snap_release_tree_type
     prebuilts_dir=$(xmlstarlet sel -T -t -m "//snap_release/image[@software_image='$software_image']" -v "@prebuilts_dir" -nl $snap_release_xml)
     echo prebuilts_dir=$prebuilts_dir
     oss_url=$(xmlstarlet sel -T -t -m "//snap_release/image[@software_image='$software_image']" -v "@oss_url" -nl $snap_release_xml)
     echo oss_url=$oss_url
     oss_manifest_git=$(xmlstarlet sel -T -t -m "//snap_release/image[@software_image='$software_image']" -v "@oss_manifest_git" -nl $snap_release_xml)
     echo oss_manifest_git=$oss_manifest_git
     au_tag=$(xmlstarlet sel -T -t -m "//snap_release/image[@software_image='$software_image']" -v "@au_tag" -nl $snap_release_xml)
     echo au_tag=$au_tag
     prop_url=$(xmlstarlet sel -T -t -m "//snap_release/image[@software_image='$software_image']" -v "@prop_url" -nl $snap_release_xml)
     echo prop_url=$prop_url
     hf_manifest_git=$(xmlstarlet sel -T -t -m "//snap_release/image[@software_image='$software_image']" -v "@hf_manifest_git" -nl $snap_release_xml)
     echo hf_manifest_git=$hf_manifest_git
     hf_manifest_branch=$(xmlstarlet sel -T -t -m "//snap_release/image[@software_image='$software_image']" -v "@hf_manifest_branch" -nl $snap_release_xml)
     echo hf_manifest_branch=$hf_manifest_branch
     hf_manifest_branch=$hf_customer_id-$hf_manifest_branch
     echo "hf_manifest_branch = $hf_manifest_branch"

    if [ $oss_url == "NA" ] || [ $oss_manifest_git == "NA" ];then
        si_has_oss=false
        echo "software_image $software_image doesn't have OSS part"
    fi

    if [ -z "$repo_branch" ]; then
      repo_branch="clo-stable"
    fi

    if [ -z "$repo_url" ]; then
      repo_url="https://git.codelinaro.org/clo/la/tools/repo.git"
    fi

    if [ -z "$jobs" ]; then
      echo "No input for jobs, defaulting to 8"
      jobs=8
    fi
    if [ $tree_type == $software_image ] || [ $token == $software_image ];then
      echo "sync_standalone = $sync_standalone_tree"

      if [ "$sync_standalone_tree" = true ]; then
        ROOT_PWD=$tree_dest_path
      else
        ROOT_PWD=$tree_dest_path/$software_image
      fi

      echo "ROOT_PWD = $ROOT_PWD"
      mkdir -p $ROOT_PWD
      mkdir -p $ROOT_PWD/$IMAGE_PROP_DIR_NAME
      mkdir -p $ROOT_PWD/$REPO_BACKUP_PATH

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
      pushd $ROOT_PWD
      if [ "$si_has_oss" = true ]; then
        echo " repo init OSS manifest"
        ARGS="-c -u $oss_url/$oss_manifest_git.git -b release -m $au_tag.xml --repo-url=$repo_url --repo-branch=$repo_branch"
        if $shallow_clone; then
          ARGS="--depth=1 $ARGS"
        fi
        echo "repo init $ARGS"
        repo init $ARGS
        echo "OSS manifest repo init done"
        remove_defaults_update_remotes "$ROOT_PWD/.repo/manifests/$au_tag.xml" "${software_image}_oss" "${software_image}"
        mv $ROOT_PWD/qimagemanifest.xml $ROOT_PWD/.repo/manifests/$au_tag.xml

        if [ $master_repo_flag = true ];then
	      MASTER_REPO_PATH=$ROOT_PWD
	      master_manifest=$au_tag.xml
	      master_au_tag=$au_tag
	      master_repo_flag=false
        fi
        echo "MASTER_REPO_PATH = $MASTER_REPO_PATH"
      fi
      sleep 5


      if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
        if [ -d $ROOT_PWD/.repo ]; then
          rm -rf $ROOT_PWD/$REPO_BACKUP_PATH/.repo
          mv $ROOT_PWD/.repo $ROOT_PWD/$REPO_BACKUP_PATH/
        fi
        pushd $ROOT_PWD/$IMAGE_PROP_DIR_NAME
        echo " repo init PROP Manifest"
        chipcode_hf_mf_name="${au_tag}.xml"
	ARGS="-c --no-clone-bundle -u $prop_url/$hf_manifest_git -m $chipcode_hf_mf_name -b $hf_manifest_branch --repo-url=$repo_url --repo-branch=$repo_branch"
	if $shallow_clone; then
            ARGS="--depth=1 $ARGS"
        fi
        echo "repo init $ARGS"
        repo init $ARGS
        echo "SI PROP repo init done"
        remove_defaults_update_remotes "$ROOT_PWD/$IMAGE_PROP_DIR_NAME/.repo/manifests/$au_tag.xml" "${software_image}_prop" "${software_image}"
	mv $ROOT_PWD/qimagemanifest.xml $ROOT_PWD/$IMAGE_PROP_DIR_NAME/.repo/manifests/$au_tag.xml

        if [ -d $ROOT_PWD/$REPO_BACKUP_PATH/.repo ]; then
          mv $ROOT_PWD/$REPO_BACKUP_PATH/.repo $ROOT_PWD/
        fi
        if [ "$si_has_oss" = false ] && [ "$no_oss_iteration" = true ]; then
	  mkdir -p $ROOT_PWD/.repo/manifests/
          pushd $ROOT_PWD
          if [ $master_repo_flag = true ];then
              MASTER_REPO_PATH=$ROOT_PWD/$IMAGE_PROP_DIR_NAME
              master_manifest=$au_tag.xml
              master_au_tag=$au_tag
              master_repo_flag=false
	  else
	    mkdir -p $ROOT_PWD/.repo/local_manifests
	    echo "cp $ROOT_PWD/$IMAGE_PROP_DIR_NAME/.repo/manifests/$chipcode_hf_mf_name $ROOT_PWD/.repo/local_manifests/"
            cp $ROOT_PWD/$IMAGE_PROP_DIR_NAME/.repo/manifests/$chipcode_hf_mf_name $ROOT_PWD/.repo/local_manifests/hf_$chipcode_hf_mf_name
	  fi
          pushd $ROOT_PWD
          no_oss_iteration=false
        else
	  mkdir -p $ROOT_PWD/.repo/local_manifests
	  mkdir -p $MASTER_REPO_PATH/.repo/local_manifests
	  echo "cp $ROOT_PWD/$IMAGE_PROP_DIR_NAME/.repo/manifests/$chipcode_hf_mf_name $ROOT_PWD/.repo/local_manifests/"
          cp $ROOT_PWD/$IMAGE_PROP_DIR_NAME/.repo/manifests/$chipcode_hf_mf_name $ROOT_PWD/.repo/local_manifests/hf_$chipcode_hf_mf_name
          pushd $ROOT_PWD
        fi
      fi
    fi
    done
    done
    echo "Tree sync initiated"
    if [ "$sync_standalone_tree" = false ] && [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ];then
      if [ -d $tree_dest_path/$REPO_BACKUP_PATH/.repo ]; then
        mv $tree_dest_path/$REPO_BACKUP_PATH/.repo $tree_dest_path/
        cp $MASTER_REPO_PATH/.repo/manifests/$master_manifest $tree_dest_path/.repo/manifests
        ln -s $tree_dest_path/.repo/manifests/$master_manifest $tree_dest_path/.repo/manifest.xml
        mkdir -p $tree_dest_path/.repo/local_manifests
      else
        cp -r $MASTER_REPO_PATH/.repo $tree_dest_path/
      fi


      for token in ${software_image_combo}
      do
        temp_au_tag=$(xmlstarlet sel -T -t -m "//snap_release/image[@software_image='$token']" -v "@au_tag" -nl $snap_release_xml)
	if [ -f $tree_dest_path/$token/.repo/manifests/${temp_au_tag}.xml  ]; then
	 echo "$tree_dest_path/$token/.repo/manifests/${temp_au_tag}.xml is available for copy"
	 echo "copying $tree_dest_path/$token/.repo/manifests/${temp_au_tag}.xml to $tree_dest_path/.repo/local_manifests/"
	 cp $tree_dest_path/$token/.repo/manifests/${temp_au_tag}.xml $tree_dest_path/.repo/local_manifests/
        fi
	if [ -d $tree_dest_path/$token/.repo/local_manifests  ]; then
          echo "$tree_dest_path/$token/.repo/local_manifests/ is available"
	  echo "copying $tree_dest_path/$token/.repo/local_manifests/* $tree_dest_path/.repo/local_manifests/"
	  cp $tree_dest_path/$token/.repo/local_manifests/* $tree_dest_path/.repo/local_manifests/
	fi
      done
	echo "Removing $tree_dest_path/.repo/local_manifests/$master_manifest"
	rm -rf $tree_dest_path/.repo/local_manifests/$master_manifest
    fi
    if [ "$sync_standalone_tree" = false ] && [ "$prop_dest" == $PROP_DEST_CHIPCODE ];then
      if [ -d $tree_dest_path/$REPO_BACKUP_PATH/.repo ]; then
        mv $tree_dest_path/$REPO_BACKUP_PATH/.repo $tree_dest_path/
        cp $MASTER_REPO_PATH/.repo/manifests/$master_manifest $tree_dest_path/.repo/manifests
        ln -s $tree_dest_path/.repo/manifests/$master_manifest $tree_dest_path/.repo/manifest.xml
      else
        cp -r $MASTER_REPO_PATH/.repo $tree_dest_path/
      fi
      mkdir -p $tree_dest_path/.repo/local_manifests


      for token in ${software_image_combo}
      do
        temp_au_tag=$(xmlstarlet sel -T -t -m "//snap_release/image[@software_image='$token']" -v "@au_tag" -nl $snap_release_xml)
	if [ -f "$tree_dest_path/$token/.repo/manifests/${temp_au_tag}.xml" ];then
          echo "copying $tree_dest_path/$token/.repo/manifests/${temp_au_tag}.xml to $tree_dest_path/.repo/local_manifests/"
          cp $tree_dest_path/$token/.repo/manifests/${temp_au_tag}.xml $tree_dest_path/.repo/local_manifests/
        fi
      done
      echo "Removing $tree_dest_path/.repo/local_manifests/$master_manifest"
      rm -rf $tree_dest_path/.repo/local_manifests/$master_manifest
    fi
    pushd $tree_dest_path

    if [ -f "$tree_dest_path/.repo/manifests/$master_manifest" ];then
      sleep 2
      repo manifest > $COMBINED_MANIFEST
      echo "Combined Manifest generated"
      rm -rf $ROOT_PWD/$IMAGE_PROP_DIR_NAME $ROOT_PWD/$IMAGE_OSS_DIR_NAME
      for token in ${software_image_combo}
      do
        rm -rf $tree_dest_path/$token
      done
      if [ ! -z "$optional_sync_arg" ]; then
        echo "repo sync -j${jobs} $optional_sync_arg"
        repo sync -j${jobs} $optional_sync_arg
      else
        echo "repo sync -j${jobs}"
        repo sync -j${jobs}
      fi
    fi
    if [ "$sync_standalone_tree" = false ] && [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ];then
      for token in ${software_image_combo}
      do
        if [[ $token =~ "QSSI" ]];then
         cp -rf $tree_dest_path/system_prebuilt_dir/vendor/qcom/proprietary/prebuilt_HY11 $tree_dest_path/vendor/qcom/proprietary/
         rm -rf $tree_dest_path/system_prebuilt_dir
        fi
        if [[ $token =~ "KERNEL" ]];then
	  cp -rf $tree_dest_path/kernel_prebuilt_dir/kernel_platform/qcom/proprietary/prebuilt_HY11 $tree_dest_path/kernel_platform/qcom/proprietary/
	  rm -rf $tree_dest_path/kernel_prebuilt_dir
        fi
        if [[ $token =~ "VENDOR" ]];then
	  cp -rf $tree_dest_path/vendor_prebuilt_dir/vendor/qcom/proprietary/prebuilt_HY11 $tree_dest_path/vendor/qcom/proprietary/
	  if [ -d "$tree_dest_path/$PROP_PATH/prebuilt_HY11" ];then
	    cp -f $tree_dest_path/$PROP_PATH/prebuilt_HY11/Android.mk $tree_dest_path/$PROP_PATH/prebuilt_HY11/mAndroid.mk
	  fi
	  rm -rf $tree_dest_path/vendor_prebuilt_dir
        fi
      done
      if [ -f "$tree_dest_path/$PROP_PATH/prebuilt_HY11/mAndroid.mk" ];then
	mv $tree_dest_path/$PROP_PATH/prebuilt_HY11/mAndroid.mk $tree_dest_path/$PROP_PATH/prebuilt_HY11/Android.mk
      fi
      rm -rf $tree_dest_path/*_prebuilt_*
    fi

    if [ "$sync_standalone_tree" = true ] && [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ];then
	if [[ $tree_type =~ "QSSI" ]];then
	 cp -rf $tree_dest_path/system_prebuilt_dir/vendor/qcom/proprietary/prebuilt_HY11 $tree_dest_path/vendor/qcom/proprietary/
	 rm -rf $tree_dest_path/system_prebuilt_dir
        fi
	if [[ $tree_type =~ "KERNEL" ]];then
	 cp -rf $tree_dest_path/kernel_prebuilt_dir/kernel_platform/qcom/proprietary/prebuilt_HY11 $tree_dest_path/kernel_platform/qcom/proprietary/
	 rm -rf $tree_dest_path/kernel_prebuilt_dir
        fi
	if [[ $tree_type =~ "VENDOR" ]];then
	 cp -rf $tree_dest_path/vendor_prebuilt_dir/vendor/qcom/proprietary/prebuilt_HY11 $tree_dest_path/vendor/qcom/proprietary/
	  if [ -d "$tree_dest_path/$PROP_PATH/prebuilt_HY11" ];then
	    cp -f $tree_dest_path/$PROP_PATH/prebuilt_HY11/Android.mk $tree_dest_path/$PROP_PATH/prebuilt_HY11/mAndroid.mk
	  fi
	 rm -rf $tree_dest_path/vendor_prebuilt_dir
        fi
        if [ -f "$tree_dest_path/$PROP_PATH/prebuilt_HY11/mAndroid.mk" ];then
	  mv $tree_dest_path/$PROP_PATH/prebuilt_HY11/mAndroid.mk $tree_dest_path/$PROP_PATH/prebuilt_HY11/Android.mk
        fi
        echo "standalone tree $tree_type sync done, exiting"
        exit
    fi
    if [ "$sync_standalone_tree" = false ] && [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
        echo "Copying chipcode content"
        for token in ${software_image_combo}
        do
          temp_si_chipcode_path=$(xmlstarlet sel -T -t -m "//snap_release/image[@software_image='$token']" -v "@si_chipcode_path" -nl $snap_release_xml)
          echo "Copying $token chipcode content"
	  if [[ "$token" =~ "LE.UM" ]] || [[ "$token" =~ "LE.PRODUCT" ]] || [[ "$token" =~ "LE.FRAMEWORK" ]]; then
            cp -r $nhprop_chipcode_path/$temp_si_chipcode_path/* $tree_dest_path/
	  elif [[ $token =~ "KERNEL" ]]; then
	    mkdir -p  $tree_dest_path/$KERNEL_PLATFORM_PATH
            cp -r $nhprop_chipcode_path/$temp_si_chipcode_path/* $tree_dest_path/$KERNEL_PLATFORM_PATH/
          else
	    mkdir -p $tree_dest_path/$PROP_PATH
            cp -r $nhprop_chipcode_path/$temp_si_chipcode_path/* $tree_dest_path/
	    if [[ $token =~ "VENDOR" ]]; then
	          if [ -d "$tree_dest_path/$PROP_PATH/prebuilt_HY11" ];then
		    cp -f $tree_dest_path/$PROP_PATH/prebuilt_HY11/Android.mk $tree_dest_path/$PROP_PATH/prebuilt_HY11/mAndroid.mk || true
		  fi
	          if [ -d "$tree_dest_path/$PROP_PATH/prebuilt_HY22" ];then
		    cp -f $tree_dest_path/$PROP_PATH/prebuilt_HY22/Android.mk $tree_dest_path/$PROP_PATH/prebuilt_HY22/mAndroid.mk || true
		  fi
            fi
	  fi
        done
        echo "chipcode content copied"
	if [ -f "$tree_dest_path/$PROP_PATH/prebuilt_HY11/mAndroid.mk" ];then
		mv $tree_dest_path/$PROP_PATH/prebuilt_HY11/mAndroid.mk $tree_dest_path/$PROP_PATH/prebuilt_HY11/Android.mk || true
	fi
	if [ -f "$tree_dest_path/$PROP_PATH/prebuilt_HY22/mAndroid.mk" ];then
		mv $tree_dest_path/$PROP_PATH/prebuilt_HY22/mAndroid.mk $tree_dest_path/$PROP_PATH/prebuilt_HY22/Android.mk || true
	fi
        elif [ "$sync_standalone_tree" = true ] && [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
        echo "standalone tree Copying chipcode content"
        temp_si_chipcode_path=$(xmlstarlet sel -T -t -m "//snap_release/image[@software_image='$tree_type']" -v "@si_chipcode_path" -nl $snap_release_xml)
	echo "temp_si_chipcode_path = $temp_si_chipcode_path"
        echo "Copying $tree_type chipcode content"
	if [[ "$tree_type" =~ "LE.UM" ]] || [[ "$tree_type" =~ "LE.PRODUCT" ]] || [[ "$tree_type" =~ "LE.FRAMEWORK" ]]; then
          cp -r $nhprop_chipcode_path/$temp_si_chipcode_path/* $tree_dest_path/
	elif [[ $tree_type =~ "KERNEL" ]]; then
	  mkdir -p  $tree_dest_path/$KERNEL_PLATFORM_PATH
          cp -r $nhprop_chipcode_path/$temp_si_chipcode_path/* $tree_dest_path/$KERNEL_PLATFORM_PATH/
        else
	  mkdir -p $tree_dest_path/$PROP_PATH
          cp -r $nhprop_chipcode_path/$temp_si_chipcode_path/* $tree_dest_path/
	  if [[ $tree_type =~ "VENDOR" ]]; then
	          if [ -d "$tree_dest_path/$PROP_PATH/prebuilt_HY11" ];then
		    cp -f $tree_dest_path/$PROP_PATH/prebuilt_HY11/Android.mk $tree_dest_path/$PROP_PATH/prebuilt_HY11/mAndroid.mk || true
		  fi
	          if [ -d "$tree_dest_path/$PROP_PATH/prebuilt_HY22" ];then
		    cp -f $tree_dest_path/$PROP_PATH/prebuilt_HY22/Android.mk $tree_dest_path/$PROP_PATH/prebuilt_HY22/mAndroid.mk || true
		  fi
          fi
	fi
        echo "chipcode content copied"
	if [ -f "$tree_dest_path/$PROP_PATH/prebuilt_HY11/mAndroid.mk" ];then
		mv $tree_dest_path/$PROP_PATH/prebuilt_HY11/mAndroid.mk $tree_dest_path/$PROP_PATH/prebuilt_HY11/Android.mk || true
	fi
	if [ -f "$tree_dest_path/$PROP_PATH/prebuilt_HY22/mAndroid.mk" ];then
		mv $tree_dest_path/$PROP_PATH/prebuilt_HY22/mAndroid.mk $tree_dest_path/$PROP_PATH/prebuilt_HY22/Android.mk || true
	fi
    fi
    echo "tree $snap_release_tree_type sync done"
    exit
  fi
  echo "Using V2 sync method"
  OPTIONS=$(getopt  --long $INPUT_V2_OPTS -o '' -- "$@")
  eval set -- "$OPTIONS"

  while true ; do
    case "$1" in
       --workspace_path ) tree_dest_path="$2"; shift 2;;
       --image_type ) image_type="$2"; shift 2;;
       --tree_type ) tree_type=$2; shift 2;;
       --prop_opt ) prop_dest=$2; shift 2;;
       --common_oss_url )
         common_oss_url=$2
         vendor_oss_url=$2
         qssi_oss_url=$2
         kernel_oss_url=$2
         display_oss_url=$2
         camera_oss_url=$2
         video_oss_url=$2
         audio_oss_url=$2
         sensor_oss_url=$2
         xr_oss_url=$2
         cv_oss_url=$2
         graphics_oss_url=$2
         le_oss_url=$2
          shift 2;;
       --common_chipcode_hf_server )
         common_chipcode_hf_server=$2
         vendor_chipcode_hf_server=$2
         qssi_chipcode_hf_server=$2
         kernel_chipcode_hf_server=$2
         display_chipcode_hf_server=$2
         camera_chipcode_hf_server=$2
         video_chipcode_hf_server=$2
         audio_chipcode_hf_server=$2
         sensor_chipcode_hf_server=$2
         xr_chipcode_hf_server=$2
         cv_chipcode_hf_server=$2
         graphics_chipcode_hf_server=$2
          shift 2;;
       --vendor_oss_url ) vendor_oss_url=$2; shift 2;;
       --vendor_oss_manifest_git ) vendor_oss_manifest_git=$2; shift 2;;
       --grease_server ) grease_server=$2;shift 2;;
       --grease_userid ) grease_userid=$2; shift 2;;
       --grease_pass ) grease_pass=$2; shift 2;;
       --vendor_au_tag ) vendor_au_tag=$2; shift 2;;
       --vendor_grease_branch ) vendor_grease_branch=$2;shift 2;;
       --vendor_chipcode_path ) vendor_chipcode_path=$2; shift 2;;
       --vendor_chipcode_hf_server ) vendor_chipcode_hf_server=$2; shift 2;;
       --vendor_chipcode_hf_manifest_git ) vendor_chipcode_hf_manifest_git=$2; shift 2;;
       --vendor_chipcode_hf_manifest_branch ) vendor_chipcode_hf_manifest_branch=$2; shift 2;;
       --qssi_oss_url ) qssi_oss_url=$2; shift 2;;
       --qssi_oss_manifest_git ) qssi_oss_manifest_git=$2; shift 2;;
       --qssi_au_tag ) qssi_au_tag=$2; shift 2;;
       --qssi_grease_branch ) qssi_grease_branch=$2;shift 2;;
       --qssi_chipcode_path ) qssi_chipcode_path=$2; shift 2;;
       --qssi_chipcode_hf_server ) qssi_chipcode_hf_server=$2; shift 2;;
       --qssi_chipcode_hf_manifest_git ) qssi_chipcode_hf_manifest_git=$2; shift 2;;
       --qssi_chipcode_hf_manifest_branch ) qssi_chipcode_hf_manifest_branch=$2; shift 2;;
       --kernel_oss_url ) kernel_oss_url=$2; shift 2;;
       --kernel_oss_manifest_git ) kernel_oss_manifest_git=$2; shift 2;;
       --kernel_au_tag ) kernel_au_tag=$2; shift 2;;
       --kernel_grease_branch ) kernel_grease_branch=$2;shift 2;;
       --kernel_chipcode_path ) kernel_chipcode_path=$2; shift 2;;
       --kernel_chipcode_hf_server ) kernel_chipcode_hf_server=$2; shift 2;;
       --kernel_chipcode_hf_manifest_git ) kernel_chipcode_hf_manifest_git=$2; shift 2;;
       --kernel_chipcode_hf_manifest_branch ) kernel_chipcode_hf_manifest_branch=$2; shift 2;;
       --display_oss_url ) display_oss_url=$2; shift 2;;
       --display_oss_manifest_git ) display_oss_manifest_git=$2; shift 2;;
       --display_oss_manifest_git ) display_oss_manifest_git=$2; shift 2;;
       --display_au_tag ) display_au_tag=$2; shift 2;;
       --display_grease_branch ) display_grease_branch=$2;shift 2;;
       --display_chipcode_path ) display_chipcode_path=$2; shift 2;;
       --display_chipcode_hf_server ) display_chipcode_hf_server=$2; shift 2;;
       --display_chipcode_hf_manifest_git ) display_chipcode_hf_manifest_git=$2; shift 2;;
       --display_chipcode_hf_manifest_branch ) display_chipcode_hf_manifest_branch=$2; shift 2;;
       --camera_oss_url ) camera_oss_url=$2; shift 2;;
       --camera_oss_manifest_git ) camera_oss_manifest_git=$2; shift 2;;
       --camera_oss_manifest_git ) camera_oss_manifest_git=$2; shift 2;;
       --camera_au_tag ) camera_au_tag=$2; shift 2;;
       --camera_grease_branch ) camera_grease_branch=$2;shift 2;;
       --camera_chipcode_path ) camera_chipcode_path=$2; shift 2;;
       --camera_chipcode_hf_server ) camera_chipcode_hf_server=$2; shift 2;;
       --camera_chipcode_hf_manifest_git ) camera_chipcode_hf_manifest_git=$2; shift 2;;
       --camera_chipcode_hf_manifest_branch ) camera_chipcode_hf_manifest_branch=$2; shift 2;;
       --video_oss_url ) video_oss_url=$2; shift 2;;
       --video_oss_manifest_git ) video_oss_manifest_git=$2; shift 2;;
       --video_oss_manifest_git ) video_oss_manifest_git=$2; shift 2;;
       --video_au_tag ) video_au_tag=$2; shift 2;;
       --video_grease_branch ) video_grease_branch=$2;shift 2;;
       --video_chipcode_path ) video_chipcode_path=$2; shift 2;;
       --video_chipcode_hf_server ) video_chipcode_hf_server=$2; shift 2;;
       --video_chipcode_hf_manifest_git ) video_chipcode_hf_manifest_git=$2; shift 2;;
       --video_chipcode_hf_manifest_branch ) video_chipcode_hf_manifest_branch=$2; shift 2;;
       --audio_oss_url ) audio_oss_url=$2; shift 2;;
       --audio_oss_manifest_git ) audio_oss_manifest_git=$2; shift 2;;
       --audio_oss_manifest_git ) audio_oss_manifest_git=$2; shift 2;;
       --audio_au_tag ) audio_au_tag=$2; shift 2;;
       --audio_grease_branch ) audio_grease_branch=$2;shift 2;;
       --audio_chipcode_path ) audio_chipcode_path=$2; shift 2;;
       --audio_chipcode_hf_server ) audio_chipcode_hf_server=$2; shift 2;;
       --audio_chipcode_hf_manifest_git ) audio_chipcode_hf_manifest_git=$2; shift 2;;
       --audio_chipcode_hf_manifest_branch ) audio_chipcode_hf_manifest_branch=$2; shift 2;;
       --sensor_oss_url ) sensor_oss_url=$2; shift 2;;
       --sensor_oss_manifest_git ) sensor_oss_manifest_git=$2; shift 2;;
       --sensor_oss_manifest_git ) sensor_oss_manifest_git=$2; shift 2;;
       --sensor_au_tag ) sensor_au_tag=$2; shift 2;;
       --sensor_grease_branch ) sensor_grease_branch=$2;shift 2;;
       --sensor_chipcode_path ) sensor_chipcode_path=$2; shift 2;;
       --sensor_chipcode_hf_server ) sensor_chipcode_hf_server=$2; shift 2;;
       --sensor_chipcode_hf_manifest_git ) sensor_chipcode_hf_manifest_git=$2; shift 2;;
       --sensor_chipcode_hf_manifest_branch ) sensor_chipcode_hf_manifest_branch=$2; shift 2;;
       --xr_oss_url ) xr_oss_url=$2; shift 2;;
       --xr_oss_manifest_git ) xr_oss_manifest_git=$2; shift 2;;
       --xr_oss_manifest_git ) xr_oss_manifest_git=$2; shift 2;;
       --xr_au_tag ) xr_au_tag=$2; shift 2;;
       --xr_grease_branch ) xr_grease_branch=$2;shift 2;;
       --xr_chipcode_path ) xr_chipcode_path=$2; shift 2;;
       --xr_chipcode_hf_server ) xr_chipcode_hf_server=$2; shift 2;;
       --xr_chipcode_hf_manifest_git ) xr_chipcode_hf_manifest_git=$2; shift 2;;
       --xr_chipcode_hf_manifest_branch ) xr_chipcode_hf_manifest_branch=$2; shift 2;;
       --cv_oss_url ) cv_oss_url=$2; shift 2;;
       --cv_oss_manifest_git ) cv_oss_manifest_git=$2; shift 2;;
       --cv_oss_manifest_git ) cv_oss_manifest_git=$2; shift 2;;
       --cv_au_tag ) cv_au_tag=$2; shift 2;;
       --cv_grease_branch ) cv_grease_branch=$2;shift 2;;
       --cv_chipcode_path ) cv_chipcode_path=$2; shift 2;;
       --cv_chipcode_hf_server ) cv_chipcode_hf_server=$2; shift 2;;
       --cv_chipcode_hf_manifest_git ) cv_chipcode_hf_manifest_git=$2; shift 2;;
       --cv_chipcode_hf_manifest_branch ) cv_chipcode_hf_manifest_branch=$2; shift 2;;
       --graphics_oss_url ) graphics_oss_url=$2; shift 2;;
       --graphics_oss_manifest_git ) graphics_oss_manifest_git=$2; shift 2;;
       --graphics_oss_manifest_git ) graphics_oss_manifest_git=$2; shift 2;;
       --graphics_au_tag ) graphics_au_tag=$2; shift 2;;
       --graphics_grease_branch ) graphics_grease_branch=$2;shift 2;;
       --graphics_chipcode_path ) graphics_chipcode_path=$2; shift 2;;
       --graphics_chipcode_hf_server ) graphics_chipcode_hf_server=$2; shift 2;;
       --graphics_chipcode_hf_manifest_git ) graphics_chipcode_hf_manifest_git=$2; shift 2;;
       --graphics_chipcode_hf_manifest_branch ) graphics_chipcode_hf_manifest_branch=$2; shift 2;;
       --le_oss_url ) le_oss_url=$2; shift 2;;
       --le_oss_manifest_git ) le_oss_manifest_git=$2; shift 2;;
       --le_crmid ) le_crmid=$2; shift 2;;
       --le_chipcode_path ) le_chipcode_path=$2; shift 2;;
       --repo_url ) repo_url=$2; shift 2;;
       --repo_branch ) repo_branch=$2; shift 2;;
       --groups ) groups=$2; shift 2;;
       --shallow_clone ) shallow_clone=$2; shift 2;;
       --jobs ) jobs=$2; shift 2;;
       --optional_sync_arg ) optional_sync_arg=$OPTIONAL_SYNC_ARG; shift;;
       -- ) shift; break;;
       *)echo "Error!!!";exit 1;;
    esac
  done
fi

echo "shallow_clone = $shallow_clone"
echo "common_oss_url = $common_oss_url"
echo "vendor_oss_url = $vendor_oss_url"
echo "qssi_oss_url = $qssi_oss_url"
echo "kernel_oss_url = $kernel_oss_url"
echo "le_oss_url = $le_oss_url"

echo "common_chipcode_hf_server = $common_chipcode_hf_server"
echo "vendor_chipcode_hf_server = $vendor_chipcode_hf_server"
echo "qssi_chipcode_hf_server = $qssi_chipcode_hf_server"
echo "kernel_chipcode_hf_server = $kernel_chipcode_hf_server"
echo "display_chipcode_hf_server = $display_chipcode_hf_server"
echo "video_chipcode_hf_server = $video_chipcode_hf_server"
echo "audio_chipcode_hf_server = $audio_chipcode_hf_server"
echo "sensor_chipcode_hf_server = $sensor_chipcode_hf_server"
echo "xr_chipcode_hf_server = $xr_chipcode_hf_server"
echo "cv_chipcode_hf_server = $cv_chipcode_hf_server"
echo "graphics_chipcode_hf_server = $graphics_chipcode_hf_server"
echo "camera_chipcode_hf_server = $camera_chipcode_hf_server"

if [ ! -z "$optional_sync_arg" ]; then
  echo "optional_sync_arguments = $optional_sync_arg"
fi


#If image name is not specified, treat default as LA image
if [ -z "$image_type" ]; then
  echo "Choosing default option: $tree_type sync"
  image_type=$LA_IMAGE
fi

if [ -z "$qssi_oss_url" ]; then
  qssi_oss_url="https://git.codelinaro.org"
fi

if [ -z "$vendor_oss_url" ]; then
  vendor_oss_url="https://git.codelinaro.org"
fi

if [ -z "$repo_branch" ]; then
  repo_branch="clo-stable"
fi

if [ -z "$repo_url" ]; then
  repo_url="https://git.codelinaro.org/clo/la/tools/repo.git"
fi

if [ -z "$jobs" ]; then
  echo "No input for jobs, defaulting to 8"
  jobs=8
fi

if [ "$image_type" == $LE_IMAGE ]; then
  echo "Initiating sync process for LE image"

  ROOT_PWD=$tree_dest_path
  if [ -z "$le_chipcode_path" ]; then
    echo "Pls pass LE image chipcode path Eg: /test/path/to/le_chipcode_path/LE.UM.4.3.1.r1-03200-genericarmv8-64.0-1/apps_proc/"
    exit 1
  fi

  if [ -z "$le_crmid" ]; then
    echo "Pls pass LE CRM ID: Eg: LE.UM.4.3.1.r1-03100-genericarmv8-64.0"
    exit 1
  fi

  if [ ! -d "$le_chipcode_path" ]
  then
    echo "LE Image chipcode PATH le_chipcode_path DOES NOT exists"
    exit 1
  fi

  mkdir -p $ROOT_PWD
  rm -rf $ROOT_PWD/$LE_APPS_PROC_PATH/.repo/local_manifests
  rm -rf $ROOT_PWD/$LE_APPS_PROC_PATH/$LE_OSS_DIR_NAME $ROOT_PWD/$LE_APPS_PROC_PATH/$KERNEL_OSS_DIR_NAME
  mkdir -p $ROOT_PWD/$LE_APPS_PROC_PATH/$REPO_BACKUP_PATH

  if [ -d $ROOT_PWD/$LE_APPS_PROC_PATH/.repo ]; then
    pushd $ROOT_PWD/$LE_APPS_PROC_PATH/$REPO_MANIFESTS
    git reset --hard
    pushd $ROOT_PWD/$LE_APPS_PROC_PATH
  fi

  if [ -d $ROOT_PWD/$LE_APPS_PROC_PATH/$REPO_BACKUP_PATH/.repo ]; then
    pushd $ROOT_PWD/$LE_APPS_PROC_PATH/$REPO_BACKUP_PATH/$REPO_MANIFESTS
    git reset --hard
    mv $ROOT_PWD/$LE_APPS_PROC_PATH/$REPO_BACKUP_PATH/.repo $ROOT_PWD/
    pushd $ROOT_PWD/$LE_APPS_PROC_PATH
  fi

  if [ "$tree_type" == $LE_STANDALONE ] || [ "$tree_type" == $LE_COMBINED ]
  then

    echo "Copying LE chipcode path folder(apps_proc)"
    cp -r $le_chipcode_path $ROOT_PWD
    echo "LE chipcode content(apps_proc folder) copied"

    pushd $ROOT_PWD/$LE_APPS_PROC_PATH
    mkdir -p $ROOT_PWD/$LE_APPS_PROC_PATH/$LE_OSS_DIR_NAME
    mkdir -p $ROOT_PWD/$LE_APPS_PROC_PATH/$KERNEL_OSS_DIR_NAME
    echo " repo init LE manifest"
    ARGS="-u $le_oss_url/$le_oss_manifest_git.git -b release -m $le_crmid.xml --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "LE manifest repo init done"

    le_oss_manifest=$ROOT_PWD/$LE_APPS_PROC_PATH/.repo/manifests/$le_crmid.xml
    qremote="cafle"
    echo "LE CAF manifest = $kernel_oss_manifest, qremote = $qremote"
    xml1=$(cat "$le_oss_manifest" | remove_defaults)

    echo "Getting remotes"
    m1remotes=($(cat "$le_oss_manifest" |\
             xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
             ))
    echo "replacing remotes"
    for m1remote in "${m1remotes[@]}"; do
      echo "m1remote = "$m1remote""
      if [ "$m1remote" == "caf" ]; then
        xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      fi
      done
      rm -rf $ROOT_PWD/$LE_APPS_PROC_PATH/lecafmanifest.xml
      {
        echo '<?xml version="1.0" encoding="UTF-8"?>
          <manifest>'

        echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
        echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

        echo '</manifest>'
      } |  xmlstarlet fo >> $ROOT_PWD/$LE_APPS_PROC_PATH/lecafmanifest.xml

    echo " LE caf manifest processing is done"
    mv $ROOT_PWD/$LE_APPS_PROC_PATH/lecafmanifest.xml $ROOT_PWD/$LE_APPS_PROC_PATH/.repo/manifests/$le_crmid.xml


    if [ "$tree_type" == $LE_COMBINED ]; then
      if [ -d $ROOT_PWD/$LE_APPS_PROC_PATH/.repo ]; then
        rm -rf $ROOT_PWD/$LE_APPS_PROC_PATH/$REPO_BACKUP_PATH/.repo
        mv $ROOT_PWD/$LE_APPS_PROC_PATH/.repo $ROOT_PWD/$LE_APPS_PROC_PATH/$REPO_BACKUP_PATH/
      fi

      pushd $ROOT_PWD/$LE_APPS_PROC_PATH/$KERNEL_OSS_DIR_NAME

      if grep -qF "$kernel_au_fnd_str" $ROOT_PWD/$LE_APPS_PROC_PATH/$REPO_BACKUP_PATH/.repo/manifests/$le_crmid.xml;then
        echo "Found KERNEL SI AU TAG in LE Manifest"
        kernel_au_tag=$(xmlstarlet sel -T -t -m  "//refs/image[@project='kernelplatform/manifest']" -v "@tag" -nl $ROOT_PWD/$LE_APPS_PROC_PATH/$REPO_BACKUP_PATH/.repo/manifests/$le_crmid.xml)
        echo "Kernel AU TAG = $kernel_au_tag"
      else
        echo "Didn't find KERNEL SI AU TAG in LE  manifest"
        if [ -z "$kernel_au_tag" ]; then
          echo "Not able to find a way to fetch kernel AU TAG, exiting"
          exit 1
        fi
      fi

      echo "kernel AU TAG = $kernel_au_tag"
      kernel_oss_au_xml="caf_$kernel_au_tag.xml"
      [ "$kernel_oss_url" == "https://git.codelinaro.org" ] && kernel_oss_au_xml="$kernel_au_tag.xml"
      echo " repo init KERNEL PLATFORM manifest"
      ARGS="-u $kernel_oss_url/$kernel_oss_manifest_git.git -b release -m $kernel_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
      if $shallow_clone; then
         ARGS="--depth=1 $ARGS"
      fi
      echo "repo init $ARGS"
      repo init $ARGS
      echo "KERNEL PLATFORM  manifest repo init done"

      if [ -d $ROOT_PWD/$LE_APPS_PROC_PATH/$REPO_BACKUP_PATH/.repo ];then
        mv $ROOT_PWD/$LE_APPS_PROC_PATH/$REPO_BACKUP_PATH/.repo $ROOT_PWD/$LE_APPS_PROC_PATH/
      fi

      mkdir -p $ROOT_PWD/$LE_APPS_PROC_PATH/.repo/local_manifests

      kernel_oss_manifest=$ROOT_PWD/$LE_APPS_PROC_PATH/$KERNEL_OSS_DIR_NAME/.repo/manifests/$kernel_oss_au_xml
      qremote="cafk"
      echo "kernel OSS manifest = $kernel_oss_manifest, qremote = $qremote"
      xml1=$(cat "$kernel_oss_manifest" | remove_defaults)

      echo "Getting remotes"
      m1remotes=($(cat "$kernel_oss_manifest" |\
               xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
               ))
      echo "replacing remotes"
      for m1remote in "${m1remotes[@]}"; do
        xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      done
      rm -rf $ROOT_PWD/$LE_APPS_PROC_PATH/kcafmanifest.xml
      {
        echo '<?xml version="1.0" encoding="UTF-8"?>
          <manifest>'

        echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
        echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

        echo '</manifest>'
      } |  xmlstarlet fo >> $ROOT_PWD/$LE_APPS_PROC_PATH/kcafmanifest.xml

      echo " kernel caf manifest processing is done"
      mv $ROOT_PWD/$LE_APPS_PROC_PATH/kcafmanifest.xml $ROOT_PWD/$LE_APPS_PROC_PATH/.repo/local_manifests/
      echo "copying kernel platform proprietary content"
      mkdir -p  $ROOT_PWD/$LE_APPS_PROC_PATH/$KERNEL_PLATFORM_PATH
      cp -r $kernel_chipcode_path/* $ROOT_PWD/$LE_APPS_PROC_PATH/$KERNEL_PLATFORM_PATH/
      echo "Completed copying kernel platform proprietary content"
    fi
    rm -rf $ROOT_PWD/$LE_APPS_PROC_PATH/$KERNEL_OSS_DIR_NAME
    rm -rf $ROOT_PWD/$LE_APPS_PROC_PATH/$LE_OSS_DIR_NAME
    echo "Tree sync initiated"
    pushd $ROOT_PWD/$LE_APPS_PROC_PATH
    sleep 2
    repo manifest > $COMBINED_MANIFEST
    echo "Combined Manifest generated"
    if [ ! -z "$optional_sync_arg" ]; then
      echo "repo sync -j${jobs} $optional_sync_arg"
      repo sync -j${jobs} $optional_sync_arg
    else
      echo "repo sync -j${jobs}"
      repo sync -j${jobs}
    fi
    if [ "$tree_type" == $LE_COMBINED ]; then
      le_ksrc_path=$(xmlstarlet sel -T -t -m  "//refs/image[@project='kernelplatform/manifest']" -v "@src_path" -nl $ROOT_PWD/$LE_APPS_PROC_PATH/$REPO_MANIFESTS/$le_crmid.xml)
      echo "le_ksrc_path=$le_ksrc_path"
      mkdir -p $ROOT_PWD/$LE_APPS_PROC_PATH/$le_ksrc_path
      mv $ROOT_PWD/$LE_APPS_PROC_PATH/$KERNEL_PLATFORM_PATH $ROOT_PWD/$LE_APPS_PROC_PATH/$le_ksrc_path/
    fi
    echo "repo sync successful"
  fi
  echo "Completed sync process for LE image, exiting"
  exit 0
fi

echo "Initiating sync process for LA image:$tree_type"

echo "tree_type = $tree_type"

if [ -z "$tree_type" ]; then
  if [ -z "$prop_dest" ]; then
    if [ -z "$tree_dest_path" ]; then
      echo "Falling back to legacy sync model"
      if [ -z "$vendor_oss_manifest_git" ]; then
        echo "legacy sync: public CAF"
        $PWD/sync_all.sh -a $vendor_au_tag -b $vendor_grease_branch -u $grease_userid -p $grease_pass -s $grease_server
      else
        echo "legacy sync: private CAF"
        $PWD/sync_all.sh -a $vendor_au_tag -b $vendor_grease_branch -u $grease_userid -p $grease_pass -s $grease_server -c $vendor_oss_manifest_git -l $vendor_oss_url
      fi
      exit 0
    fi
  fi
fi

echo "using enhanced AIM sync model"

if [ -z "$tree_type" ]; then
  echo "Please input tree type(vendor Tree(vt)/QSSI Tree(qt)/Single Tree(st))"
  usage
fi

if [ -z "$prop_dest" ]; then
  echo "Please input source code/prebuilt location(Grease(gr)/Chipcode(ch))"
  usage
fi

#Pick up the arguments based on Tree type
#check, if req is to sync QSSI SI STANDALONE Tree
if [ "$tree_type" == $QSSI_TREE_STANDALONE ]; then
  echo "Selected tree type is 'QSSI Tree'"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     if grease_args_check "qssi" "$grease_server" "$qssi_grease_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     if chipcode_args_check "qssi" "$qssi_chipcode_path"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
     if chipcode_hf_args_check "qssi" "$qssi_chipcode_hf_server" "$qssi_chipcode_hf_manifest_git" "$qssi_chipcode_hf_manifest_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if oss_args_check "qssi" "$qssi_oss_manifest_git" "$qssi_oss_url"
     then echo "success"
     else echo " failure"
  fi

fi

#Pick up the arguments based on Tree type
#check, if req is to sync KERNEL SI STANDALONE Tree
if [ "$tree_type" == $KERNEL_TREE_STANDALONE ]; then
  echo "Selected tree type is 'Kernel Platform Tree'"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     if grease_args_check "kernel" "$grease_server" "$kernel_grease_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     if chipcode_args_check "kernel" "$kernel_chipcode_path"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
     if chipcode_hf_args_check "kernel" "$kernel_chipcode_hf_server" "$kernel_chipcode_hf_manifest_git" "$kernel_chipcode_hf_manifest_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if oss_args_check "kernel" "$kernel_oss_manifest_git" "$kernel_oss_url"
     then echo "success"
     else echo " failure"
  fi

fi

#Pick up the arguments based on Tree type
#check, if req is to sync TECHPACK DISPLAY SI STANDALONE Tree
if [ "$tree_type" == $DISPLAY_TREE_STANDALONE ]; then
  echo "Selected tree type is 'TECHPACK DISPLAY Tree'"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     if grease_args_check "display" "$grease_server" "$display_grease_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     if chipcode_args_check "display" "$display_chipcode_path"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
     if chipcode_hf_args_check "display" "$display_chipcode_hf_server" "$display_chipcode_hf_manifest_git" "$display_chipcode_hf_manifest_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if oss_args_check "display" "$display_oss_manifest_git" "$display_oss_url"
     then echo "success"
     else echo " failure"
  fi

fi

#Pick up the arguments based on Tree type
#check, if req is to sync TECHPACK CAMERA SI STANDALONE Tree
if [ "$tree_type" == $CAMERA_TREE_STANDALONE ]; then
  echo "Selected tree type is 'TECHPACK CAMERA Tree'"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     if grease_args_check "camera" "$grease_server" "$camera_grease_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     if chipcode_args_check "camera" "$camera_chipcode_path"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
     if chipcode_hf_args_check "camera" "$camera_chipcode_hf_server" "$camera_chipcode_hf_manifest_git" "$camera_chipcode_hf_manifest_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if oss_args_check "camera" "$camera_oss_manifest_git" "$camera_oss_url"
     then echo "success"
     else echo " failure"
  fi

fi


#Pick up the arguments based on Tree type
#check, if req is to sync TECHPACK VIDEO SI STANDALONE Tree
if [ "$tree_type" == $VIDEO_TREE_STANDALONE ]; then
  echo "Selected tree type is 'TECHPACK VIDEO Tree'"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     if grease_args_check "video" "$grease_server" "$video_grease_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     if chipcode_args_check "video" "$video_chipcode_path"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
     if chipcode_hf_args_check "video" "$video_chipcode_hf_server" "$video_chipcode_hf_manifest_git" "$video_chipcode_hf_manifest_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if oss_args_check "video" "$video_oss_manifest_git" "$video_oss_url"
     then echo "success"
     else echo " failure"
  fi

fi

#Pick up the arguments based on Tree type
#check, if req is to sync TECHPACK SENSOR SI STANDALONE Tree
if [ "$tree_type" == $SENSOR_TREE_STANDALONE ]; then
  echo "Selected tree type is 'TECHPACK SENSOR Tree'"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     if grease_args_check "sensor" "$grease_server" "$sensor_grease_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     if chipcode_args_check "sensor" "$sensor_chipcode_path"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
     if chipcode_hf_args_check "sensor" "$sensor_chipcode_hf_server" "$sensor_chipcode_hf_manifest_git" "$sensor_chipcode_hf_manifest_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  #if oss_args_check "sensor" "$sensor_oss_manifest_git" "$sensor_oss_url"
  #   then echo "success"
  #   else echo " failure"
  #fi

fi

#Pick up the arguments based on Tree type
#check, if req is to sync TECHPACK XR SI STANDALONE Tree
if [ "$tree_type" == $XR_TREE_STANDALONE ]; then
  echo "Selected tree type is 'TECHPACK XR Tree'"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     if grease_args_check "xr" "$grease_server" "$xr_grease_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     if chipcode_args_check "xr" "$xr_chipcode_path"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
     if chipcode_hf_args_check "xr" "$xr_chipcode_hf_server" "$xr_chipcode_hf_manifest_git" "$xr_chipcode_hf_manifest_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  #if oss_args_check "xr" "$xr_oss_manifest_git" "$xr_oss_url"
  #   then echo "success"
  #   else echo " failure"
  #fi

fi

#Pick up the arguments based on Tree type
#check, if req is to sync TECHPACK CV SI STANDALONE Tree
if [ "$tree_type" == $CV_TREE_STANDALONE ]; then
  echo "Selected tree type is 'TECHPACK CV Tree'"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     if grease_args_check "cv" "$grease_server" "$cv_grease_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     if chipcode_args_check "cv" "$cv_chipcode_path"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
     if chipcode_hf_args_check "cv" "$cv_chipcode_hf_server" "cv_chipcode_hf_manifest_git" "$cv_chipcode_hf_manifest_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if oss_args_check "cv" "$cv_oss_manifest_git" "$cv_oss_url"
     then echo "success"
     else echo " failure"
  fi

fi

#Pick up the arguments based on Tree type
#check, if req is to sync TECHPACK GRAPHICS SI STANDALONE Tree
if [ "$tree_type" == $GRAPHICS_TREE_STANDALONE ]; then
  echo "Selected tree type is 'TECHPACK GRAPHICS Tree'"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     if grease_args_check "graphics" "$grease_server" "$graphics_grease_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     if chipcode_args_check "graphics" "$graphics_chipcode_path"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
     if chipcode_hf_args_check "graphics" "$graphics_chipcode_hf_server" "graphics_chipcode_hf_manifest_git" "$graphics_chipcode_hf_manifest_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if oss_args_check "graphics" "$graphics_oss_manifest_git" "$graphics_oss_url"
     then echo "success"
     else echo " failure"
  fi

fi

#Pick up the arguments based on Tree type
#check, if req is to sync TECHPACK AUDIO SI STANDALONE Tree
if [ "$tree_type" == $AUDIO_TREE_STANDALONE ]; then
  echo "Selected tree type is 'TECHPACK AUDIO Tree'"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     if grease_args_check "audio" "$grease_server" "$audio_grease_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     if chipcode_args_check "audio" "$audio_chipcode_path"
        then echo "success"
        else echo " failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
     if chipcode_hf_args_check "audio" "$audio_chipcode_hf_server" "$audio_chipcode_hf_manifest_git" "$audio_chipcode_hf_manifest_branch"
        then echo "success"
        else echo " failure"
     fi
  fi

  if oss_args_check "audio" "$audio_oss_manifest_git" "$audio_oss_url"
     then echo "success"
     else echo " failure"
  fi

fi

#check, if the req is to sync Vendor Tree
if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $VENDOR_TREE_STANDALONE ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]
then
  echo "Selected tree type is 'LA_COMBINED_TREE/LA_COMBINED_TREE_GROUPS/VENDOR_STANDALONE/LA_COMBINED_TECHPACK_TREE'"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
     if grease_args_check "vendor" "$grease_server" "$vendor_grease_branch"
     then
        echo "Vendor Grease args check Success"
        else echo "Vendor Grease args check failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE ]; then
     if chipcode_args_check "vendor" "$vendor_chipcode_path"
     then
        echo "Vendor Grease args check Success"
        else echo "Vendor Grease args check failure"
     fi
  fi

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
     if chipcode_hf_args_check "vendor" "$vendor_chipcode_hf_server" "$vendor_chipcode_hf_manifest_git" "$vendor_chipcode_hf_manifest_branch"
     then
        echo "Vendor Chipcode HF args check Success"
        else echo "Vendor Chipcode HF args check failure"
     fi
  fi

  if oss_args_check "vendor" "$vendor_oss_manifest_git" "$vendor_oss_url"
  then
     echo "Vendor OSS args check success"
     else echo "Vendor OSS args check failure"
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
if [ -z "$vendor_oss_manifest_git" ]; then
  vendor_oss_manifest_git=la/vendor/manifest
fi

if [ -z "$vendor_grease_manifest_git" ]; then
  vendor_grease_manifest_git=la/vendor/manifest
fi

if [ -z "$qssi_oss_manifest_git" ]; then
  qssi_oss_manifest_git=la/system/manifest
fi


if [ -z "$qssi_grease_manifest_git" ]; then
  qssi_grease_manifest_git=la/system/manifest
fi

if [ -z "$kernel_grease_manifest_git" ]; then
  kernel_grease_manifest_git=kernelplatform/manifest
fi

if [ -z "$kernel_oss_manifest_git" ]; then
  kernel_oss_manifest_git=kernelplatform/manifest
fi

if [ -z "$display_grease_manifest_git" ]; then
    display_grease_manifest_git=techpack/display/manifest
fi

if [ -z "$display_oss_manifest_git" ]; then
    display_oss_manifest_git=techpack/display/manifest
fi

if [ -z "$camera_grease_manifest_git" ]; then
    camera_grease_manifest_git=techpack/camera/manifest
fi

if [ -z "$camera_oss_manifest_git" ]; then
    camera_oss_manifest_git=techpack/camera/manifest
fi

if [ -z "$video_grease_manifest_git" ]; then
    video_grease_manifest_git=techpack/video/manifest
fi

if [ -z "$video_oss_manifest_git" ]; then
    video_oss_manifest_git=techpack/video/manifest
fi

if [ -z "$audio_grease_manifest_git" ]; then
    audio_grease_manifest_git=techpack/audio/manifest
fi

if [ -z "$audio_oss_manifest_git" ]; then
    audio_oss_manifest_git=techpack/audio/manifest
fi

if [ -z "$sensor_grease_manifest_git" ]; then
    sensor_grease_manifest_git=techpack/sensors/manifest
fi

if [ -z "$sensor_oss_manifest_git" ]; then
    sensor_oss_manifest_git=techpack/sensors/manifest
fi

if [ -z "$xr_grease_manifest_git" ]; then
    xr_grease_manifest_git=techpack/xr/manifest
fi

if [ -z "$xr_oss_manifest_git" ]; then
    xr_oss_manifest_git=techpack/xr/manifest
fi

if [ -z "$cv_grease_manifest_git" ]; then
    cv_grease_manifest_git=techpack/cv/manifest
fi

if [ -z "$cv_oss_manifest_git" ]; then
    cv_oss_manifest_git=techpack/cv/manifest
fi

if [ -z "$graphics_grease_manifest_git" ]; then
    graphics_grease_manifest_git=techpack/graphics/manifest
fi

if [ -z "$graphics_oss_manifest_git" ]; then
    graphics_oss_manifest_git=techpack/graphics/manifest
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
  if [ "$tree_type" == $QSSI_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]
    then
    if [ ! -d "$qssi_chipcode_path" ]
    then
      echo "QSSI SI chipcode PATH qssi_chipcode_path DOES NOT exists"
      exit 1
    fi
  fi
  if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $VENDOR_TREE_STANDALONE ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]
    then
    if [ ! -d "$vendor_chipcode_path" ]
      then
      echo "Vendor SI chipcode PATH vendor_chipcode_path DOES NOT exists"
      exit 1
    fi
  fi
fi


mkdir -p $ROOT_PWD
rm -rf $ROOT_PWD/.repo/local_manifests
rm -rf $ROOT_PWD/$QSSI_OSS_DIR_NAME $ROOT_PWD/$QSSI_PROP_DIR_NAME $ROOT_PWD/$VENDOR_OSS_DIR_NAME $ROOT_PWD/$VENDOR_PROP_DIR_NAME $ROOT_PWD/$KERNEL_OSS_DIR_NAME $ROOT_PWD/$KERNEL_PROP_DIR_NAME $ROOT_PWD/$DISPLAY_OSS_DIR_NAME $ROOT_PWD/$DISPLAY_PROP_DIR_NAME $ROOT_PWD/$AUDIO_OSS_DIR_NAME $ROOT_PWD/$AUDIO_PROP_DIR_NAME $ROOT_PWD/$CAMERA_OSS_DIR_NAME $ROOT_PWD/$CAMERA_PROP_DIR_NAME $ROOT_PWD/$VIDEO_OSS_DIR_NAME $ROOT_PWD/$VIDEO_PROP_DIR_NAME $ROOT_PWD/$XR_OSS_DIR_NAME $ROOT_PWD/$XR_PROP_DIR_NAME $ROOT_PWD/$SENSOR_OSS_DIR_NAME $ROOT_PWD/$SENSOR_PROP_DIR_NAME $ROOT_PWD/$CV_OSS_DIR_NAME $ROOT_PWD/$CV_PROP_DIR_NAME $ROOT_PWD/$GRAPHICS_OSS_DIR_NAME $ROOT_PWD/$GRAPHICS_PROP_DIR_NAME
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


#check, if the req is to sync QSSI standalone Tree
if [ "$tree_type" == $QSSI_TREE_STANDALONE ]; then

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    qssi_chipcode_hf_fetch_url="https://${qssi_chipcode_hf_server}/"
    qssi_chipcode_hf_mf_url="${qssi_chipcode_hf_fetch_url}${qssi_chipcode_hf_manifest_git}"
  else
    qssi_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
    qssi_grease_mf_url="${qssi_grease_fetch_url}${qssi_grease_manifest_git}"
  fi

  mkdir -p $ROOT_PWD/$QSSI_OSS_DIR_NAME
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
    mkdir -p $ROOT_PWD/$QSSI_PROP_DIR_NAME
  fi
  qssi_oss_au_xml="caf_${qssi_au_tag}.xml"
  [ "$qssi_oss_url" == "https://git.codelinaro.org" ] && qssi_oss_au_xml="${qssi_au_tag}.xml"
  pushd $ROOT_PWD
  echo " repo init QSSI OSS"
  ARGS="-u $qssi_oss_url/$qssi_oss_manifest_git.git -b release -m $qssi_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
  if $shallow_clone; then
     ARGS="--depth=1 $ARGS"
  fi
  echo "repo init $ARGS"
  repo init $ARGS
  echo "QSSI SI OSS repo init done"

  if [ -d $ROOT_PWD/.repo ]; then
    rm -rf $ROOT_PWD/$REPO_BACKUP_PATH/.repo
    mv $ROOT_PWD/.repo $ROOT_PWD/$REPO_BACKUP_PATH/
  fi

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    pushd $ROOT_PWD/$QSSI_PROP_DIR_NAME
    echo " repo init QSSI SI PROP"
    qssi_grease_mf_name="${qssi_au_tag}.xml"
    ARGS="--no-clone-bundle -u $qssi_grease_mf_url -m $qssi_grease_mf_name -b $qssi_grease_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "QSSI SI PROP repo init done"
  elif [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    pushd $ROOT_PWD/$QSSI_PROP_DIR_NAME
    echo " repo init QSSI SI PROP"
    qssi_chipcode_hf_mf_name="${qssi_au_tag}.xml"
    ARGS="--no-clone-bundle -u $qssi_chipcode_hf_mf_url -m $qssi_chipcode_hf_mf_name  -b $qssi_chipcode_hf_manifest_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "QSSI SI PROP repo init done"
  fi
  pushd $ROOT_PWD
fi

#check, if the req is to sync KERNEL SI standalone  Tree
if [ "$tree_type" == $KERNEL_TREE_STANDALONE ]; then
  found_kernel_au=true
  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    kernel_chipcode_hf_fetch_url="https://${kernel_chipcode_hf_server}/"
    kernel_chipcode_hf_mf_url="${kernel_chipcode_hf_fetch_url}${kernel_chipcode_hf_manifest_git}"
  else
    kernel_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
    kernel_grease_mf_url="${kernel_grease_fetch_url}${kernel_grease_manifest_git}"
  fi

  mkdir -p $ROOT_PWD/$KERNEL_OSS_DIR_NAME
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
    mkdir -p $ROOT_PWD/$KERNEL_PROP_DIR_NAME
  fi
  pushd $ROOT_PWD
  kernel_oss_au_xml="caf_$kernel_au_tag.xml"
  [ "$kernel_oss_url" == "https://git.codelinaro.org" ] && kernel_oss_au_xml="$kernel_au_tag.xml"
  echo " repo init KERNEL SI OSS"
  ARGS="-u $kernel_oss_url/$kernel_oss_manifest_git.git -b release -m $kernel_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
  if $shallow_clone; then
     ARGS="--depth=1 $ARGS"
  fi
  echo "repo init $ARGS"
  repo init $ARGS
  echo "KERNEL SI OSS repo init done"

if [ -d $ROOT_PWD/.repo ]; then
  rm -rf $ROOT_PWD/$REPO_BACKUP_PATH/.repo
  mv $ROOT_PWD/.repo $ROOT_PWD/$REPO_BACKUP_PATH/
fi

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    pushd $ROOT_PWD/$KERNEL_PROP_DIR_NAME
    echo " repo init KERNEL SI PROP"
    kernel_grease_mf_name="${kernel_au_tag}.xml"
    ARGS="--no-clone-bundle -u $kernel_grease_mf_url -m $kernel_grease_mf_name -b $kernel_grease_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "KERNEL SI PROP repo init done"
  elif [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    pushd $ROOT_PWD/$KERNEL_PROP_DIR_NAME
    echo " repo init KERNEL SI PROP"
    kernel_chipcode_hf_mf_name="${kernel_au_tag}.xml"
    ARGS="--no-clone-bundle -u $kernel_chipcode_hf_mf_url -m $kernel_chipcode_hf_mf_name -b $kernel_chipcode_hf_manifest_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "KERNEL SI PROP repo init done"

  fi
  pushd $ROOT_PWD
fi

#check, if the req is to sync TECHPACK DISPLAY  SI standalone  Tree
if [ "$tree_type" == $DISPLAY_TREE_STANDALONE ]; then
  found_display_au=true
  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    display_chipcode_hf_fetch_url="https://${display_chipcode_hf_server}/"
    display_chipcode_hf_mf_url="${display_chipcode_hf_fetch_url}${display_chipcode_hf_manifest_git}"
  else
    display_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
    display_grease_mf_url="${display_grease_fetch_url}${display_grease_manifest_git}"
  fi

  mkdir -p $ROOT_PWD/$DISPLAY_OSS_DIR_NAME
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
    mkdir -p $ROOT_PWD/$DISPLAY_PROP_DIR_NAME
  fi
  pushd $ROOT_PWD
  display_oss_au_xml="caf_$display_au_tag.xml"
  [ "$display_oss_url" == "https://git.codelinaro.org" ] && display_oss_au_xml="$display_au_tag.xml"
  echo " repo init TECHPACK DISPLAY SI OSS"
  ARGS="-c -u $display_oss_url/$display_oss_manifest_git.git -b release -m $display_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
  if $shallow_clone; then
     ARGS="--depth=1 $ARGS"
  fi
  echo "repo init $ARGS"
  repo init $ARGS
  echo "DISPLAY SI OSS repo init done"

if [ -d $ROOT_PWD/.repo ]; then
  rm -rf $ROOT_PWD/$REPO_BACKUP_PATH/.repo
  mv $ROOT_PWD/.repo $ROOT_PWD/$REPO_BACKUP_PATH/
fi

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    pushd $ROOT_PWD/$DISPLAY_PROP_DIR_NAME
    echo " repo init TECHPACK DISPLAY SI PROP"
    display_grease_mf_name="${display_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $display_grease_mf_url -m $display_grease_mf_name -b $display_grease_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK DISPLAY SI PROP repo init done"
  elif [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    pushd $ROOT_PWD/$DISPLAY_PROP_DIR_NAME
    echo " repo init TECHPACK DISPLAY SI PROP"
    display_chipcode_hf_mf_name="${display_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $display_chipcode_hf_mf_url -m $display_chipcode_hf_mf_name -b $display_chipcode_hf_manifest_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK DISPLAY SI PROP repo init done"

  fi
  pushd $ROOT_PWD
fi


#check, if the req is to sync TECHPACK CAMERA  SI standalone  Tree
if [ "$tree_type" == $CAMERA_TREE_STANDALONE ]; then
  found_camera_au=true
  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    camera_chipcode_hf_fetch_url="https://${camera_chipcode_hf_server}/"
    camera_chipcode_hf_mf_url="${camera_chipcode_hf_fetch_url}${camera_chipcode_hf_manifest_git}"
  else
    camera_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
    camera_grease_mf_url="${camera_grease_fetch_url}${camera_grease_manifest_git}"
  fi

  mkdir -p $ROOT_PWD/$CAMERA_OSS_DIR_NAME
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
    mkdir -p $ROOT_PWD/$CAMERA_PROP_DIR_NAME
  fi
  pushd $ROOT_PWD
  camera_oss_au_xml="caf_$camera_au_tag.xml"
  [ "$camera_oss_url" == "https://git.codelinaro.org" ] && camera_oss_au_xml="$camera_au_tag.xml"
  echo " repo init TECHPACK CAMERA SI OSS"
  ARGS="-c -u $camera_oss_url/$camera_oss_manifest_git.git -b release -m $camera_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
  if $shallow_clone; then
     ARGS="--depth=1 $ARGS"
  fi
  echo "repo init $ARGS"
  repo init $ARGS
  echo "CAMERA SI OSS repo init done"

if [ -d $ROOT_PWD/.repo ]; then
  rm -rf $ROOT_PWD/$REPO_BACKUP_PATH/.repo
  mv $ROOT_PWD/.repo $ROOT_PWD/$REPO_BACKUP_PATH/
fi

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    pushd $ROOT_PWD/$CAMERA_PROP_DIR_NAME
    echo " repo init TECHPACK CAMERA SI PROP"
    camera_grease_mf_name="${camera_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $camera_grease_mf_url -m $camera_grease_mf_name -b $camera_grease_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK CAMERA SI PROP repo init done"
  elif [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    pushd $ROOT_PWD/$CAMERA_PROP_DIR_NAME
    echo " repo init TECHPACK CAMERA SI PROP"
    camera_chipcode_hf_mf_name="${camera_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $camera_chipcode_hf_mf_url -m $camera_chipcode_hf_mf_name -b $camera_chipcode_hf_manifest_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK CAMERA SI PROP repo init done"

  fi
  pushd $ROOT_PWD
fi


#check, if the req is to sync TECHPACK VIDEO  SI standalone  Tree
if [ "$tree_type" == $VIDEO_TREE_STANDALONE ]; then
  found_video_au=true
  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    video_chipcode_hf_fetch_url="https://${video_chipcode_hf_server}/"
    video_chipcode_hf_mf_url="${video_chipcode_hf_fetch_url}${video_chipcode_hf_manifest_git}"
  else
    video_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
    video_grease_mf_url="${video_grease_fetch_url}${video_grease_manifest_git}"
  fi

  mkdir -p $ROOT_PWD/$VIDEO_OSS_DIR_NAME
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
    mkdir -p $ROOT_PWD/$VIDEO_PROP_DIR_NAME
  fi
  pushd $ROOT_PWD
  video_oss_au_xml="caf_$video_au_tag.xml"
  [ "$video_oss_url" == "https://git.codelinaro.org" ] && video_oss_au_xml="$video_au_tag.xml"
  echo " repo init TECHPACK VIDEO SI OSS"
  ARGS="-c -u $video_oss_url/$video_oss_manifest_git.git -b release -m $video_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
  if $shallow_clone; then
     ARGS="--depth=1 $ARGS"
  fi
  echo "repo init $ARGS"
  repo init $ARGS
  echo "VIDEO SI OSS repo init done"

if [ -d $ROOT_PWD/.repo ]; then
  rm -rf $ROOT_PWD/$REPO_BACKUP_PATH/.repo
  mv $ROOT_PWD/.repo $ROOT_PWD/$REPO_BACKUP_PATH/
fi

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    pushd $ROOT_PWD/$VIDEO_PROP_DIR_NAME
    echo " repo init TECHPACK VIDEO SI PROP"
    video_grease_mf_name="${video_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $video_grease_mf_url -m $video_grease_mf_name -b $video_grease_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK VIDEO SI PROP repo init done"
  elif [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    pushd $ROOT_PWD/$VIDEO_PROP_DIR_NAME
    echo " repo init TECHPACK VIDEO SI PROP"
    video_chipcode_hf_mf_name="${video_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $video_chipcode_hf_mf_url -m $video_chipcode_hf_mf_name -b $video_chipcode_hf_manifest_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK VIDEO SI PROP repo init done"

  fi
  pushd $ROOT_PWD
fi


#check, if the req is to sync TECHPACK AUDIO  SI standalone  Tree
if [ "$tree_type" == $AUDIO_TREE_STANDALONE ]; then
  found_audio_au=true
  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    audio_chipcode_hf_fetch_url="https://${audio_chipcode_hf_server}/"
    audio_chipcode_hf_mf_url="${audio_chipcode_hf_fetch_url}${audio_chipcode_hf_manifest_git}"
  else
    audio_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
    audio_grease_mf_url="${audio_grease_fetch_url}${audio_grease_manifest_git}"
  fi

  mkdir -p $ROOT_PWD/$AUDIO_OSS_DIR_NAME
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
    mkdir -p $ROOT_PWD/$AUDIO_PROP_DIR_NAME
  fi
  pushd $ROOT_PWD
  audio_oss_au_xml="caf_$audio_au_tag.xml"
  [ "$audio_oss_url" == "https://git.codelinaro.org" ] && audio_oss_au_xml="$audio_au_tag.xml"
  echo " repo init TECHPACK AUDIO SI OSS"
  ARGS="-c -u $audio_oss_url/$audio_oss_manifest_git.git -b release -m $audio_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
  if $shallow_clone; then
     ARGS="--depth=1 $ARGS"
  fi
  echo "repo init $ARGS"
  repo init $ARGS
  echo "AUDIO SI OSS repo init done"

if [ -d $ROOT_PWD/.repo ]; then
  rm -rf $ROOT_PWD/$REPO_BACKUP_PATH/.repo
  mv $ROOT_PWD/.repo $ROOT_PWD/$REPO_BACKUP_PATH/
fi

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    pushd $ROOT_PWD/$AUDIO_PROP_DIR_NAME
    echo " repo init TECHPACK AUDIO SI PROP"
    audio_grease_mf_name="${audio_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $audio_grease_mf_url -m $audio_grease_mf_name -b $audio_grease_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK AUDIO SI PROP repo init done"
  elif [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    pushd $ROOT_PWD/$AUDIO_PROP_DIR_NAME
    echo " repo init TECHPACK AUDIO SI PROP"
    audio_chipcode_hf_mf_name="${audio_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $audio_chipcode_hf_mf_url -m $audio_chipcode_hf_mf_name -b $audio_chipcode_hf_manifest_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK AUDIO SI PROP repo init done"

  fi
  pushd $ROOT_PWD
fi

#check, if the req is to sync TECHPACK GRAPHICS  SI standalone  Tree
if [ "$tree_type" == $GRAPHICS_TREE_STANDALONE ]; then
  found_graphics_au=true
  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    graphics_chipcode_hf_fetch_url="https://${graphics_chipcode_hf_server}/"
    graphics_chipcode_hf_mf_url="${graphics_chipcode_hf_fetch_url}${graphics_chipcode_hf_manifest_git}"
  else
    graphics_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
    graphics_grease_mf_url="${graphics_grease_fetch_url}${graphics_grease_manifest_git}"
  fi

  mkdir -p $ROOT_PWD/$GRAPHICS_OSS_DIR_NAME
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
    mkdir -p $ROOT_PWD/$GRAPHICS_PROP_DIR_NAME
  fi
  pushd $ROOT_PWD
  graphics_oss_au_xml="caf_$graphics_au_tag.xml"
  [ "$graphics_oss_url" == "https://git.codelinaro.org" ] && graphics_oss_au_xml="$graphics_au_tag.xml"
  echo " repo init TECHPACK GRAPHICS SI OSS"
  ARGS="-c -u $graphics_oss_url/$graphics_oss_manifest_git.git -b release -m $graphics_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
  if $shallow_clone; then
     ARGS="--depth=1 $ARGS"
  fi
  echo "repo init $ARGS"
  repo init $ARGS
  echo "GRAPHICS SI OSS repo init done"

if [ -d $ROOT_PWD/.repo ]; then
  rm -rf $ROOT_PWD/$REPO_BACKUP_PATH/.repo
  mv $ROOT_PWD/.repo $ROOT_PWD/$REPO_BACKUP_PATH/
fi

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    pushd $ROOT_PWD/$GRAPHICS_PROP_DIR_NAME
    echo " repo init TECHPACK GRAPHICS SI PROP"
    graphics_grease_mf_name="${graphics_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $graphics_grease_mf_url -m $graphics_grease_mf_name -b $graphics_grease_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK GRAPHICS SI PROP repo init done"
  elif [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    pushd $ROOT_PWD/$GRAPHICS_PROP_DIR_NAME
    echo " repo init TECHPACK GRAPHICS SI PROP"
    graphics_chipcode_hf_mf_name="${graphics_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $graphics_chipcode_hf_mf_url -m $graphics_chipcode_hf_mf_name -b $graphics_chipcode_hf_manifest_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK GRAPHICS SI PROP repo init done"

  fi
  pushd $ROOT_PWD
fi

#check, if the req is to sync TECHPACK SENSOR  SI standalone  Tree
if [ "$tree_type" == $SENSOR_TREE_STANDALONE ]; then
  found_sensor_au=true
  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    sensor_chipcode_hf_fetch_url="https://${sensor_chipcode_hf_server}/"
    sensor_chipcode_hf_mf_url="${sensor_chipcode_hf_fetch_url}${sensor_chipcode_hf_manifest_git}"
  else
    sensor_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
    sensor_grease_mf_url="${sensor_grease_fetch_url}${sensor_grease_manifest_git}"
  fi

  mkdir -p $ROOT_PWD/$SENSOR_OSS_DIR_NAME
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
    mkdir -p $ROOT_PWD/$SENSOR_PROP_DIR_NAME
  fi
  pushd $ROOT_PWD
  #sensor_oss_au_xml="caf_$sensor_au_tag.xml"
  #[ "$sensor_oss_url" == "https://git.codelinaro.org" ] && sensor_oss_au_xml="$sensor_au_tag.xml"
  #echo " repo init TECHPACK SENSOR SI OSS"
  #ARGS="-c --no-clone-bundle -u $sensor_oss_url/$sensor_oss_manifest_git.git -b release -m $sensor_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
  #if $shallow_clone; then
  #    ARGS="--depth=1 $ARGS"
  #fi
  #echo "repo init $ARGS"
  #repo init $ARGS
  #echo "SENSOR SI OSS repo init done"

if [ -d $ROOT_PWD/.repo ]; then
  rm -rf $ROOT_PWD/$REPO_BACKUP_PATH/.repo
  mv $ROOT_PWD/.repo $ROOT_PWD/$REPO_BACKUP_PATH/
fi

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    pushd $ROOT_PWD/$SENSOR_PROP_DIR_NAME
    echo " repo init TECHPACK SENSOR SI PROP"
    sensor_grease_mf_name="${sensor_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $sensor_grease_mf_url -m $sensor_grease_mf_name -b $sensor_grease_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK SENSOR SI PROP repo init done"
  elif [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    pushd $ROOT_PWD/$SENSOR_PROP_DIR_NAME
    echo " repo init TECHPACK SENSOR SI PROP"
    sensor_chipcode_hf_mf_name="${sensor_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $sensor_chipcode_hf_mf_url -m $sensor_chipcode_hf_mf_name -b $sensor_chipcode_hf_manifest_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK SENSOR SI PROP repo init done"

  fi
  pushd $ROOT_PWD
fi

#check, if the req is to sync TECHPACK XR  SI standalone  Tree
if [ "$tree_type" == $XR_TREE_STANDALONE ]; then
  found_xr_au=true
  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    xr_chipcode_hf_fetch_url="https://${xr_chipcode_hf_server}/"
    xr_chipcode_hf_mf_url="${xr_chipcode_hf_fetch_url}${xr_chipcode_hf_manifest_git}"
  else
    xr_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
    xr_grease_mf_url="${xr_grease_fetch_url}${xr_grease_manifest_git}"
  fi

  mkdir -p $ROOT_PWD/$XR_OSS_DIR_NAME
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
    mkdir -p $ROOT_PWD/$XR_PROP_DIR_NAME
  fi
  pushd $ROOT_PWD
  #xr_oss_au_xml="caf_$xr_au_tag.xml"
  #[ "$xr_oss_url" == "https://git.codelinaro.org" ] && xr_oss_au_xml="$xr_au_tag.xml"
  #echo " repo init TECHPACK XR SI OSS"
  #ARGS="-c --no-clone-bundle -u $xr_oss_url/$xr_oss_manifest_git.git -b release -m $xr_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
  #if $shallow_clone; then
  #    ARGS="--depth=1 $ARGS"
  #fi
  #echo "repo init $ARGS"
  #repo init $ARGS
  #echo "XR SI OSS repo init done"

if [ -d $ROOT_PWD/.repo ]; then
  rm -rf $ROOT_PWD/$REPO_BACKUP_PATH/.repo
  mv $ROOT_PWD/.repo $ROOT_PWD/$REPO_BACKUP_PATH/
fi

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    pushd $ROOT_PWD/$XR_PROP_DIR_NAME
    echo " repo init TECHPACK XR SI PROP"
    xr_grease_mf_name="${xr_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $xr_grease_mf_url -m $xr_grease_mf_name -b $xr_grease_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK XR SI PROP repo init done"
  elif [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    pushd $ROOT_PWD/$XR_PROP_DIR_NAME
    echo " repo init TECHPACK XR SI PROP"
    xr_chipcode_hf_mf_name="${xr_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $xr_chipcode_hf_mf_url -m $xr_chipcode_hf_mf_name -b $xr_chipcode_hf_manifest_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK XR SI PROP repo init done"

  fi
  pushd $ROOT_PWD
fi

#check, if the req is to sync TECHPACK CV  SI standalone  Tree
if [ "$tree_type" == $CV_TREE_STANDALONE ]; then
  found_cv_au=true
  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    cv_chipcode_hf_fetch_url="https://${cv_chipcode_hf_server}/"
    cv_chipcode_hf_mf_url="${cv_chipcode_hf_fetch_url}${cv_chipcode_hf_manifest_git}"
  else
    cv_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
    cv_grease_mf_url="${cv_grease_fetch_url}${cv_grease_manifest_git}"
  fi

  mkdir -p $ROOT_PWD/$CV_OSS_DIR_NAME
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
    mkdir -p $ROOT_PWD/$CV_PROP_DIR_NAME
  fi
  pushd $ROOT_PWD
  cv_oss_au_xml="caf_$cv_au_tag.xml"
  [ "$cv_oss_url" == "https://git.codelinaro.org" ] && cv_oss_au_xml="$cv_au_tag.xml"
  echo " repo init TECHPACK CV SI OSS"
  ARGS="-c -u $cv_oss_url/$cv_oss_manifest_git.git -b release -m $cv_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
  if $shallow_clone; then
      ARGS="--depth=1 $ARGS"
  fi
  echo "repo init $ARGS"
  repo init $ARGS
  echo "CV SI OSS repo init done"

if [ -d $ROOT_PWD/.repo ]; then
  rm -rf $ROOT_PWD/$REPO_BACKUP_PATH/.repo
  mv $ROOT_PWD/.repo $ROOT_PWD/$REPO_BACKUP_PATH/
fi

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    pushd $ROOT_PWD/$CV_PROP_DIR_NAME
    echo " repo init TECHPACK CV SI PROP"
    cv_grease_mf_name="${cv_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $cv_grease_mf_url -m $cv_grease_mf_name -b $cv_grease_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK CV SI PROP repo init done"
  elif [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    pushd $ROOT_PWD/$CV_PROP_DIR_NAME
    echo " repo init TECHPACK CV SI PROP"
    cv_chipcode_hf_mf_name="${cv_au_tag}.xml"
    ARGS="-c --no-clone-bundle -u $cv_chipcode_hf_mf_url -m $cv_chipcode_hf_mf_name -b $cv_chipcode_hf_manifest_branch --repo-url=$repo_url --repo-branch=$repo_branch"
    if $shallow_clone; then
       ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "TECHPACK CV SI PROP repo init done"

  fi
  pushd $ROOT_PWD
fi

#check, if the req is to sync Vendor/Vendor standalone Tree
if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $VENDOR_TREE_STANDALONE ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]
then
  mkdir -p $ROOT_PWD/$QSSI_OSS_DIR_NAME
  mkdir -p $ROOT_PWD/$VENDOR_OSS_DIR_NAME
  mkdir -p $ROOT_PWD/$KERNEL_OSS_DIR_NAME
  mkdir -p $ROOT_PWD/$DISPLAY_OSS_DIR_NAME
  mkdir -p $ROOT_PWD/$CAMERA_OSS_DIR_NAME
  mkdir -p $ROOT_PWD/$VIDEO_OSS_DIR_NAME
  mkdir -p $ROOT_PWD/$AUDIO_OSS_DIR_NAME
  mkdir -p $ROOT_PWD/$SENSOR_OSS_DIR_NAME
  mkdir -p $ROOT_PWD/$XR_OSS_DIR_NAME
  mkdir -p $ROOT_PWD/$CV_OSS_DIR_NAME
  mkdir -p $ROOT_PWD/$GRAPHICS_OSS_DIR_NAME
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
    mkdir -p $ROOT_PWD/$VENDOR_PROP_DIR_NAME
    mkdir -p $ROOT_PWD/$QSSI_PROP_DIR_NAME
    mkdir -p $ROOT_PWD/$KERNEL_PROP_DIR_NAME
    mkdir -p $ROOT_PWD/$DISPLAY_PROP_DIR_NAME
    mkdir -p $ROOT_PWD/$CAMERA_PROP_DIR_NAME
    mkdir -p $ROOT_PWD/$VIDEO_PROP_DIR_NAME
    mkdir -p $ROOT_PWD/$AUDIO_PROP_DIR_NAME
    mkdir -p $ROOT_PWD/$SENSOR_PROP_DIR_NAME
    mkdir -p $ROOT_PWD/$XR_PROP_DIR_NAME
    mkdir -p $ROOT_PWD/$CV_PROP_DIR_NAME
    mkdir -p $ROOT_PWD/$GRAPHICS_PROP_DIR_NAME
  fi

  pushd $ROOT_PWD
  vendor_oss_au_xml="caf_$vendor_au_tag.xml"
  [ "$vendor_oss_url" == "https://git.codelinaro.org" ] && vendor_oss_au_xml="$vendor_au_tag.xml"
  echo " repo init Vendor SI OSS"
  ARGS="-u $vendor_oss_url/$vendor_oss_manifest_git.git -b release -m $vendor_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
  if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
    ARGS="$ARGS -g $groups"
  fi
  if $shallow_clone; then
     ARGS="--depth=1 $ARGS"
  fi
  echo "repo init $ARGS"
  repo init $ARGS
  echo "Vendor SI OSS init done"
  if [ -d $ROOT_PWD/.repo ]; then
    rm -rf $ROOT_PWD/$REPO_BACKUP_PATH/.repo
    mv $ROOT_PWD/.repo $ROOT_PWD/$REPO_BACKUP_PATH/
  fi
  if [ "$tree_type" != $LA_VENDOR_TECHPACK_TREE ]; then
    echo "Get QSSI AU_TAG"
    qssi_au_tag=$(xmlstarlet sel -T -t -m  "//refs/image[@project='la/system/manifest']" -v "@tag" -nl $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml)

    if grep -qF "$kernel_au_fnd_str" $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml;then
      echo "Found KERNEL SI AU TAG in vendor SI Manifest"
      kernel_au_tag=$(xmlstarlet sel -T -t -m  "//refs/image[@project='kernelplatform/manifest']" -v "@tag" -nl $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml)
      echo "Kernel AU TAG = $kernel_au_tag"
      found_kernel_au=true
    else
      echo "Didn't find KERNEL SI AU TAG in Vendor SI manifest"
    fi
    echo "found_kernel_au = $found_kernel_au"
  fi

if grep -qF "$display_au_fnd_str" $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml;then
    echo "Found TECHPACK DISPLAY SI AU TAG in vendor SI Manifest"
    display_au_tag=$(xmlstarlet sel -T -t -m  "//refs/image[@project='techpack/display/manifest']" -v "@tag" -nl $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml)
    echo "TECHPACK DISPLAY AU TAG = $display_au_tag"
    found_display_au=true
  else
    echo "Didn't find TECHPACK DISPLAY SI AU TAG in Vendor SI manifest"
  fi
  echo "found_TECHPACK_DISPLAY_au = $found_display_au"

if grep -qF "$camera_au_fnd_str" $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml;then
    echo "Found TECHPACK CAMERA SI AU TAG in vendor SI Manifest"
    camera_au_tag=$(xmlstarlet sel -T -t -m  "//refs/image[@project='techpack/camera/manifest']" -v "@tag" -nl $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml)
    echo "TECHPACK CAMERA AU TAG = $camera_au_tag"
    found_camera_au=true
  else
    echo "Didn't find TECHPACK CAMERA SI AU TAG in Vendor SI manifest"
  fi
  echo "found_TECHPACK_CAMERA_au = $found_camera_au"

if grep -qF "$audio_au_fnd_str" $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml;then
    echo "Found TECHPACK AUDIO SI AU TAG in vendor SI Manifest"
    audio_au_tag=$(xmlstarlet sel -T -t -m  "//refs/image[@project='techpack/audio/manifest']" -v "@tag" -nl $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml)
    echo "TECHPACK AUDIO AU TAG = $audio_au_tag"
    found_audio_au=true
  else
    echo "Didn't find TECHPACK AUDIO SI AU TAG in Vendor SI manifest"
  fi
  echo "found_TECHPACK_AUDIO_au = $found_audio_au"

if [ "$tree_type" = $LA_COMBINED_TREE_GROUPS ]; then
  #Disable sensor and xr code
  found_sensor_au=false
  found_xr_au=false
else
  #Enable sensor and xr code
  found_sensor_au=true
  found_xr_au=true
fi


#if grep -qF "$sensor_au_fnd_str" $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml;then
#    echo "Found TECHPACK SENSOR SI AU TAG in vendor SI Manifest"
#    sensor_au_tag=$(xmlstarlet sel -T -t -m  "//refs/image[@project='techpack/sensors/manifest']" -v "@tag" -nl $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml)
#    echo "TECHPACK SENSOR AU TAG = $sensor_au_tag"
#    found_sensor_au=true
#  else
#    echo "Didn't find TECHPACK SENSOR SI AU TAG in Vendor SI manifest"
#  fi
#  echo "found_TECHPACK_SENSOR_au = $found_sensor_au"

#if grep -qF "$xr_au_fnd_str" $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml;then
#    echo "Found TECHPACK XR SI AU TAG in vendor SI Manifest"
#    xr_au_tag=$(xmlstarlet sel -T -t -m  "//refs/image[@project='techpack/xr/manifest']" -v "@tag" -nl $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml)
#    echo "TECHPACK XR AU TAG = $xr_au_tag"
#    found_xr_au=true
#  else
#    echo "Didn't find TECHPACK XR SI AU TAG in Vendor SI manifest"
#  fi
#  echo "found_TECHPACK_XR_au = $found_xr_au"

if grep -qF "$cv_au_fnd_str" $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml;then
    echo "Found TECHPACK CV SI AU TAG in vendor SI Manifest"
    cv_au_tag=$(xmlstarlet sel -T -t -m  "//refs/image[@project='techpack/cv/manifest']" -v "@tag" -nl $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml)
    echo "TECHPACK CV AU TAG = $cv_au_tag"
    found_cv_au=true
  else
    echo "Didn't find TECHPACK CV SI AU TAG in Vendor SI manifest"
  fi
  echo "found_TECHPACK_CV_au = $found_cv_au"

if grep -qF "$graphics_au_fnd_str" $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml;then
    echo "Found TECHPACK GRAPHICS SI AU TAG in vendor SI Manifest"
    graphics_au_tag=$(xmlstarlet sel -T -t -m  "//refs/image[@project='techpack/graphics/manifest']" -v "@tag" -nl $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml)
    echo "TECHPACK GRAPHICS AU TAG = $graphics_au_tag"
    found_graphics_au=true
  else
    echo "Didn't find TECHPACK GRAPHICS SI AU TAG in Vendor SI manifest"
  fi
  echo "found_TECHPACK_GRAPHICS_au = $found_graphics_au"

if grep -qF "$video_au_fnd_str" $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml;then
    echo "Found TECHPACK VIDEO SI AU TAG in vendor SI Manifest"
    video_au_tag=$(xmlstarlet sel -T -t -m  "//refs/image[@project='techpack/video/manifest']" -v "@tag" -nl $ROOT_PWD/$REPO_BACKUP_PATH/.repo/manifests/$vendor_oss_au_xml)
    echo "TECHPACK VIDEO AU TAG = $video_au_tag"
    found_video_au=true
  else
    echo "Didn't find TECHPACK VIDEO SI AU TAG in Vendor SI manifest"
  fi

  echo "found_TECHPACK_VIDEO_au = $found_video_au"

  echo "QSSI_AU_TAG=$qssi_au_tag"

  if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
    vendor_chipcode_hf_fetch_url="https://${vendor_chipcode_hf_server}/"
    vendor_fetch_mf_url="${vendor_chipcode_hf_fetch_url}${vendor_chipcode_hf_manifest_git}"
    vendor_mf_branch="${vendor_chipcode_hf_manifest_branch}"
  else
    vendor_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
    vendor_fetch_mf_url="${vendor_grease_fetch_url}${vendor_grease_manifest_git}"
    vendor_mf_branch="${vendor_grease_branch}"
  fi

  if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]
  then
    if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ];then
      if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
        if chipcode_hf_args_check "qssi" "$qssi_chipcode_hf_server" "$qssi_chipcode_hf_manifest_git" "$qssi_chipcode_hf_manifest_branch"
        then
           qssi_chipcode_hf_fetch_url="https://${qssi_chipcode_hf_server}/"
           qssi_fetch_mf_url="${qssi_chipcode_hf_fetch_url}${qssi_chipcode_hf_manifest_git}"
           qssi_mf_branch="${qssi_chipcode_hf_manifest_branch}"
        fi
      else
        if grease_args_check "qssi" "$grease_server" "$qssi_grease_branch"
        then
           qssi_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
           qssi_fetch_mf_url="${qssi_grease_fetch_url}${qssi_grease_manifest_git}"
           qssi_mf_branch="${qssi_grease_branch}"
        fi
      fi
      echo "QSSI_AU_TAG before processing=$qssi_au_tag"

      if oss_args_check "qssi" "$qssi_oss_manifest_git" "$qssi_oss_url"
         then echo "success"
         else echo " failure"
      fi

      pushd $ROOT_PWD/$QSSI_OSS_DIR_NAME
      qssi_oss_au_xml="caf_$qssi_au_tag.xml"
      [ "$qssi_oss_url" == "https://git.codelinaro.org" ] && qssi_oss_au_xml="$qssi_au_tag.xml"
      echo " repo init QSSI OSS"
      ARGS="-u $qssi_oss_url/$qssi_oss_manifest_git.git -b release -m $qssi_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
      if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
        ARGS="$ARGS -g $groups"
      fi
      if $shallow_clone; then
        ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "QSSI OSS repo init done"

      if [ "$found_kernel_au" = true ] ; then
        if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
          if chipcode_hf_args_check "kernel" "$kernel_chipcode_hf_server" "$kernel_chipcode_hf_manifest_git" "$kernel_chipcode_hf_manifest_branch"
          then
             kernel_chipcode_hf_fetch_url="https://${kernel_chipcode_hf_server}/"
             kernel_fetch_mf_url="${kernel_chipcode_hf_fetch_url}${kernel_chipcode_hf_manifest_git}"
             kernel_mf_branch="${kernel_chipcode_hf_manifest_branch}"
          fi
        else
          if grease_args_check "kernel" "$grease_server" "$kernel_grease_branch"
          then
             kernel_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
             kernel_fetch_mf_url="${kernel_grease_fetch_url}${kernel_grease_manifest_git}"
             kernel_mf_branch="${kernel_grease_branch}"
          fi
        fi
        echo "kernel_AU_TAG before processing=$kernel_au_tag"

        if oss_args_check "kernel" "$kernel_oss_manifest_git" "$kernel_oss_url"
           then echo "success"
           else echo " failure"
        fi
        pushd $ROOT_PWD/$KERNEL_OSS_DIR_NAME
        kernel_oss_au_xml="caf_$kernel_au_tag.xml"
        [ "$kernel_oss_url" == "https://git.codelinaro.org" ] && kernel_oss_au_xml="$kernel_au_tag.xml"
        echo " repo init KERNEL SI OSS"
        ARGS="-u $kernel_oss_url/$kernel_oss_manifest_git.git -b release -m $kernel_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
        if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
          ARGS="$ARGS -g $groups"
        fi
        if $shallow_clone; then
          ARGS="--depth=1 $ARGS"
        fi
        echo "repo init $ARGS"
        repo init $ARGS
        echo "KERNEL SI OSS repo init done"
      fi
    fi

    if [ "$found_display_au" = true ] ; then
      if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
        if chipcode_hf_args_check "display" "$display_chipcode_hf_server" "$display_chipcode_hf_manifest_git" "$display_chipcode_hf_manifest_branch"
        then
           display_chipcode_hf_fetch_url="https://${display_chipcode_hf_server}/"
           display_fetch_mf_url="${display_chipcode_hf_fetch_url}${display_chipcode_hf_manifest_git}"
           display_mf_branch="${display_chipcode_hf_manifest_branch}"
        fi
      else
        if grease_args_check "display" "$grease_server" "$display_grease_branch"
        then
           display_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
           display_fetch_mf_url="${display_grease_fetch_url}${display_grease_manifest_git}"
           display_mf_branch="${display_grease_branch}"
        fi
      fi
      echo "TECHPACK_DISPLAY_AU_TAG before processing=$display_au_tag"

      if oss_args_check "display" "$display_oss_manifest_git" "$display_oss_url"
         then echo "success"
         else echo " failure"
      fi

      pushd $ROOT_PWD/$DISPLAY_OSS_DIR_NAME
      display_oss_au_xml="caf_$display_au_tag.xml"
      [ "$display_oss_url" == "https://git.codelinaro.org" ] && display_oss_au_xml="$display_au_tag.xml"
      echo " repo init TECHPACK DISPLAY SI OSS"
      ARGS="-u $display_oss_url/$display_oss_manifest_git.git -b release -m $display_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
      if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
        ARGS="$ARGS -g $groups"
      fi
      if $shallow_clone; then
        ARGS="--depth=1 $ARGS"
      fi
      echo "repo init $ARGS"
      repo init $ARGS
      echo "TECHPACK DISPLAY SI OSS repo init done"
    fi

    if [ "$found_camera_au" = true ] ; then
      if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
        if chipcode_hf_args_check "camera" "$camera_chipcode_hf_server" "$camera_chipcode_hf_manifest_git" "$camera_chipcode_hf_manifest_branch"
        then
           camera_chipcode_hf_fetch_url="https://${camera_chipcode_hf_server}/"
           camera_fetch_mf_url="${camera_chipcode_hf_fetch_url}${camera_chipcode_hf_manifest_git}"
           camera_mf_branch="${camera_chipcode_hf_manifest_branch}"
        fi
      else
        if grease_args_check "camera" "$grease_server" "$camera_grease_branch"
        then
           camera_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
           camera_fetch_mf_url="${camera_grease_fetch_url}${camera_grease_manifest_git}"
           camera_mf_branch="${camera_grease_branch}"
        fi
      fi
      echo "TECHPACK_CAMERA_AU_TAG before processing=$camera_au_tag"

      if oss_args_check "camera" "$camera_oss_manifest_git" "$camera_oss_url"
         then echo "success"
         else echo " failure"
      fi

      pushd $ROOT_PWD/$CAMERA_OSS_DIR_NAME
      camera_oss_au_xml="caf_$camera_au_tag.xml"
      [ "$camera_oss_url" == "https://git.codelinaro.org" ] && camera_oss_au_xml="$camera_au_tag.xml"
      echo " repo init TECHPACK CAMERA SI OSS"
      ARGS="-u $camera_oss_url/$camera_oss_manifest_git.git -b release -m $camera_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
      if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
        ARGS="$ARGS -g $groups"
      fi
      if $shallow_clone; then
        ARGS="--depth=1 $ARGS"
      fi
      echo "repo init $ARGS"
      repo init $ARGS
      echo "TECHPACK CAMERA SI OSS repo init done"
    fi

    if [ "$found_graphics_au" = true ] ; then
      if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
        if chipcode_hf_args_check "graphics" "$graphics_chipcode_hf_server" "$graphics_chipcode_hf_manifest_git" "$graphics_chipcode_hf_manifest_branch"
        then
           graphics_chipcode_hf_fetch_url="https://${graphics_chipcode_hf_server}/"
           graphics_fetch_mf_url="${graphics_chipcode_hf_fetch_url}${graphics_chipcode_hf_manifest_git}"
           graphics_mf_branch="${graphics_chipcode_hf_manifest_branch}"
        fi
      else
        if grease_args_check "graphics" "$grease_server" "$graphics_grease_branch"
        then
           graphics_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
           graphics_fetch_mf_url="${graphics_grease_fetch_url}${graphics_grease_manifest_git}"
           graphics_mf_branch="${graphics_grease_branch}"
        fi
      fi
      echo "TECHPACK_GRAPHICS_AU_TAG before processing=$graphics_au_tag"

      if oss_args_check "graphics" "$graphics_oss_manifest_git" "$graphics_oss_url"
         then echo "success"
         else echo " failure"
      fi

      pushd $ROOT_PWD/$GRAPHICS_OSS_DIR_NAME
      graphics_oss_au_xml="caf_$graphics_au_tag.xml"
      [ "$graphics_oss_url" == "https://git.codelinaro.org" ] && graphics_oss_au_xml="$graphics_au_tag.xml"
      echo " repo init TECHPACK GRAPHICS SI OSS"
      ARGS="-u $graphics_oss_url/$graphics_oss_manifest_git.git -b release -m $graphics_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
      if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
        ARGS="$ARGS -g $groups"
      fi
      if $shallow_clone; then
        ARGS="--depth=1 $ARGS"
      fi
      echo "repo init $ARGS"
      repo init $ARGS
      echo "TECHPACK GRAPHICS SI OSS repo init done"
    fi

    if [ "$found_audio_au" = true ] ; then
      if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
        if chipcode_hf_args_check "audio" "$audio_chipcode_hf_server" "$audio_chipcode_hf_manifest_git" "$audio_chipcode_hf_manifest_branch"
        then
           audio_chipcode_hf_fetch_url="https://${audio_chipcode_hf_server}/"
           audio_fetch_mf_url="${audio_chipcode_hf_fetch_url}${audio_chipcode_hf_manifest_git}"
           audio_mf_branch="${audio_chipcode_hf_manifest_branch}"
        fi
      else
        if grease_args_check "camera" "$grease_server" "$audio_grease_branch"
        then
           audio_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
           audio_fetch_mf_url="${audio_grease_fetch_url}${audio_grease_manifest_git}"
           audio_mf_branch="${audio_grease_branch}"
        fi
      fi
      echo "TECHPACK_AUDIO_AU_TAG before processing=$audio_au_tag"

      if oss_args_check "audio" "$audio_oss_manifest_git" "$audio_oss_url"
         then echo "success"
         else echo " failure"
      fi

      pushd $ROOT_PWD/$AUDIO_OSS_DIR_NAME
      audio_oss_au_xml="caf_$audio_au_tag.xml"
      [ "$audio_oss_url" == "https://git.codelinaro.org" ] && audio_oss_au_xml="$audio_au_tag.xml"
      echo " repo init TECHPACK AUDIO SI OSS"
      ARGS="-u $audio_oss_url/$audio_oss_manifest_git.git -b release -m $audio_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
      if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
        ARGS="$ARGS -g $groups"
      fi
      if $shallow_clone; then
        ARGS="--depth=1 $ARGS"
      fi
      echo "repo init $ARGS"
      repo init $ARGS
      echo "TECHPACK AUDIO SI OSS repo init done"
    fi

    if [ "$found_sensor_au" = true ] ; then
      if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
        if chipcode_hf_args_check "sensor" "$sensor_chipcode_hf_server" "$sensor_chipcode_hf_manifest_git" "$sensor_chipcode_hf_manifest_branch"
        then
           sensor_chipcode_hf_fetch_url="https://${sensor_chipcode_hf_server}/"
           sensor_fetch_mf_url="${sensor_chipcode_hf_fetch_url}${sensor_chipcode_hf_manifest_git}"
           sensor_mf_branch="${sensor_chipcode_hf_manifest_branch}"
        fi
      else
        if grease_args_check "sensor" "$grease_server" "$sensor_grease_branch"
        then
           sensor_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
           sensor_fetch_mf_url="${sensor_grease_fetch_url}${sensor_grease_manifest_git}"
           sensor_mf_branch="${sensor_grease_branch}"
        fi
      fi
      echo "TECHPACK_SENSOR_AU_TAG before processing=$sensor_au_tag"

      #if oss_args_check "sensor" "$sensor_oss_manifest_git" "$sensor_oss_url"
      #   then echo "success"
      #   else echo " failure"
      #fi

      pushd $ROOT_PWD/$SENSOR_OSS_DIR_NAME
      #sensor_oss_au_xml="caf_$sensor_au_tag.xml"
      #[ "$sensor_oss_url" == "https://git.codelinaro.org" ] && sensor_oss_au_xml="$sensor_au_tag.xml"
      #echo " repo init TECHPACK SENSOR SI OSS"
      #ARGS="--no-clone-bundle -u $sensor_oss_url/$sensor_oss_manifest_git.git -b release -m $sensor_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
      #if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
      #  ARGS="$ARGS -g $groups"
      #fi
      #if $shallow_clone; then
      #  ARGS="--depth=1 $ARGS"
      #fi
      #echo "repo init $ARGS"
      #repo init $ARGS
      #echo "TECHPACK SENSOR SI OSS repo init done"
    fi

    if [ "$found_xr_au" = true ] ; then
      if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
        if chipcode_hf_args_check "xr" "$xr_chipcode_hf_server" "$xr_chipcode_hf_manifest_git" "$xr_chipcode_hf_manifest_branch"
        then
           xr_chipcode_hf_fetch_url="https://${xr_chipcode_hf_server}/"
           xr_fetch_mf_url="${xr_chipcode_hf_fetch_url}${xr_chipcode_hf_manifest_git}"
           xr_mf_branch="${xr_chipcode_hf_manifest_branch}"
        fi
      else
        if grease_args_check "xr" "$grease_server" "$xr_grease_branch"
        then
           xr_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
           xr_fetch_mf_url="${xr_grease_fetch_url}${xr_grease_manifest_git}"
           xr_mf_branch="${xr_grease_branch}"
        fi
      fi
      echo "TECHPACK_XR_AU_TAG before processing=$xr_au_tag"

      #if oss_args_check "xr" "$xr_oss_manifest_git" "$xr_oss_url"
      #   then echo "success"
      #   else echo " failure"
      #fi

      pushd $ROOT_PWD/$XR_OSS_DIR_NAME
      #xr_oss_au_xml="caf_$xr_au_tag.xml"
      #[ "$xr_oss_url" == "https://git.codelinaro.org" ] && xr_oss_au_xml="$xr_au_tag.xml"
      #echo " repo init TECHPACK XR SI OSS"
      #ARGS="--no-clone-bundle -u $xr_oss_url/$xr_oss_manifest_git.git -b release -m $xr_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
      #if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
      #  ARGS="$ARGS -g $groups"
      #fi
      #if $shallow_clone; then
      #  ARGS="--depth=1 $ARGS"
      #fi
      #echo "repo init $ARGS"
      #repo init $ARGS
      #echo "TECHPACK XR SI OSS repo init done"
    fi

    if [ "$found_cv_au" = true ] ; then
      if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
        if chipcode_hf_args_check "cv" "$cv_chipcode_hf_server" "$cv_chipcode_hf_manifest_git" "$cv_chipcode_hf_manifest_branch"
        then
           cv_chipcode_hf_fetch_url="https://${cv_chipcode_hf_server}/"
           cv_fetch_mf_url="${cv_chipcode_hf_fetch_url}${cv_chipcode_hf_manifest_git}"
           cv_mf_branch="${cv_chipcode_hf_manifest_branch}"
        fi
      else
        if grease_args_check "cv" "$grease_server" "$cv_grease_branch"
        then
           cv_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
           cv_fetch_mf_url="${cv_grease_fetch_url}${cv_grease_manifest_git}"
           cv_mf_branch="${cv_grease_branch}"
        fi
      fi
      echo "TECHPACK_CV_AU_TAG before processing=$cv_au_tag"

      if oss_args_check "cv" "$cv_oss_manifest_git" "$cv_oss_url"
         then echo "success"
         else echo " failure"
      fi

      pushd $ROOT_PWD/$CV_OSS_DIR_NAME
      cv_oss_au_xml="caf_$cv_au_tag.xml"
      [ "$cv_oss_url" == "https://git.codelinaro.org" ] && cv_oss_au_xml="$cv_au_tag.xml"
      echo " repo init TECHPACK CV SI OSS"
      ARGS="-u $cv_oss_url/$cv_oss_manifest_git.git -b release -m $cv_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
      if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
        ARGS="$ARGS -g $groups"
      fi
      if $shallow_clone; then
        ARGS="--depth=1 $ARGS"
      fi
      echo "repo init $ARGS"
      repo init $ARGS
      echo "TECHPACK CV SI OSS repo init done"
    fi

    if [ "$found_video_au" = true ] ; then
      if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
        if chipcode_hf_args_check "video" "$video_chipcode_hf_server" "$video_chipcode_hf_manifest_git" "$video_chipcode_hf_manifest_branch"
        then
           video_chipcode_hf_fetch_url="https://${video_chipcode_hf_server}/"
           video_fetch_mf_url="${video_chipcode_hf_fetch_url}${video_chipcode_hf_manifest_git}"
           video_mf_branch="${video_chipcode_hf_manifest_branch}"
        fi
      else
        if grease_args_check "video" "$grease_server" "$video_grease_branch"
        then
           video_grease_fetch_url="ssh://${grease_userid}@${grease_server}:29418/"
           video_fetch_mf_url="${video_grease_fetch_url}${video_grease_manifest_git}"
           video_mf_branch="${video_grease_branch}"
        fi
      fi
      echo "TECHPACK_VIDEO_AU_TAG before processing=$video_au_tag"

      if oss_args_check "video" "$video_oss_manifest_git" "$video_oss_url"
         then echo "success"
         else echo " failure"
      fi

      pushd $ROOT_PWD/$VIDEO_OSS_DIR_NAME
      video_oss_au_xml="caf_$video_au_tag.xml"
      [ "$video_oss_url" == "https://git.codelinaro.org" ] && video_oss_au_xml="$video_au_tag.xml"
      echo " repo init TECHPACK VIDEO SI OSS"
      ARGS="-u $video_oss_url/$video_oss_manifest_git.git -b release -m $video_oss_au_xml --repo-url=$repo_url --repo-branch=$repo_branch"
      if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
        ARGS="$ARGS -g $groups"
      fi
      if $shallow_clone; then
        ARGS="--depth=1 $ARGS"
      fi
      echo "repo init $ARGS"
      repo init $ARGS
      echo "TECHPACK VIDEO SI OSS repo init done"
    fi
  fi

  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
    pushd $ROOT_PWD/$VENDOR_PROP_DIR_NAME
    echo " repo init Vendor SI PROP"
    vendor_mf_name="${vendor_au_tag}.xml"
    ARGS="-u $vendor_fetch_mf_url \
          -m $vendor_mf_name \
          -b $vendor_mf_branch \
          --repo-url=$repo_url \
          --repo-branch=$repo_branch"
    if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
      ARGS="$ARGS -g $groups"
    fi
    if $shallow_clone; then
      ARGS="--depth=1 $ARGS"
    fi
    echo "repo init $ARGS"
    repo init $ARGS
    echo "Vendor SI PROP repo init done"

    if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]
    then
      if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] ; then
        pushd $ROOT_PWD/$QSSI_PROP_DIR_NAME
        echo " repo init QSSI SI PROP"
        qssi_mf_name="${qssi_au_tag}.xml"
        ARGS="--no-clone-bundle -u $qssi_fetch_mf_url \
            -m $qssi_mf_name \
            -b $qssi_mf_branch \
            --repo-url=$repo_url \
            --repo-branch=$repo_branch"
        if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
          ARGS="$ARGS -g $groups"
        fi
        if $shallow_clone; then
          ARGS="--depth=1 $ARGS"
        fi
        echo "repo init $ARGS"
        repo init $ARGS
        echo "QSSI SI PROP repo init done"
        if [ "$found_kernel_au" = true ] ; then
          pushd $ROOT_PWD/$KERNEL_PROP_DIR_NAME
          echo " repo init KERNEL SI PROP"
          kernel_mf_name="${kernel_au_tag}.xml"
          ARGS="--no-clone-bundle -u $kernel_fetch_mf_url \
              -m $kernel_mf_name \
              -b $kernel_mf_branch \
              --repo-url=$repo_url \
              --repo-branch=$repo_branch"
          if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
            ARGS="$ARGS -g $groups"
          fi
          if $shallow_clone; then
            ARGS="--depth=1 $ARGS"
          fi
          echo "repo init $ARGS"
          repo init $ARGS
          echo "KERNEL SI PROP repo init done"
        fi
      fi
      if [ "$found_display_au" = true ] ; then
        pushd $ROOT_PWD/$DISPLAY_PROP_DIR_NAME
        echo " repo init TECHPACK DISPLAY SI PROP"
        display_mf_name="${display_au_tag}.xml"
        ARGS="-c --no-clone-bundle -u $display_fetch_mf_url \
              -m $display_mf_name \
              -b $display_mf_branch \
              --repo-url=$repo_url \
              --repo-branch=$repo_branch"
        if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
          ARGS="$ARGS -g $groups"
        fi
        if $shallow_clone; then
          ARGS="--depth=1 $ARGS"
        fi
        echo "repo init $ARGS"
        repo init $ARGS
        echo "TECHPACK DISPLAY SI PROP repo init done"
      fi
      if [ "$found_camera_au" = true ] ; then
        pushd $ROOT_PWD/$CAMERA_PROP_DIR_NAME
        echo " repo init TECHPACK CAMERA SI PROP"
        camera_mf_name="${camera_au_tag}.xml"
        ARGS="-c --no-clone-bundle -u $camera_fetch_mf_url \
              -m $camera_mf_name \
              -b $camera_mf_branch \
              --repo-url=$repo_url \
              --repo-branch=$repo_branch"
        if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
          ARGS="$ARGS -g $groups"
        fi
        if $shallow_clone; then
          ARGS="--depth=1 $ARGS"
        fi
        echo "repo init $ARGS"
        repo init $ARGS
        echo "TECHPACK CAMERA SI PROP repo init done"
      fi
      if [ "$found_graphics_au" = true ] ; then
        pushd $ROOT_PWD/$GRAPHICS_PROP_DIR_NAME
        echo " repo init TECHPACK GRAPHICS SI PROP"
        graphics_mf_name="${graphics_au_tag}.xml"
        ARGS="--no-clone-bundle -u $graphics_fetch_mf_url \
              -m $graphics_mf_name \
              -b $graphics_mf_branch \
              --repo-url=$repo_url \
              --repo-branch=$repo_branch"
        if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
          ARGS="$ARGS -g $groups"
        fi
        if $shallow_clone; then
          ARGS="--depth=1 $ARGS"
        fi
        echo "repo init $ARGS"
        repo init $ARGS
        echo "TECHPACK GRAPHICS SI PROP repo init done"
      fi
      if [ "$found_audio_au" = true ] ; then
        pushd $ROOT_PWD/$AUDIO_PROP_DIR_NAME
        echo " repo init TECHPACK AUDIO SI PROP"
        audio_mf_name="${audio_au_tag}.xml"
        ARGS="--no-clone-bundle -u $audio_fetch_mf_url \
              -m $audio_mf_name \
              -b $audio_mf_branch \
              --repo-url=$repo_url \
              --repo-branch=$repo_branch"
        if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
          ARGS="$ARGS -g $groups"
        fi
        if $shallow_clone; then
          ARGS="--depth=1 $ARGS"
        fi
        echo "repo init $ARGS"
        repo init $ARGS
        echo "TECHPACK AUDIO SI PROP repo init done"
      fi
      if [ "$found_sensor_au" = true ] ; then
        pushd $ROOT_PWD/$SENSOR_PROP_DIR_NAME
        echo " repo init TECHPACK SENSOR SI PROP"
        sensor_mf_name="${sensor_au_tag}.xml"
        ARGS="-c --no-clone-bundle -u $sensor_fetch_mf_url \
              -m $sensor_mf_name \
              -b $sensor_mf_branch \
              --repo-url=$repo_url \
              --repo-branch=$repo_branch"
        if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
          ARGS="$ARGS -g $groups"
        fi
        if $shallow_clone; then
          ARGS="--depth=1 $ARGS"
        fi
        echo "repo init $ARGS"
        repo init $ARGS
        echo "TECHPACK SENSOR SI PROP repo init done"
      fi
      if [ "$found_xr_au" = true ] ; then
        pushd $ROOT_PWD/$XR_PROP_DIR_NAME
        echo " repo init TECHPACK XR SI PROP"
        xr_mf_name="${xr_au_tag}.xml"
        ARGS="-c --no-clone-bundle -u $xr_fetch_mf_url \
              -m $xr_mf_name \
              -b $xr_mf_branch \
              --repo-url=$repo_url \
              --repo-branch=$repo_branch"
        if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
          ARGS="$ARGS -g $groups"
        fi
        if $shallow_clone; then
          ARGS="--depth=1 $ARGS"
        fi
        echo "repo init $ARGS"
        repo init $ARGS
        echo "TECHPACK XR SI PROP repo init done"
      fi
      if [ "$found_cv_au" = true ] ; then
        pushd $ROOT_PWD/$CV_PROP_DIR_NAME
        echo " repo init TECHPACK CV SI PROP"
        cv_mf_name="${cv_au_tag}.xml"
        ARGS="--no-clone-bundle -u $cv_fetch_mf_url \
              -m $cv_mf_name \
              -b $cv_mf_branch \
              --repo-url=$repo_url \
              --repo-branch=$repo_branch"
        if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
          ARGS="$ARGS -g $groups"
        fi
        if $shallow_clone; then
          ARGS="--depth=1 $ARGS"
        fi
        echo "repo init $ARGS"
        repo init $ARGS
        echo "TECHPACK CV SI PROP repo init done"
      fi
      if [ "$found_video_au" = true ] ; then
        pushd $ROOT_PWD/$VIDEO_PROP_DIR_NAME
        echo " repo init TECHPACK VIDEO SI PROP"
        video_mf_name="${video_au_tag}.xml"
        ARGS="--no-clone-bundle -u $video_fetch_mf_url \
              -m $video_mf_name \
              -b $video_mf_branch \
              --repo-url=$repo_url \
              --repo-branch=$repo_branch"
        if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
          ARGS="$ARGS -g $groups"
        fi
        if $shallow_clone; then
          ARGS="--depth=1 $ARGS"
        fi
        echo "repo init $ARGS"
        repo init $ARGS
        echo "TECHPACK VIDEO SI PROP repo init done"
      fi
    fi
  fi
  pushd $ROOT_PWD
fi

if [ "$tree_type" == $QSSI_TREE_STANDALONE ] || [ "$tree_type" == $KERNEL_TREE_STANDALONE ] || [ "$tree_type" == $CAMERA_TREE_STANDALONE ] || [ "$tree_type" == $DISPLAY_TREE_STANDALONE ] || [ "$tree_type" == $XR_TREE_STANDALONE ] || [ "$tree_type" == $SENSOR_TREE_STANDALONE ] || [ "$tree_type" == $GRAPHICS_TREE_STANDALONE ] || [ "$tree_type" == $CV_TREE_STANDALONE ] || [ "$tree_type" == $AUDIO_TREE_STANDALONE ]  || [ "$tree_type" == $VIDEO_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]
then
  # Since the defaults will likely conflict (like remote), delete them.
  # Must remove the defaults before updating so that the default remote
  # is updated correctly.
  pushd $ROOT_PWD
  if [ "$tree_type" == $QSSI_TREE_STANDALONE ] || [ "$tree_type" == $KERNEL_TREE_STANDALONE ] || [ "$tree_type" == $DISPLAY_TREE_STANDALONE ] || [ "$tree_type" == $CAMERA_TREE_STANDALONE ] || [ "$tree_type" == $XR_TREE_STANDALONE ] || [ "$tree_type" == $SENSOR_TREE_STANDALONE ] || [ "$tree_type" == $GRAPHICS_TREE_STANDALONE ] || [ "$tree_type" == $CV_TREE_STANDALONE ] || [ "$tree_type" == $AUDIO_TREE_STANDALONE ] || [ "$tree_type" == $VIDEO_TREE_STANDALONE ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]
  then
    if [ -d $ROOT_PWD/$REPO_BACKUP_PATH/.repo ];then
      mv $ROOT_PWD/$REPO_BACKUP_PATH/.repo $ROOT_PWD/
    fi
  fi
  echo "calling remove defaults"

  if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]
  then
    if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
      qssi_oss_au_xml="caf_$qssi_au_tag.xml"
      [ "$qssi_oss_url" == "https://git.codelinaro.org" ] && qssi_oss_au_xml="$qssi_au_tag.xml"
      qssi_oss_manifest=$ROOT_PWD/$QSSI_OSS_DIR_NAME/.repo/manifests/$qssi_oss_au_xml
      qremote="cafq"
      echo "qssi OSS manifest = $qssi_oss_manifest, qremote = $qremote"
      xml1=$(cat "$qssi_oss_manifest" | remove_defaults)

      echo "Getting remotes"
      m1remotes=($(cat "$qssi_oss_manifest" |\
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
      mv $ROOT_PWD/qcafmanifest.xml $ROOT_PWD/$QSSI_OSS_DIR_NAME/.repo/manifests/$qssi_oss_au_xml

      if [ "$found_kernel_au" = true ] ; then
        kernel_oss_au_xml="caf_$kernel_au_tag.xml"
        [ "$kernel_oss_url" == "https://git.codelinaro.org" ] && kernel_oss_au_xml="$kernel_au_tag.xml"
        kernel_oss_manifest=$ROOT_PWD/$KERNEL_OSS_DIR_NAME/.repo/manifests/$kernel_oss_au_xml
        qremote="cafk"
        echo "kernel OSS manifest = $kernel_oss_manifest, qremote = $qremote"
        xml1=$(cat "$kernel_oss_manifest" | remove_defaults)

        echo "Getting remotes"
        m1remotes=($(cat "$kernel_oss_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
        echo "replacing remotes"
        for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
        done
        rm -rf $ROOT_PWD/kcafmanifest.xml
        {
          echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

          echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
          echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

          echo '</manifest>'
        } |  xmlstarlet fo >> $ROOT_PWD/kcafmanifest.xml

        echo " kernel caf manifest processing is done"
        mv $ROOT_PWD/kcafmanifest.xml $ROOT_PWD/$KERNEL_OSS_DIR_NAME/.repo/manifests/$kernel_oss_au_xml
      fi
    fi
    if [ "$found_display_au" = true ] ; then
      display_oss_au_xml="caf_$display_au_tag.xml"
      [ "$display_oss_url" == "https://git.codelinaro.org" ] && display_oss_au_xml="$display_au_tag.xml"
      display_oss_manifest=$ROOT_PWD/$DISPLAY_OSS_DIR_NAME/.repo/manifests/$display_oss_au_xml
      qremote="cafd"
      echo "TECHPACK DISPLAY OSS manifest = $display_oss_manifest, qremote = $qremote"
      xml1=$(cat "$display_oss_manifest" | remove_defaults)

      echo "Getting remotes"
      m1remotes=($(cat "$display_oss_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
      echo "replacing remotes"
      for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      done
      rm -rf $ROOT_PWD/dcafmanifest.xml
      {
        echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

        echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
        echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

        echo '</manifest>'
      } |  xmlstarlet fo >> $ROOT_PWD/dcafmanifest.xml

      echo " TECHPACK DISPLAY caf manifest processing is done"
      mv $ROOT_PWD/dcafmanifest.xml $ROOT_PWD/$DISPLAY_OSS_DIR_NAME/.repo/manifests/$display_oss_au_xml
    fi

    if [ "$found_camera_au" = true ] ; then
      camera_oss_au_xml="caf_$camera_au_tag.xml"
      [ "$camera_oss_url" == "https://git.codelinaro.org" ] && camera_oss_au_xml="$camera_au_tag.xml"
      camera_oss_manifest=$ROOT_PWD/$CAMERA_OSS_DIR_NAME/.repo/manifests/$camera_oss_au_xml
      qremote="cafc"
      echo "TECHPACK CAMERA OSS manifest = $camera_oss_manifest, qremote = $qremote"
      xml1=$(cat "$camera_oss_manifest" | remove_defaults)

      echo "Getting remotes"
      m1remotes=($(cat "$camera_oss_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
      echo "replacing remotes"
      for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      done
      rm -rf $ROOT_PWD/ccafmanifest.xml
      {
        echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

        echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
        echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

        echo '</manifest>'
      } |  xmlstarlet fo >> $ROOT_PWD/ccafmanifest.xml

      echo " TECHPACK CAMERA caf manifest processing is done"
      mv $ROOT_PWD/ccafmanifest.xml $ROOT_PWD/$CAMERA_OSS_DIR_NAME/.repo/manifests/$camera_oss_au_xml
    fi

    if [ "$found_graphics_au" = true ] ; then
      graphics_oss_au_xml="caf_$graphics_au_tag.xml"
      [ "$graphics_oss_url" == "https://git.codelinaro.org" ] && graphics_oss_au_xml="$graphics_au_tag.xml"
      graphics_oss_manifest=$ROOT_PWD/$GRAPHICS_OSS_DIR_NAME/.repo/manifests/$graphics_oss_au_xml
      qremote="cafg"
      echo "TECHPACK GRAPHICS OSS manifest = $graphics_oss_manifest, qremote = $qremote"
      xml1=$(cat "$graphics_oss_manifest" | remove_defaults)

      echo "Getting remotes"
      m1remotes=($(cat "$graphics_oss_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
      echo "replacing remotes"
      for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      done
      rm -rf $ROOT_PWD/gcafmanifest.xml
      {
        echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

        echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
        echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

        echo '</manifest>'
      } |  xmlstarlet fo >> $ROOT_PWD/gcafmanifest.xml

      echo " TECHPACK GRAPHICS caf manifest processing is done"
      mv $ROOT_PWD/gcafmanifest.xml $ROOT_PWD/$GRAPHICS_OSS_DIR_NAME/.repo/manifests/$graphics_oss_au_xml
    fi

    if [ "$found_audio_au" = true ] ; then
      audio_oss_au_xml="caf_$audio_au_tag.xml"
      [ "$audio_oss_url" == "https://git.codelinaro.org" ] && audio_oss_au_xml="$audio_au_tag.xml"
      audio_oss_manifest=$ROOT_PWD/$AUDIO_OSS_DIR_NAME/.repo/manifests/$audio_oss_au_xml
      qremote="cafa"
      echo "TECHPACK AUDIO OSS manifest = $audio_oss_manifest, qremote = $qremote"
      xml1=$(cat "$audio_oss_manifest" | remove_defaults)

      echo "Getting remotes"
      m1remotes=($(cat "$audio_oss_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
      echo "replacing remotes"
      for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      done
      rm -rf $ROOT_PWD/acafmanifest.xml
      {
        echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

        echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
        echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

        echo '</manifest>'
      } |  xmlstarlet fo >> $ROOT_PWD/acafmanifest.xml

      echo " TECHPACK AUDIO caf manifest processing is done"
      mv $ROOT_PWD/acafmanifest.xml $ROOT_PWD/$AUDIO_OSS_DIR_NAME/.repo/manifests/$audio_oss_au_xml
    fi

    found_sensor_au=false
    if [ "$found_sensor_au" = true ] ; then
      sensor_oss_au_xml="caf_$sensor_au_tag.xml"
      [ "$sensor_oss_url" == "https://git.codelinaro.org" ] && sensor_oss_au_xml="$sensor_au_tag.xml"
      sensor_oss_manifest=$ROOT_PWD/$SENSOR_OSS_DIR_NAME/.repo/manifests/$sensor_oss_au_xml
      qremote="cafs"
      echo "TECHPACK SENSOR OSS manifest = $sensor_oss_manifest, qremote = $qremote"
      xml1=$(cat "$sensor_oss_manifest" | remove_defaults)

      echo "Getting remotes"
      m1remotes=($(cat "$sensor_oss_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
      echo "replacing remotes"
      for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      done
      rm -rf $ROOT_PWD/scafmanifest.xml
      {
        echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

        echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
        echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

        echo '</manifest>'
      } |  xmlstarlet fo >> $ROOT_PWD/scafmanifest.xml

      echo " TECHPACK SENSOR caf manifest processing is done"
      mv $ROOT_PWD/scafmanifest.xml $ROOT_PWD/$SENSOR_OSS_DIR_NAME/.repo/manifests/$sensor_oss_au_xml
    fi
    found_sensor_au=true

    found_xr_au=false
    if [ "$found_xr_au" = true ] ; then
      xr_oss_au_xml="caf_$xr_au_tag.xml"
      [ "$xr_oss_url" == "https://git.codelinaro.org" ] && xr_oss_au_xml="$xr_au_tag.xml"
      xr_oss_manifest=$ROOT_PWD/$XR_OSS_DIR_NAME/.repo/manifests/$xr_oss_au_xml
      qremote="cafxr"
      echo "TECHPACK XR OSS manifest = $xr_oss_manifest, qremote = $qremote"
      xml1=$(cat "$xr_oss_manifest" | remove_defaults)

      echo "Getting remotes"
      m1remotes=($(cat "$xr_oss_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
      echo "replacing remotes"
      for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      done
      rm -rf $ROOT_PWD/xrcafmanifest.xml
      {
        echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

        echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
        echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

        echo '</manifest>'
      } |  xmlstarlet fo >> $ROOT_PWD/xrcafmanifest.xml

      echo " TECHPACK XR caf manifest processing is done"
      mv $ROOT_PWD/xrcafmanifest.xml $ROOT_PWD/$XR_OSS_DIR_NAME/.repo/manifests/$xr_oss_au_xml
    fi
    found_xr_au=true

    if [ "$found_cv_au" = true ] ; then
      cv_oss_au_xml="caf_$cv_au_tag.xml"
      [ "$cv_oss_url" == "https://git.codelinaro.org" ] && cv_oss_au_xml="$cv_au_tag.xml"
      cv_oss_manifest=$ROOT_PWD/$CV_OSS_DIR_NAME/.repo/manifests/$cv_oss_au_xml
      qremote="cafcv"
      echo "TECHPACK CV OSS manifest = $cv_oss_manifest, qremote = $qremote"
      xml1=$(cat "$cv_oss_manifest" | remove_defaults)

      echo "Getting remotes"
      m1remotes=($(cat "$cv_oss_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
      echo "replacing remotes"
      for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      done
      rm -rf $ROOT_PWD/cvcafmanifest.xml
      {
        echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

        echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
        echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

        echo '</manifest>'
      } |  xmlstarlet fo >> $ROOT_PWD/cvcafmanifest.xml

      echo " TECHPACK CV caf manifest processing is done"
      mv $ROOT_PWD/cvcafmanifest.xml $ROOT_PWD/$CV_OSS_DIR_NAME/.repo/manifests/$cv_oss_au_xml
    fi

    if [ "$found_video_au" = true ] ; then
      video_oss_au_xml="caf_$video_au_tag.xml"
      [ "$video_oss_url" == "https://git.codelinaro.org" ] && video_oss_au_xml="$video_au_tag.xml"
      video_oss_manifest=$ROOT_PWD/$VIDEO_OSS_DIR_NAME/.repo/manifests/$video_oss_au_xml
      qremote="cafv"
      echo "TECHPACK VIDEO OSS manifest = $video_oss_manifest, qremote = $qremote"
      xml1=$(cat "$video_oss_manifest" | remove_defaults)

      echo "Getting remotes"
      m1remotes=($(cat "$video_oss_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
      echo "replacing remotes"
      for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      done
      rm -rf $ROOT_PWD/vcafmanifest.xml
      {
        echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

        echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
        echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

        echo '</manifest>'
      } |  xmlstarlet fo >> $ROOT_PWD/vcafmanifest.xml

      echo " TECHPACK VIDEO caf manifest processing is done"
      mv $ROOT_PWD/vcafmanifest.xml $ROOT_PWD/$VIDEO_OSS_DIR_NAME/.repo/manifests/$video_oss_au_xml
    fi
  fi

  if [[ "$prop_dest" == $PROP_DEST_GREASE  || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
    if [ "$tree_type" == $QSSI_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]
    then
      if [ "$tree_type" == $QSSI_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] ; then
        qssi_grease_manifest=$ROOT_PWD/$QSSI_PROP_DIR_NAME/.repo/manifests/${qssi_au_tag}.xml
        qremote="greaseq"
        echo "QSSI Grease manifest = $qssi_grease_manifest, qremote = $qremote"
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
      fi
    fi
    if [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ];then
      found_kernel_au=false
    fi
    if [ "$found_kernel_au" = true ] ; then
      kernel_grease_manifest=$ROOT_PWD/$KERNEL_PROP_DIR_NAME/.repo/manifests/${kernel_au_tag}.xml
      qremote="greasek"
      echo "Kernel Grease manifest = $kernel_grease_manifest, qremote = $qremote"
      xml1=$(cat "$kernel_grease_manifest" | remove_defaults)

      echo "Getting remotes"
      m1remotes=($(cat "$kernel_grease_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
      echo "replacing remotes"
      for m1remote in "${m1remotes[@]}"; do
        xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      done
      rm -rf $ROOT_PWD/kgreasemanifest.xml
      {
          echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

          echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
          echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

          echo '</manifest>'
      } |  xmlstarlet fo >> $ROOT_PWD/kgreasemanifest.xml

      echo "Kernel grease manifest processing is done"
      mv $ROOT_PWD/kgreasemanifest.xml $ROOT_PWD/$KERNEL_PROP_DIR_NAME/.repo/manifests/${kernel_au_tag}.xml
    fi
    if [ "$found_display_au" = true ] ; then
      display_grease_manifest=$ROOT_PWD/$DISPLAY_PROP_DIR_NAME/.repo/manifests/${display_au_tag}.xml
      qremote="greased"
      echo "display Grease manifest = $display_grease_manifest, qremote = $qremote"
      xml1=$(cat "$display_grease_manifest" | remove_defaults)

      echo "Getting remotes"
      m1remotes=($(cat "$display_grease_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
      echo "replacing remotes"
      for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      done
      rm -rf $ROOT_PWD/dgreasemanifest.xml
      {
        echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

        echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
        echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

        echo '</manifest>'
      } |  xmlstarlet fo >> $ROOT_PWD/dgreasemanifest.xml

      echo "TECHPACK DISPLAY grease manifest processing is done"
      mv $ROOT_PWD/dgreasemanifest.xml $ROOT_PWD/$DISPLAY_PROP_DIR_NAME/.repo/manifests/${display_au_tag}.xml
    fi
    if [ "$found_camera_au" = true ] ; then
      camera_grease_manifest=$ROOT_PWD/$CAMERA_PROP_DIR_NAME/.repo/manifests/${camera_au_tag}.xml
      qremote="greasec"
      echo "camera Grease manifest = $camera_grease_manifest, qremote = $qremote"
      xml1=$(cat "$camera_grease_manifest" | remove_defaults)

      echo "Getting remotes"
      m1remotes=($(cat "$camera_grease_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
      echo "replacing remotes"
      for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      done
      rm -rf $ROOT_PWD/cgreasemanifest.xml
      {
        echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

        echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
        echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

        echo '</manifest>'
      } |  xmlstarlet fo >> $ROOT_PWD/cgreasemanifest.xml

      echo "TECHPACK CAMERA grease manifest processing is done"
      mv $ROOT_PWD/cgreasemanifest.xml $ROOT_PWD/$CAMERA_PROP_DIR_NAME/.repo/manifests/${camera_au_tag}.xml
    fi

    if [ "$found_graphics_au" = true ] ; then
      graphics_grease_manifest=$ROOT_PWD/$GRAPHICS_PROP_DIR_NAME/.repo/manifests/${graphics_au_tag}.xml
      qremote="greaseg"
      echo "graphics Grease manifest = $graphics_grease_manifest, qremote = $qremote"
      xml1=$(cat "$graphics_grease_manifest" | remove_defaults)

      echo "Getting remotes"
      m1remotes=($(cat "$graphics_grease_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
      echo "replacing remotes"
      for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      done
      rm -rf $ROOT_PWD/ggreasemanifest.xml
      {
        echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

        echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
        echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

        echo '</manifest>'
      } |  xmlstarlet fo >> $ROOT_PWD/ggreasemanifest.xml

      echo "TECHPACK GRAPHICS grease manifest processing is done"
      mv $ROOT_PWD/ggreasemanifest.xml $ROOT_PWD/$GRAPHICS_PROP_DIR_NAME/.repo/manifests/${graphics_au_tag}.xml
    fi
    if [ "$found_audio_au" = true ] ; then
      audio_grease_manifest=$ROOT_PWD/$AUDIO_PROP_DIR_NAME/.repo/manifests/${audio_au_tag}.xml
      qremote="greasea"
      echo "audio Grease manifest = $audio_grease_manifest, qremote = $qremote"
      xml1=$(cat "$audio_grease_manifest" | remove_defaults)

      echo "Getting remotes"
      m1remotes=($(cat "$audio_grease_manifest" |\
                 xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                 ))
      echo "replacing remotes"
      for m1remote in "${m1remotes[@]}"; do
          xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
      done
      rm -rf $ROOT_PWD/agreasemanifest.xml
      {
        echo '<?xml version="1.0" encoding="UTF-8"?>
            <manifest>'

        echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
        echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

        echo '</manifest>'
      } |  xmlstarlet fo >> $ROOT_PWD/agreasemanifest.xml

      echo "TECHPACK AUDIO grease manifest processing is done"
      mv $ROOT_PWD/agreasemanifest.xml $ROOT_PWD/$AUDIO_PROP_DIR_NAME/.repo/manifests/${audio_au_tag}.xml
    fi

    if [ "$tree_type" != $LA_COMBINED_TREE_GROUPS ]; then
      if [ "$found_sensor_au" = true ] ; then
        sensor_grease_manifest=$ROOT_PWD/$SENSOR_PROP_DIR_NAME/.repo/manifests/${sensor_au_tag}.xml
        qremote="greases"
        echo "sensor Grease manifest = $sensor_grease_manifest, qremote = $qremote"
        xml1=$(cat "$sensor_grease_manifest" | remove_defaults)

        echo "Getting remotes"
        m1remotes=($(cat "$sensor_grease_manifest" |\
                    xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                    ))
        echo "replacing remotes"
        for m1remote in "${m1remotes[@]}"; do
            xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
        done
        rm -rf $ROOT_PWD/sgreasemanifest.xml
        {
          echo '<?xml version="1.0" encoding="UTF-8"?>
              <manifest>'

          echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
          echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

          echo '</manifest>'
        } |  xmlstarlet fo >> $ROOT_PWD/sgreasemanifest.xml

        echo "TECHPACK SENSOR grease manifest processing is done"
        mv $ROOT_PWD/sgreasemanifest.xml $ROOT_PWD/$SENSOR_PROP_DIR_NAME/.repo/manifests/${sensor_au_tag}.xml
      fi

      if [ "$found_xr_au" = true ] ; then
        xr_grease_manifest=$ROOT_PWD/$XR_PROP_DIR_NAME/.repo/manifests/${xr_au_tag}.xml
        qremote="greasexr"
        echo "xr Grease manifest = $xr_grease_manifest, qremote = $qremote"
        xml1=$(cat "$xr_grease_manifest" | remove_defaults)

        echo "Getting remotes"
        m1remotes=($(cat "$xr_grease_manifest" |\
                    xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                    ))
        echo "replacing remotes"
        for m1remote in "${m1remotes[@]}"; do
            xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
        done
        rm -rf $ROOT_PWD/xrgreasemanifest.xml
        {
          echo '<?xml version="1.0" encoding="UTF-8"?>
              <manifest>'

          echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
          echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

          echo '</manifest>'
        } |  xmlstarlet fo >> $ROOT_PWD/xrgreasemanifest.xml

        echo "TECHPACK XR grease manifest processing is done"
        mv $ROOT_PWD/xrgreasemanifest.xml $ROOT_PWD/$XR_PROP_DIR_NAME/.repo/manifests/${xr_au_tag}.xml
      fi
      if [ "$found_cv_au" = true ] ; then
        cv_grease_manifest=$ROOT_PWD/$CV_PROP_DIR_NAME/.repo/manifests/${cv_au_tag}.xml
        qremote="greasecv"
        echo "cv Grease manifest = $cv_grease_manifest, qremote = $qremote"
        xml1=$(cat "$cv_grease_manifest" | remove_defaults)

        echo "Getting remotes"
        m1remotes=($(cat "$cv_grease_manifest" |\
                    xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
                    ))
        echo "replacing remotes"
        for m1remote in "${m1remotes[@]}"; do
            xml1=$(echo "$xml1" | update_remote "$m1remote" "$qremote")
        done
        rm -rf $ROOT_PWD/cvgreasemanifest.xml
        {
          echo '<?xml version="1.0" encoding="UTF-8"?>
              <manifest>'

          echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
          echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"

          echo '</manifest>'
        } |  xmlstarlet fo >> $ROOT_PWD/cvgreasemanifest.xml

        echo "TECHPACK CV grease manifest processing is done"
        mv $ROOT_PWD/cvgreasemanifest.xml $ROOT_PWD/$CV_PROP_DIR_NAME/.repo/manifests/${cv_au_tag}.xml
      fi

      if [ "$found_video_au" = true ] ; then
        video_grease_manifest=$ROOT_PWD/$VIDEO_PROP_DIR_NAME/.repo/manifests/${video_au_tag}.xml
        qremote="greasev"
        echo "video Grease manifest = $video_grease_manifest, qremote = $qremote"
        xml1=$(cat "$video_grease_manifest" | remove_defaults)

        echo "Getting remotes"
        m1remotes=($(cat "$video_grease_manifest" |\
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

        echo "TECHPACK VIDEO grease manifest processing is done"
        mv $ROOT_PWD/vgreasemanifest.xml $ROOT_PWD/$VIDEO_PROP_DIR_NAME/.repo/manifests/${video_au_tag}.xml
      fi
    fi
    if [ "$tree_type" == $QSSI_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]
    then
      if [ "$tree_type" == $QSSI_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]
      then
        if [[ "$prop_dest" == $PROP_DEST_GREASE ]];then
          echo "getting prebuilts for QSSI"
          # Test Grease Gerrit server connection
          set +e
          ssh -p 29418 ${grease_userid}@${grease_server}
          if [ $? -ne 127 ]; then
            echo "Please check your grease user name and password for correctness."
            usage
          fi
          set -e
          pushd $ROOT_PWD
          qssi_cdr_customer=$(dirname ${qssi_grease_branch})
          qssi_tar_file_gz="prebuilt_${qssi_cdr_customer}_${qssi_au_tag}.tar.gz"
          qssi_tar_file_xz="prebuilt_${qssi_cdr_customer}_${qssi_au_tag}.tar.xz"
          qssi_parent_branch=`echo ${qssi_grease_branch} | cut -d '/' -f2`
          qssi_tar_file_on_server_gz="https://${grease_server}/binaries/outgoing/${qssi_cdr_customer}/${qssi_parent_branch}/${qssi_tar_file_gz}"
          qssi_tar_file_on_server_xz="https://${grease_server}/binaries/outgoing/${qssi_cdr_customer}/${qssi_parent_branch}/${qssi_tar_file_xz}"
          ( wget --continue --no-check-certificate --user=$grease_userid \
           --password=$grease_pass $qssi_tar_file_on_server_gz ||
          wget --continue --no-check-certificate --user=$grease_userid \
           --password=$grease_pass $qssi_tar_file_on_server_xz)
          if [ "$found_kernel_au" = true ] ; then
            echo "getting QSSI prebuilts for Kernel platform"
            #Todo: KERNEL_PLATFORM has no binary shippable
          fi
	fi
	if [ "$found_display_au" = true ] ; then
          echo "getting QSSI prebuilts for TECHPACK_DISPLAY Platform"
	      #CHECK THIS TODO For TECHPACK
          #Todo: TECHPACK_DISPLAY has no binary shippable
        fi
	if [ "$found_camera_au" = true ] ; then
          echo "getting QSSI prebuilts for TECHPACK_CAMERA Platform"
	      #CHECK THIS TODO For TECHPACK
          #Todo: TECHPACK_CAMERA has no binary shippable
        fi
	if [ "$found_audio_au" = true ] ; then
          echo "getting QSSI prebuilts for TECHPACK_AUDIO Platform"
	      #CHECK THIS TODO For TECHPACK
          #Todo: TECHPACK_AUDIO has no binary shippable
        fi
	if [ "$found_graphics_au" = true ] ; then
          echo "getting QSSI prebuilts for TECHPACK_GRAPHICS Platform"
              #CHECK THIS TODO For TECHPACK
          #Todo: TECHPACK_GRAPHICS has no binary shippable
        fi
	if [ "$found_sensor_au" = true ] ; then
          echo "getting QSSI prebuilts for TECHPACK_SENSOR Platform"
	      #CHECK THIS TODO For TECHPACK
          #Todo: TECHPACK_SENSOR has no binary shippable
        fi
	if [ "$found_xr_au" = true ] ; then
          echo "getting QSSI prebuilts for TECHPACK_XR Platform"
              #CHECK THIS TODO For TECHPACK
          #Todo: TECHPACK_XR has no binary shippable
        fi
	if [ "$found_cv_au" = true ] ; then
          echo "getting QSSI prebuilts for TECHPACK_CV Platform"
	      #CHECK THIS TODO For TECHPACK
          #Todo: TECHPACK_CV has no binary shippable
        fi
	if [ "$found_video_au" = true ] ; then
          echo "getting QSSI prebuilts for TECHPACK_VIDEO Platform"
	      #CHECK THIS TODO For TECHPACK
          #Todo: TECHPACK_VIDEO has no binary shippable
        fi
      fi
    fi
  fi
fi #IF NOT VENDOR_TREE_STANDALONE

if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $VENDOR_TREE_STANDALONE ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]
then
  if [ -d $ROOT_PWD/$REPO_BACKUP_PATH/.repo ];then
    mv $ROOT_PWD/$REPO_BACKUP_PATH/.repo $ROOT_PWD/
  fi

  echo "Initializing local manifest"
  mkdir -p $ROOT_PWD/.repo/local_manifests
  if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]
  then
    qssi_oss_au_xml="caf_${qssi_au_tag}.xml"
    [ "$qssi_oss_url" == "https://git.codelinaro.org" ] && qssi_oss_au_xml="${qssi_au_tag}.xml"
    if [ "$qssi_oss_url" == "https://git.codelinaro.org" ]; then
       cp $ROOT_PWD/$QSSI_OSS_DIR_NAME/.repo/manifests/$qssi_oss_au_xml $ROOT_PWD/.repo/local_manifests/cloq_$qssi_oss_au_xml
    else
       cp $ROOT_PWD/$QSSI_OSS_DIR_NAME/.repo/manifests/$qssi_oss_au_xml $ROOT_PWD/.repo/local_manifests/
    fi
    echo "QSSI OSS Manifest copied as local manifest to local_manifests"
    if [ "$found_kernel_au" = true ] ; then
      kernel_oss_au_xml="caf_${kernel_au_tag}.xml"
      [ "$kernel_oss_url" == "https://git.codelinaro.org" ] && kernel_oss_au_xml="${kernel_au_tag}.xml"
      if [ "$kernel_oss_url" == "https://git.codelinaro.org" ]; then
         cp $ROOT_PWD/$KERNEL_OSS_DIR_NAME/.repo/manifests/$kernel_oss_au_xml $ROOT_PWD/.repo/local_manifests/clok_$kernel_oss_au_xml
      else
         cp $ROOT_PWD/$KERNEL_OSS_DIR_NAME/.repo/manifests/$kernel_oss_au_xml $ROOT_PWD/.repo/local_manifests/
      fi
      echo "KERNEL OSS Manifest copied as local manifest to local_manifests"
    fi
  fi
  if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]  ; then
    if [ "$found_display_au" = true ] ; then
      display_oss_au_xml="caf_${display_au_tag}.xml"
      [ "$display_oss_url" == "https://git.codelinaro.org" ] && display_oss_au_xml="${display_au_tag}.xml"
      if [ "$display_oss_url" == "https://git.codelinaro.org" ]; then
         cp $ROOT_PWD/$DISPLAY_OSS_DIR_NAME/.repo/manifests/$display_oss_au_xml $ROOT_PWD/.repo/local_manifests/clod_$display_oss_au_xml
      else
         cp $ROOT_PWD/$DISPLAY_OSS_DIR_NAME/.repo/manifests/$display_oss_au_xml $ROOT_PWD/.repo/local_manifests/
      fi
      echo "TECHPACK DISPLAY OSS Manifest copied as local manifest to local_manifests"
    fi
    if [ "$found_camera_au" = true ] ; then
      camera_oss_au_xml="caf_${camera_au_tag}.xml"
      [ "$camera_oss_url" == "https://git.codelinaro.org" ] && camera_oss_au_xml="${camera_au_tag}.xml"
      if [ "$camera_oss_url" == "https://git.codelinaro.org" ]; then
         cp $ROOT_PWD/$CAMERA_OSS_DIR_NAME/.repo/manifests/$camera_oss_au_xml $ROOT_PWD/.repo/local_manifests/clod_$camera_oss_au_xml
      else
         cp $ROOT_PWD/$CAMERA_OSS_DIR_NAME/.repo/manifests/$camera_oss_au_xml $ROOT_PWD/.repo/local_manifests/
      fi
      echo "TECHPACK CAMERA OSS Manifest copied as local manifest to local_manifests"
    fi
    if [ "$found_graphics_au" = true ] ; then
      graphics_oss_au_xml="caf_${graphics_au_tag}.xml"
      [ "$graphics_oss_url" == "https://git.codelinaro.org" ] && graphics_oss_au_xml="${graphics_au_tag}.xml"
      if [ "$graphics_oss_url" == "https://git.codelinaro.org" ]; then
         cp $ROOT_PWD/$GRAPHICS_OSS_DIR_NAME/.repo/manifests/$graphics_oss_au_xml $ROOT_PWD/.repo/local_manifests/clod_$graphics_oss_au_xml
      else
         cp $ROOT_PWD/$GRAPHICS_OSS_DIR_NAME/.repo/manifests/$graphics_oss_au_xml $ROOT_PWD/.repo/local_manifests/
      fi
      echo "TECHPACK GRAPHICS OSS Manifest copied as local manifest to local_manifests"
    fi
    if [ "$found_audio_au" = true ] ; then
      audio_oss_au_xml="caf_${audio_au_tag}.xml"
      [ "$audio_oss_url" == "https://git.codelinaro.org" ] && audio_oss_au_xml="${audio_au_tag}.xml"
      if [ "$audio_oss_url" == "https://git.codelinaro.org" ]; then
         cp $ROOT_PWD/$AUDIO_OSS_DIR_NAME/.repo/manifests/$audio_oss_au_xml $ROOT_PWD/.repo/local_manifests/clod_$audio_oss_au_xml
      else
         cp $ROOT_PWD/$AUDIO_OSS_DIR_NAME/.repo/manifests/$audio_oss_au_xml $ROOT_PWD/.repo/local_manifests/
      fi
      echo "TECHPACK AUDIO OSS Manifest copied as local manifest to local_manifests"
    fi
    found_sensor_au=false
    if [ "$found_sensor_au" = true ] ; then
      sensor_oss_au_xml="caf_${sensor_au_tag}.xml"
      [ "$sensor_oss_url" == "https://git.codelinaro.org" ] && sensor_oss_au_xml="${sensor_au_tag}.xml"
      if [ "$sensor_oss_url" == "https://git.codelinaro.org" ]; then
         cp $ROOT_PWD/$SENSOR_OSS_DIR_NAME/.repo/manifests/$sensor_oss_au_xml $ROOT_PWD/.repo/local_manifests/clod_$sensor_oss_au_xml
      else
         cp $ROOT_PWD/$SENSOR_OSS_DIR_NAME/.repo/manifests/$sensor_oss_au_xml $ROOT_PWD/.repo/local_manifests/
      fi
      echo "TECHPACK SENSOR OSS Manifest copied as local manifest to local_manifests"
    fi
    found_sensor_au=true
    found_xr_au=false
    if [ "$found_xr_au" = true ] ; then
      xr_oss_au_xml="caf_${xr_au_tag}.xml"
      [ "$xr_oss_url" == "https://git.codelinaro.org" ] && xr_oss_au_xml="${xr_au_tag}.xml"
      if [ "$xr_oss_url" == "https://git.codelinaro.org" ]; then
         cp $ROOT_PWD/$XR_OSS_DIR_NAME/.repo/manifests/$xr_oss_au_xml $ROOT_PWD/.repo/local_manifests/clod_$xr_oss_au_xml
      else
         cp $ROOT_PWD/$XR_OSS_DIR_NAME/.repo/manifests/$xr_oss_au_xml $ROOT_PWD/.repo/local_manifests/
      fi
      echo "TECHPACK XR OSS Manifest copied as local manifest to local_manifests"
    fi
    found_xr_au=true
    if [ "$found_cv_au" = true ] ; then
      cv_oss_au_xml="caf_${cv_au_tag}.xml"
      [ "$cv_oss_url" == "https://git.codelinaro.org" ] && cv_oss_au_xml="${cv_au_tag}.xml"
      if [ "$cv_oss_url" == "https://git.codelinaro.org" ]; then
         cp $ROOT_PWD/$CV_OSS_DIR_NAME/.repo/manifests/$cv_oss_au_xml $ROOT_PWD/.repo/local_manifests/clod_$cv_oss_au_xml
      else
         cp $ROOT_PWD/$CV_OSS_DIR_NAME/.repo/manifests/$cv_oss_au_xml $ROOT_PWD/.repo/local_manifests/
      fi
      echo "TECHPACK CV OSS Manifest copied as local manifest to local_manifests"
    fi
    if [ "$found_video_au" = true ] ; then
      video_oss_au_xml="caf_${video_au_tag}.xml"
      [ "$video_oss_url" == "https://git.codelinaro.org" ] && video_oss_au_xml="${video_au_tag}.xml"
      if [ "$video_oss_url" == "https://git.codelinaro.org" ]; then
         cp $ROOT_PWD/$VIDEO_OSS_DIR_NAME/.repo/manifests/$video_oss_au_xml $ROOT_PWD/.repo/local_manifests/clod_$video_oss_au_xml
      else
         cp $ROOT_PWD/$VIDEO_OSS_DIR_NAME/.repo/manifests/$video_oss_au_xml $ROOT_PWD/.repo/local_manifests/
      fi
      echo "TECHPACK VIDEO OSS Manifest copied as local manifest to local_manifests"
    fi
  fi

  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
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

    if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]
    then
      cp $ROOT_PWD/$QSSI_PROP_DIR_NAME/.repo/manifests/${qssi_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
      echo "QSSI Grease Manifest copied as local manifest to Vendor"
      if [ "$found_kernel_au" = true ] ; then
        cp $ROOT_PWD/$KERNEL_PROP_DIR_NAME/.repo/manifests/${kernel_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
        echo "Kernel Grease Manifest copied as local manifest to Vendor"
      fi
    fi
    if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ] ; then
      if [ "$found_display_au" = true ] ; then
        cp $ROOT_PWD/$DISPLAY_PROP_DIR_NAME/.repo/manifests/${display_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
        echo "TECHPACK DISPLAY Grease Manifest copied as local manifest to Vendor"
      fi
      if [ "$found_camera_au" = true ] ; then
        cp $ROOT_PWD/$CAMERA_PROP_DIR_NAME/.repo/manifests/${camera_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
        echo "TECHPACK CAMERA Grease Manifest copied as local manifest to Vendor"
      fi
      if [ "$found_graphics_au" = true ] ; then
        cp $ROOT_PWD/$GRAPHICS_PROP_DIR_NAME/.repo/manifests/${graphics_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
        echo "TECHPACK GRAPHICS Grease Manifest copied as local manifest to Vendor"
      fi
      if [ "$found_audio_au" = true ] ; then
        cp $ROOT_PWD/$AUDIO_PROP_DIR_NAME/.repo/manifests/${audio_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
        echo "TECHPACK AUDIO Grease Manifest copied as local manifest to Vendor"
      fi

      if [ "$tree_type" != $LA_COMBINED_TREE_GROUPS ]; then
        if [ "$found_sensor_au" = true ] ; then
         cp $ROOT_PWD/$SENSOR_PROP_DIR_NAME/.repo/manifests/${sensor_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
         echo "TECHPACK SENSOR Grease Manifest copied as local manifest to Vendor"
        fi
        if [ "$found_xr_au" = true ] ; then
         cp $ROOT_PWD/$XR_PROP_DIR_NAME/.repo/manifests/${xr_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
         echo "TECHPACK XR Grease Manifest copied as local manifest to Vendor"
        fi
        if [ "$found_cv_au" = true ] ; then
         cp $ROOT_PWD/$CV_PROP_DIR_NAME/.repo/manifests/${cv_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
         echo "TECHPACK CV Grease Manifest copied as local manifest to Vendor"
        fi
        if [ "$found_video_au" = true ] ; then
         cp $ROOT_PWD/$VIDEO_PROP_DIR_NAME/.repo/manifests/${video_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
         echo "TECHPACK VIDEO Grease Manifest copied as local manifest to Vendor"
        fi
      fi
    fi

    if [[ "$prop_dest" == $PROP_DEST_GREASE ]];then
      echo "getting prebuilts"
      # Test Grease Gerrit server connection
      set +e
      ssh -p 29418 ${grease_userid}@${grease_server}
      if [ $? -ne 127 ]; then
        echo "Please check your grease user name and password for correctness."
        usage
      fi
      set -e
      pushd $ROOT_PWD
      vendor_cdr_customer=$(dirname ${vendor_grease_branch})
      vendor_tar_file_gz="prebuilt_${vendor_cdr_customer}_${vendor_au_tag}.tar.gz"
      vendor_tar_file_xz="prebuilt_${vendor_cdr_customer}_${vendor_au_tag}.tar.xz"
      vendor_parent_branch=`echo ${vendor_grease_branch} | cut -d '/' -f2`
      vendor_tar_file_on_server_gz="https://${grease_server}/binaries/outgoing/${vendor_cdr_customer}/${vendor_parent_branch}/${vendor_tar_file_gz}"
      vendor_tar_file_on_server_xz="https://${grease_server}/binaries/outgoing/${vendor_cdr_customer}/${vendor_parent_branch}/${vendor_tar_file_xz}"
     ( wget --continue --no-check-certificate --user=$grease_userid \
         --password=$grease_pass $vendor_tar_file_on_server_gz ||
       wget --continue --no-check-certificate --user=$grease_userid \
         --password=$grease_pass $vendor_tar_file_on_server_xz)
    fi
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
    mkdir -p  $ROOT_PWD/$PROP_PATH
    if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]
    then
      if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]
      then
        if chipcode_args_check "qssi" "$qssi_chipcode_path"
        then
          cp -r $qssi_chipcode_path/$PROP_PATH/* $ROOT_PWD/$PROP_PATH/
        fi
        if [ "$found_kernel_au" = true ] ; then
          if chipcode_args_check "kernel" "$kernel_chipcode_path"
          then
            mkdir -p  $ROOT_PWD/$KERNEL_PLATFORM_PATH
            cp -r $kernel_chipcode_path/* $ROOT_PWD/$KERNEL_PLATFORM_PATH/
          fi
        fi
      fi
      if [ "$found_display_au" = true ] ; then
         if chipcode_args_check "display" "$display_chipcode_path"
         then
            mkdir -p  $ROOT_PWD/$DISPLAY_PLATFORM_PATH
            cp -r $display_chipcode_path/* $ROOT_PWD/$DISPLAY_PLATFORM_PATH/
         fi
      fi
      if [ "$found_camera_au" = true ] ; then
         if chipcode_args_check "camera" "$camera_chipcode_path"
         then
            mkdir -p  $ROOT_PWD/$CAMERA_PLATFORM_PATH
            cp -r $camera_chipcode_path/* $ROOT_PWD/$CAMERA_PLATFORM_PATH/
         fi
      fi
      if [ "$found_graphics_au" = true ] ; then
         if chipcode_args_check "graphics" "$graphics_chipcode_path"
         then
            mkdir -p  $ROOT_PWD/$GRAPHICS_PLATFORM_PATH
            cp -r $graphics_chipcode_path/* $ROOT_PWD/$GRAPHICS_PLATFORM_PATH/
         fi
      fi
      if [ "$found_audio_au" = true ] ; then
         if chipcode_args_check "audio" "$audio_chipcode_path"
         then
            mkdir -p  $ROOT_PWD/$AUDIO_PLATFORM_PATH
            cp -r $audio_chipcode_path/* $ROOT_PWD/$AUDIO_PLATFORM_PATH/
         fi
      fi
      if [ "$found_sensor_au" = true ] ; then
         if chipcode_args_check "sensor" "$sensor_chipcode_path"
         then
            mkdir -p  $ROOT_PWD/$SENSOR_PLATFORM_PATH
            cp -r $sensor_chipcode_path/* $ROOT_PWD/$SENSOR_PLATFORM_PATH/
         fi
      fi
      if [ "$found_xr_au" = true ] ; then
         if chipcode_args_check "xr" "$xr_chipcode_path"
         then
            mkdir -p  $ROOT_PWD/$XR_PLATFORM_PATH
            cp -r $xr_chipcode_path/* $ROOT_PWD/$XR_PLATFORM_PATH/
         fi
      fi
      if [ "$found_cv_au" = true ] ; then
         if chipcode_args_check "cv" "$cv_chipcode_path"
         then
            mkdir -p  $ROOT_PWD/$CV_PLATFORM_PATH
            cp -r $cv_chipcode_path/* $ROOT_PWD/$CV_PLATFORM_PATH/
         fi
      fi
      if [ "$found_video_au" = true ] ; then
         if chipcode_args_check "video" "$video_chipcode_path"
         then
            mkdir -p  $ROOT_PWD/$VIDEO_PLATFORM_PATH
            cp -r $video_chipcode_path/* $ROOT_PWD/$VIDEO_PLATFORM_PATH/
         fi
      fi
      if [ ! -z $groups ] && [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
         remove_common_sys_prjs
      fi
    fi
    cp -r $vendor_chipcode_path/$PROP_PATH $ROOT_PWD/vendor/qcom/
  fi
  rm -rf $ROOT_PWD/$VENDOR_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$VENDOR_OSS_DIR_NAME
  rm -rf $ROOT_PWD/$QSSI_OSS_DIR_NAME
  rm -rf $ROOT_PWD/$QSSI_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$KERNEL_OSS_DIR_NAME
  rm -rf $ROOT_PWD/$KERNEL_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$DISPLAY_OSS_DIR_NAME
  rm -rf $ROOT_PWD/$DISPLAY_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$CAMERA_OSS_DIR_NAME
  rm -rf $ROOT_PWD/$CAMERA_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$GRAPHICS_OSS_DIR_NAME
  rm -rf $ROOT_PWD/$GRAPHICS_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$AUDIO_OSS_DIR_NAME
  rm -rf $ROOT_PWD/$AUDIO_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$SENSOR_OSS_DIR_NAME
  rm -rf $ROOT_PWD/$SENSOR_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$XR_OSS_DIR_NAME
  rm -rf $ROOT_PWD/$XR_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$CV_OSS_DIR_NAME
  rm -rf $ROOT_PWD/$CV_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$VIDEO_OSS_DIR_NAME
  rm -rf $ROOT_PWD/$VIDEO_PROP_DIR_NAME
  echo "Tree sync initiated"
  pushd $ROOT_PWD
  sleep 2
  repo manifest > $COMBINED_MANIFEST
  echo "Combined Manifest generated"
  if [ ! -z "$optional_sync_arg" ]; then
    echo "repo sync -j${jobs} $optional_sync_arg"
    repo sync -j${jobs} $optional_sync_arg
  else
    echo "repo sync -j${jobs}"
    repo sync -j${jobs}
  fi
  echo "repo sync successful"

  if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
    echo "extracting prebuilts"
    if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]
    then
      if [ -e prebuilt_${qssi_cdr_customer}_${qssi_au_tag}.tar.gz ]; then
        tar -zxvf prebuilt_${qssi_cdr_customer}_${qssi_au_tag}.tar.gz
      else
        tar -xvf prebuilt_${qssi_cdr_customer}_${qssi_au_tag}.tar.xz
      fi
    fi
    if [ -e prebuilt_${vendor_cdr_customer}_${vendor_au_tag}.tar.gz ]; then
      tar -zxvf prebuilt_${vendor_cdr_customer}_${vendor_au_tag}.tar.gz
    else
      tar -xvf prebuilt_${vendor_cdr_customer}_${vendor_au_tag}.tar.xz
    fi
  fi

  if [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]
  then
    if [ "$tree_type" == $LA_COMBINED_TREE ]; then
      echo "LA_COMBINED_TREE is ready"
    fi
    if [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
      echo "LA_COMBINED_TREE with groups option is ready"
    fi
    if [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]; then
      echo "LA_VENDOR_TECHPACK_TREE is ready"
    fi
  else
    echo "VENDOR_STANDALONE_TREE is ready"
  fi

fi #"$tree_type" = $LA_COMBINED_TREE

if [ "$tree_type" == $QSSI_TREE_STANDALONE ]; then
  mkdir -p $ROOT_PWD/.repo/local_manifests
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then

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
  if [ ! -z "$optional_sync_arg" ]; then
    echo "repo sync -j${jobs} $optional_sync_arg"
    repo sync -j${jobs} $optional_sync_arg
  else
    echo "repo sync -j${jobs}"
    repo sync -j${jobs}
  fi
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
  rm -rf $ROOT_PWD/$QSSI_OSS_DIR_NAME
  echo "QSSI_STANDALONE_TREE is ready"
fi #"$tree_type" = $QSSI_TREE_STANDALONE

if [ "$tree_type" == $KERNEL_TREE_STANDALONE ]; then
  mkdir -p $ROOT_PWD/.repo/local_manifests
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then

    cp $ROOT_PWD/$KERNEL_PROP_DIR_NAME/.repo/manifests/${kernel_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
    echo "KERNEL SI Grease Manifest copied as local manifest to Vendor"
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
    mkdir -p  $ROOT_PWD/$KERNEL_PLATFORM_PATH
    cp -r $kernel_chipcode_path/* $ROOT_PWD/$KERNEL_PLATFORM_PATH/
  fi
  pushd $ROOT_PWD
  sleep 2
  repo manifest > $COMBINED_MANIFEST
  echo "Combined Manifest generated"
  if [ ! -z "$optional_sync_arg" ]; then
    echo "repo sync -j${jobs} $optional_sync_arg"
    repo sync -j${jobs} $optional_sync_arg
  else
    echo "repo sync -j${jobs}"
    repo sync -j${jobs}
  fi
  echo "repo sync successful"

  # commenting this part as there are no binary deliverables
  #if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
  #  echo "extracting prebuilts"
  #  if [ -e prebuilt_${kernel_cdr_customer}_${kernel_au_tag}.tar.gz ]; then
  #    tar -zxvf prebuilt_${kernel_cdr_customer}_${kernel_au_tag}.tar.gz
  #  else
  #    tar -xvf prebuilt_${kernel_cdr_customer}_${kernel_au_tag}.tar.xz
  #  fi
  #fi
  rm -rf $ROOT_PWD/$KERNEL_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$KERNEL_OSS_DIR_NAME
  echo "KERNEL_STANDALONE_TREE is ready"
fi #"$tree_type" = $KERNEL_TREE_STANDALONE

if [ "$tree_type" == $DISPLAY_TREE_STANDALONE ]; then
  mkdir -p $ROOT_PWD/.repo/local_manifests
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then

    cp $ROOT_PWD/$DISPLAY_PROP_DIR_NAME/.repo/manifests/${display_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
    echo "TECHPACK DISPLAY SI Grease Manifest copied as local manifest to Vendor"
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
    mkdir -p  $ROOT_PWD/$DISPLAY_PLATFORM_PATH
    cp -r $display_chipcode_path/* $ROOT_PWD/$DISPLAY_PLATFORM_PATH/
  fi
  pushd $ROOT_PWD
  sleep 2
  repo manifest > $COMBINED_MANIFEST
  echo "Combined Manifest generated"
  if [ ! -z "$optional_sync_arg" ]; then
    echo "repo sync -j${jobs} $optional_sync_arg"
    repo sync -j${jobs} $optional_sync_arg
  else
    echo "repo sync -j${jobs}"
    repo sync -j${jobs}
  fi
  echo "repo sync successful"

  # commenting this part as there are no binary deliverables
  # CHECK IT for TECHPACK
  #if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
  #  echo "extracting prebuilts"
  #  if [ -e prebuilt_${display_cdr_customer}_${display_au_tag}.tar.gz ]; then
  #    tar -zxvf prebuilt_${display_cdr_customer}_${display_au_tag}.tar.gz
  #  else
  #    tar -xvf prebuilt_${display_cdr_customer}_${display_au_tag}.tar.xz
  #  fi
  #fi
  rm -rf $ROOT_PWD/$DISPLAY_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$DISPLAY_OSS_DIR_NAME
  echo "DISPLAY_STANDALONE_TREE is ready"
fi #"$tree_type" = $DISPLAY_TREE_STANDALONE

if [ "$tree_type" == $CAMERA_TREE_STANDALONE ]; then
  mkdir -p $ROOT_PWD/.repo/local_manifests
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then

    cp $ROOT_PWD/$CAMERA_PROP_DIR_NAME/.repo/manifests/${camera_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
    echo "TECHPACK CAMERA SI Grease Manifest copied as local manifest to Vendor"
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
    mkdir -p  $ROOT_PWD/$CAMERA_PLATFORM_PATH
    cp -r $camera_chipcode_path/* $ROOT_PWD/$CAMERA_PLATFORM_PATH/
  fi
  pushd $ROOT_PWD
  sleep 2
  repo manifest > $COMBINED_MANIFEST
  echo "Combined Manifest generated"
  if [ ! -z "$optional_sync_arg" ]; then
    echo "repo sync -j${jobs} $optional_sync_arg"
    repo sync -j${jobs} $optional_sync_arg
  else
    echo "repo sync -j${jobs}"
    repo sync -j${jobs}
  fi
  echo "repo sync successful"

  # commenting this part as there are no binary deliverables
  # CHECK IT for TECHPACK
  #if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
  #  echo "extracting prebuilts"
  #  if [ -e prebuilt_${camera_cdr_customer}_${camera_au_tag}.tar.gz ]; then
  #    tar -zxvf prebuilt_${camera_cdr_customer}_${camera_au_tag}.tar.gz
  #  else
  #    tar -xvf prebuilt_${camera_cdr_customer}_${camera_au_tag}.tar.xz
  #  fi
  #fi
  rm -rf $ROOT_PWD/$CAMERA_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$CAMERA_OSS_DIR_NAME
  echo "CAMERA_STANDALONE_TREE is ready"
fi #"$tree_type" = $CAMERA_TREE_STANDALONE

if [ "$tree_type" == $GRAPHICS_TREE_STANDALONE ]; then
  mkdir -p $ROOT_PWD/.repo/local_manifests
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then

    cp $ROOT_PWD/$GRAPHICS_PROP_DIR_NAME/.repo/manifests/${graphics_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
    echo "TECHPACK GRAPHICS SI Grease Manifest copied as local manifest to Vendor"
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
    mkdir -p  $ROOT_PWD/$GRAPHICS_PLATFORM_PATH
    cp -r $graphics_chipcode_path/* $ROOT_PWD/$GRAPHICS_PLATFORM_PATH/
  fi
  pushd $ROOT_PWD
  sleep 2
  repo manifest > $COMBINED_MANIFEST
  echo "Combined Manifest generated"
  if [ ! -z "$optional_sync_arg" ]; then
    echo "repo sync -j${jobs} $optional_sync_arg"
    repo sync -j${jobs} $optional_sync_arg
  else
    echo "repo sync -j${jobs}"
    repo sync -j${jobs}
  fi
  echo "repo sync successful"

  # commenting this part as there are no binary deliverables
  # CHECK IT for TECHPACK
  #if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
  #  echo "extracting prebuilts"
  #  if [ -e prebuilt_${graphics_cdr_customer}_${graphics_au_tag}.tar.gz ]; then
  #    tar -zxvf prebuilt_${graphics_cdr_customer}_${graphics_au_tag}.tar.gz
  #  else
  #    tar -xvf prebuilt_${graphics_cdr_customer}_${graphics_au_tag}.tar.xz
  #  fi
  #fi
  rm -rf $ROOT_PWD/$GRAPHICS_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$GRAPHICS_OSS_DIR_NAME
  echo "GRAPHICS_STANDALONE_TREE is ready"
fi #"$tree_type" = $GRAPHICS_TREE_STANDALONE

if [ "$tree_type" == $AUDIO_TREE_STANDALONE ]; then
  mkdir -p $ROOT_PWD/.repo/local_manifests
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then

    cp $ROOT_PWD/$AUDIO_PROP_DIR_NAME/.repo/manifests/${audio_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
    echo "TECHPACK AUDIO SI Grease Manifest copied as local manifest to Vendor"
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
    mkdir -p  $ROOT_PWD/$AUDIO_PLATFORM_PATH
    cp -r $audio_chipcode_path/* $ROOT_PWD/$AUDIO_PLATFORM_PATH/
  fi
  pushd $ROOT_PWD
  sleep 2
  repo manifest > $COMBINED_MANIFEST
  echo "Combined Manifest generated"
  if [ ! -z "$optional_sync_arg" ]; then
    echo "repo sync -j${jobs} $optional_sync_arg"
    repo sync -j${jobs} $optional_sync_arg
  else
    echo "repo sync -j${jobs}"
    repo sync -j${jobs}
  fi
  echo "repo sync successful"

  # commenting this part as there are no binary deliverables
  # CHECK IT for TECHPACK
  #if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
  #  echo "extracting prebuilts"
  #  if [ -e prebuilt_${audio_cdr_customer}_${audio_au_tag}.tar.gz ]; then
  #    tar -zxvf prebuilt_${audio_cdr_customer}_${audio_au_tag}.tar.gz
  #  else
  #    tar -xvf prebuilt_${audio_cdr_customer}_${audio_au_tag}.tar.xz
  #  fi
  #fi
  rm -rf $ROOT_PWD/$AUDIO_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$AUDIO_OSS_DIR_NAME
  echo "AUDIO_STANDALONE_TREE is ready"
fi #"$tree_type" = $AUDIO_TREE_STANDALONE


if [ "$tree_type" == $SENSOR_TREE_STANDALONE ]; then
  mkdir -p $ROOT_PWD/.repo/local_manifests
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then

    cp $ROOT_PWD/$SENSOR_PROP_DIR_NAME/.repo/manifests/${sensor_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
    echo "TECHPACK SENSOR SI Grease Manifest copied as local manifest to Vendor"
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
    mkdir -p  $ROOT_PWD/$SENSOR_PLATFORM_PATH
    cp -r $sensor_chipcode_path/* $ROOT_PWD/$SENSOR_PLATFORM_PATH/
  fi
  pushd $ROOT_PWD
  sleep 2
  if [[ "$tree_type" == $SENSOR_TREE_STANDALONE && "$prop_dest" == $PROP_DEST_CHIPCODE ]]; then
     echo "Do nothing"
  else
     if [[ "$tree_type" == $SENSOR_TREE_STANDALONE && "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
         rm -rf .repo
         cp -rf $ROOT_PWD/$SENSOR_PROP_DIR_NAME/.repo ./
         echo "Do nothing"
     else
         repo manifest > $COMBINED_MANIFEST
         echo "Combined Manifest generated"
     fi
     if [ ! -z "$optional_sync_arg" ]; then
         echo "repo sync -j${jobs} $optional_sync_arg"
         repo sync -j${jobs} $optional_sync_arg
     else
         echo "repo sync -j${jobs}"
         repo sync -j${jobs}
     fi
     echo "repo sync successful"
  fi
  # commenting this part as there are no binary deliverables
  # CHECK IT for TECHPACK
  #if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
  #  echo "extracting prebuilts"
  #  if [ -e prebuilt_${sensor_cdr_customer}_${sensor_au_tag}.tar.gz ]; then
  #    tar -zxvf prebuilt_${sensor_cdr_customer}_${sensor_au_tag}.tar.gz
  #  else
  #    tar -xvf prebuilt_${sensor_cdr_customer}_$sensor_au_tag}.tar.xz
  #  fi
  #fi
  rm -rf $ROOT_PWD/$SENSOR_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$SENSOR_OSS_DIR_NAME
  echo "SENSOR_STANDALONE_TREE is ready"
fi #"$tree_type" = $SENSOR_TREE_STANDALONE

if [ "$tree_type" == $XR_TREE_STANDALONE ]; then
  mkdir -p $ROOT_PWD/.repo/local_manifests
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then

    cp $ROOT_PWD/$XR_PROP_DIR_NAME/.repo/manifests/${xr_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
    echo "TECHPACK XR SI Grease Manifest copied as local manifest to Vendor"
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
    mkdir -p  $ROOT_PWD/$XR_PLATFORM_PATH
    cp -r $xr_chipcode_path/* $ROOT_PWD/$XR_PLATFORM_PATH/
  fi
  pushd $ROOT_PWD
  sleep 2
  if [[ "$tree_type" == $XR_TREE_STANDALONE && "$prop_dest" == $PROP_DEST_CHIPCODE ]]; then
     echo "Do nothing"
  else
     if [[ "$tree_type" == $XR_TREE_STANDALONE && "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then
         rm -rf .repo
         cp -rf $ROOT_PWD/$XR_PROP_DIR_NAME/.repo ./
         echo "Do nothing"
     else
         repo manifest > $COMBINED_MANIFEST
         echo "Combined Manifest generated"
     fi
     if [ ! -z "$optional_sync_arg" ]; then
         echo "repo sync -j${jobs} $optional_sync_arg"
         repo sync -j${jobs} $optional_sync_arg
     else
         echo "repo sync -j${jobs}"
         repo sync -j${jobs}
     fi
     echo "repo sync successful"
  fi
  # commenting this part as there are no binary deliverables
  # CHECK IT for TECHPACK
  #if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
  #  echo "extracting prebuilts"
  #  if [ -e prebuilt_${xr_cdr_customer}_${xr_au_tag}.tar.gz ]; then
  #    tar -zxvf prebuilt_${xr_cdr_customer}_${xr_au_tag}.tar.gz
  #  else
  #    tar -xvf prebuilt_${xr_cdr_customer}_$xr_au_tag}.tar.xz
  #  fi
  #fi
  rm -rf $ROOT_PWD/$XR_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$XR_OSS_DIR_NAME
  echo "XR_STANDALONE_TREE is ready"
fi #"$tree_type" = $XR_TREE_STANDALONE

if [ "$tree_type" == $CV_TREE_STANDALONE ]; then
  mkdir -p $ROOT_PWD/.repo/local_manifests
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then

    cp $ROOT_PWD/$CV_PROP_DIR_NAME/.repo/manifests/${cv_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
    echo "TECHPACK CV SI Grease Manifest copied as local manifest to Vendor"
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
    mkdir -p  $ROOT_PWD/$CV_PLATFORM_PATH
    cp -r $cv_chipcode_path/* $ROOT_PWD/$CV_PLATFORM_PATH/
  fi
  pushd $ROOT_PWD
  sleep 2
  repo manifest > $COMBINED_MANIFEST
  echo "Combined Manifest generated"
  if [ ! -z "$optional_sync_arg" ]; then
    echo "repo sync -j${jobs} $optional_sync_arg"
    repo sync -j${jobs} $optional_sync_arg
  else
    echo "repo sync -j${jobs}"
    repo sync -j${jobs}
  fi
  echo "repo sync successful"

  # commenting this part as there are no binary deliverables
  # CHECK IT for TECHPACK
  #if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
  #  echo "extracting prebuilts"
  #  if [ -e prebuilt_${cv_cdr_customer}_${cv_au_tag}.tar.gz ]; then
  #    tar -zxvf prebuilt_${cv_cdr_customer}_${cv_au_tag}.tar.gz
  #  else
  #    tar -xvf prebuilt_${cv_cdr_customer}_${cv_au_tag}.tar.xz
  #  fi
  #fi
  rm -rf $ROOT_PWD/$CV_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$CV_OSS_DIR_NAME
  echo "CV_STANDALONE_TREE is ready"
fi #"$tree_type" = $CV_TREE_STANDALONE


if [ "$tree_type" == $VIDEO_TREE_STANDALONE ]; then
  mkdir -p $ROOT_PWD/.repo/local_manifests
  if [[ "$prop_dest" == $PROP_DEST_GREASE || "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]]; then

    cp $ROOT_PWD/$VIDEO_PROP_DIR_NAME/.repo/manifests/${video_au_tag}.xml $ROOT_PWD/.repo/local_manifests/
    echo "TECHPACK VIDEO SI Grease Manifest copied as local manifest to Vendor"
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
    mkdir -p  $ROOT_PWD/$VIDEO_PLATFORM_PATH
    cp -r $video_chipcode_path/* $ROOT_PWD/$VIDEO_PLATFORM_PATH/
  fi
  pushd $ROOT_PWD
  sleep 2
  repo manifest > $COMBINED_MANIFEST
  echo "Combined Manifest generated"
  if [ ! -z "$optional_sync_arg" ]; then
    echo "repo sync -j${jobs} $optional_sync_arg"
    repo sync -j${jobs} $optional_sync_arg
  else
    echo "repo sync -j${jobs}"
    repo sync -j${jobs}
  fi
  echo "repo sync successful"

  # commenting this part as there are no binary deliverables
  # CHECK IT for TECHPACK
  #if [ "$prop_dest" == $PROP_DEST_GREASE ]; then
  #  echo "extracting prebuilts"
  #  if [ -e prebuilt_${video_cdr_customer}_${video_au_tag}.tar.gz ]; then
  #    tar -zxvf prebuilt_${video_cdr_customer}_${video_au_tag}.tar.gz
  #  else
  #    tar -xvf prebuilt_${video_cdr_customer}_${video_au_tag}.tar.xz
  #  fi
  #fi
  rm -rf $ROOT_PWD/$VIDEO_PROP_DIR_NAME
  rm -rf $ROOT_PWD/$VIDEO_OSS_DIR_NAME
  echo "VIDEO_STANDALONE_TREE is ready"
fi #"$tree_type" = $VIDEO_TREE_STANDALONE

if [ "$prop_dest" == $PROP_DEST_CHIPCODE_HF ]; then
   if [ "$tree_type" == $QSSI_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
      cp -rf $ROOT_PWD/system_prebuilt_dir/vendor/qcom/proprietary/prebuilt_HY11 $ROOT_PWD/vendor/qcom/proprietary/
      rm -rf $ROOT_PWD/system_prebuilt_dir || true
   fi
   if [ "$tree_type" == $KERNEL_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]; then
      if [ -d "$ROOT_PWD/kernel_prebuilt_dir" ]; then
         cp -rf $ROOT_PWD/kernel_prebuilt_dir/kernel_platform/qcom/proprietary/prebuilt_HY11 $ROOT_PWD/kernel_platform/qcom/proprietary/
         rm -rf $ROOT_PWD/kernel_prebuilt_dir || true
      fi
   fi
   if [ "$tree_type" == $DISPLAY_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]; then
      if [ -d "$ROOT_PWD/display_prebuilt_dir" ]; then
         # Disabled copy of display_prebuilt_dir, since it is empty, it is having dummy Android.mk
         #cp -rf $ROOT_PWD/display_prebuilt_dir/vendor/qcom/proprietary/prebuilt_HY11 $ROOT_PWD/vendor/qcom/proprietary/
         rm -rf $ROOT_PWD/display_prebuilt_dir || true
      fi
   fi
   if [ "$tree_type" == $CAMERA_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]|| [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]; then
      if [ -d "$ROOT_PWD/camera_prebuilt_dir" ]; then
         # Disabled copy of camera_prebuilt_dir, since it is empty, it is having dummy Android.mk
         #cp -rf $ROOT_PWD/camera_prebuilt_dir/vendor/qcom/proprietary/prebuilt_HY11 $ROOT_PWD/vendor/qcom/proprietary/
         rm -rf $ROOT_PWD/camera_prebuilt_dir || true
      fi
   fi
   if [ "$tree_type" == $GRAPHICS_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]|| [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]; then
      if [ -d "$ROOT_PWD/graphics_prebuilt_dir" ]; then
         # Disabled copy of graphics_prebuilt_dir, since it is empty, it is having dummy Android.mk
         #cp -rf $ROOT_PWD/graphics_prebuilt_dir/vendor/qcom/proprietary/prebuilt_HY11 $ROOT_PWD/vendor/qcom/proprietary/
         rm -rf $ROOT_PWD/graphics_prebuilt_dir || true
      fi
   fi
   if [ "$tree_type" == $AUDIO_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]|| [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]; then
      if [ -d "$ROOT_PWD/audio_prebuilt_dir" ]; then
         # Disabled copy of audio_prebuilt_dir, since it is empty, it is having dummy Android.mk
         #cp -rf $ROOT_PWD/audio_prebuilt_dir/vendor/qcom/proprietary/prebuilt_HY11 $ROOT_PWD/vendor/qcom/proprietary/
         rm -rf $ROOT_PWD/audio_prebuilt_dir || true
      fi
   fi
   if [ "$tree_type" == $SENSOR_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]|| [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]; then
      if [ -d "$ROOT_PWD/sensor_prebuilt_dir" ]; then
         # Disabled copy of sensor_prebuilt_dir, since it is empty, it is having dummy Android.mk
         #cp -rf $ROOT_PWD/sensor_prebuilt_dir/vendor/qcom/proprietary/prebuilt_HY11 $ROOT_PWD/vendor/qcom/proprietary/
         rm -rf $ROOT_PWD/sensor_prebuilt_dir || true
      fi
   fi
   if [ "$tree_type" == $XR_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]|| [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]; then
      if [ -d "$ROOT_PWD/xr_prebuilt_dir" ]; then
         # Disabled copy of xr_prebuilt_dir, since it is empty, it is having dummy Android.mk
         #cp -rf $ROOT_PWD/xr_prebuilt_dir/vendor/qcom/proprietary/prebuilt_HY11 $ROOT_PWD/vendor/qcom/proprietary/
         rm -rf $ROOT_PWD/xr_prebuilt_dir || true
      fi
   fi
   if [ "$tree_type" == $CV_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]|| [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]; then
      if [ -d "$ROOT_PWD/cv_prebuilt_dir" ]; then
         # Disabled copy of cv_prebuilt_dir, since it is empty, it is having dummy Android.mk
         #cp -rf $ROOT_PWD/cv_prebuilt_dir/vendor/qcom/proprietary/prebuilt_HY11 $ROOT_PWD/vendor/qcom/proprietary/
         rm -rf $ROOT_PWD/cv_prebuilt_dir || true
      fi
   fi
   if [ "$tree_type" == $VIDEO_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ]|| [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]; then
      if [ -d "$ROOT_PWD/video_prebuilt_dir" ]; then
         # Disabled copy of video_prebuilt_dir, since it is empty, it is having dummy Android.mk
         #cp -rf $ROOT_PWD/video_prebuilt_dir/vendor/qcom/proprietary/prebuilt_HY11 $ROOT_PWD/vendor/qcom/proprietary/
         rm -rf $ROOT_PWD/video_prebuilt_dir || true
      fi
   fi
   if [ "$tree_type" == $VENDOR_TREE_STANDALONE ] || [ "$tree_type" == $LA_COMBINED_TREE ] || [ "$tree_type" == $LA_COMBINED_TREE_GROUPS ] || [ "$tree_type" == $LA_VENDOR_TECHPACK_TREE ]; then
      if [ -d "$ROOT_PWD/vendor_prebuilt_dir" ]; then
          cp -rf $ROOT_PWD/vendor_prebuilt_dir/vendor/qcom/proprietary/prebuilt_HY11 $ROOT_PWD/vendor/qcom/proprietary/
          rm -rf $ROOT_PWD/vendor_prebuilt_dir || true
      fi
   fi
fi


