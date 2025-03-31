#!/bin/bash -
# Getting the Trusted Package Creator CLI path
SCRIPT=$(readlink -f $0)
project_dir=`dirname $SCRIPT`
echo $project_dir
cd "$project_dir/../../../../ROT_Provisioning"
provisioningdir=$(pwd)
cd $project_dir
source $provisioningdir/env_sonlyapp.sh "$provisioningdir"

# Environment variable for log file
current_log_file="$project_dir/postbuild.log"
echo "" > $current_log_file

error()
{
    echo ""
    echo "====="
    echo "===== Error occurred."
    echo "===== See the $current_log_file for details. Then try again."
    echo "====="
    exit 1
}


# arg1 is the config type (Debug, Release)
config=$1

preprocess_bl2_file="$project_dir/image_macros_preprocessed_bl2.c"

appli_dir="../../../../$oemirot_boot_path_project"
update="$project_dir/../../../../ROT_Provisioning/OEMiROT_SOnlyApp/ob_flash_programming.sh"
provisioning="$project_dir/../../../../ROT_Provisioning/OEMiROT_SOnlyApp/img_config.sh"
s_main="$appli_dir/Inc/main.h"
map_properties="$project_dir/../../OEMiROT_Boot/map.properties"
s_code_xml="$project_dir/../../../../ROT_Provisioning/OEMiROT_SOnlyApp/Images/OEMiROT_S_Code_Image.xml"
s_data_xml="$project_dir/../../../../ROT_Provisioning/OEMiROT_SOnlyApp/Images/OEMiROT_S_Data_Image.xml"
s_code_init_xml="$project_dir/../../../../ROT_Provisioning/OEMiROT_SOnlyApp/Images/OEMiROT_S_Code_Init_Image.xml"
s_data_init_xml="$project_dir/../../../../ROT_Provisioning/OEMiROT_SOnlyApp/Images/OEMiROT_S_Data_Init_Image.xml"
if [ $2 == "primary_only" ]; then
s_code_xml="$project_dir/../../../../ROT_Provisioning/OEMiROT_SOnlyApp/Images/OEMiROT_Code_Image.xml"
s_data_xml="$project_dir/../../../../ROT_Provisioning/OEMiROT_SOnlyApp/Images/OEMiROT_Data_Image.xml"
s_code_init_xml="$project_dir/../../../../ROT_Provisioning/OEMiROT_SOnlyApp/Images/OEMiROT_Code_Init_Image.xml"
s_data_init_xml="$project_dir/../../../../ROT_Provisioning/OEMiROT_SOnlyApp/Images/OEMiROT_Data_Init_Image.xml"
fi
appli_flash_layout="$appli_dir/Secure_nsclib/appli_flash_layout.h"
appli_postbuild="$appli_dir/STM32CubeIDE/postbuild.sh"

auth_s="Authentication secure key"

xml_fw_app_item_name="Firmware binary input file"
xml_fw_data_item_name="Data binary input file"
xml_output_item_name="Image output file"
xml_enc_item_name="Encryption key"
code_size="Firmware area size"
data_size="Data download slot size"
scratch_sector_number="Number of scratch sectors"
firmware_execution_offset="Firmware execution area offset"

s_code_bin="$appli_dir/Binary/rot_tz_s_app.bin"

s_data_bin="../Binary/s_data.bin"
ns_data_bin="../Binary/ns_data.bin"

applicfg="$cube_fw_path/Utilities/PC_Software/ROT_AppliConfig/dist/AppliCfg.exe"
uname | grep -i -e windows -e mingw
if [ $? == 0 ] && [ -e "$applicfg" ]; then
  #line for window executable
  echo "AppliCfg with windows executable"
  python=""
else
  #line for python
  echo "AppliCfg with python script"
  applicfg="$cube_fw_path/Utilities/PC_Software/ROT_AppliConfig/AppliCfg.py"
  #determine/check python version command
  python="python3 "
fi

$python$applicfg flash --layout $preprocess_bl2_file -b S_CODE_REGION_START -m  RE_ADDRESS_SECURE_START --vb $map_properties >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b S_CODE_REGION_SIZE -m  RE_IMAGE_FLASH_SECURE_IMAGE_SIZE --vb $map_properties >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b S_DATA -m RE_S_DATA_IMAGE_NUMBER --vb $map_properties >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b CODE_IMAGE_ASSEMBLY -m RE_CODE_IMAGE_ASSEMBLY --vb $map_properties >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b MCUBOOT_OVERWRITE_ONLY -m RE_OVER_WRITE $map_properties --vb >> $current_log_file
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b CMSE_VENEER_REGION_SIZE -m RE_CMSE_VENEER_REGION_SIZE $map_properties --vb >> $current_log_file
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b S_DATA_REGION_START -m RE_AREA_4_OFFSET $map_properties --vb >> $current_log_file
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b S_DATA_REGION_SIZE -m RE_AREA_4_SIZE $map_properties --vb >> $current_log_file
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b ROT_REGION_START -m RE_FLASH_AREA_BL2_OFFSET $map_properties --vb >> $current_log_file
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b ROT_REGION_SIZE -m RE_FLASH_AREA_BL2_SIZE $map_properties --vb >> $current_log_file
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b SCRATCH_REGION_START -m RE_FLASH_AREA_SCRATCH_OFFSET $map_properties --vb >> $current_log_file
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b SCRATCH_REGION_SIZE -m RE_FLASH_AREA_SCRATCH_SIZE $map_properties --vb >> $current_log_file
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b DOWNLOAD_S_CODE_REGION_START -m RE_AREA_2_OFFSET $map_properties --vb >> $current_log_file
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b DOWNLOAD_S_DATA_REGION_START -m RE_AREA_6_OFFSET $map_properties --vb >> $current_log_file
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b bootob -m  RE_BL2_BOOT_ADDRESS  -d 0x100 --vb $update >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file  -b bootaddress -m  RE_BL2_BOOT_ADDRESS --vb $update >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b sec1_end -m  RE_BL2_SEC1_END -d 0x2000 --vb $update >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b sec2_start -m  RE_BL2_SEC2_START -d 0x2000 --vb $update >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b sec2_end -m  RE_BL2_SEC2_END -d 0x2000 --vb $update >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg setob --layout $preprocess_bl2_file -b wrpgrp1 -ms RE_BL2_WRP_START -me RE_BL2_WRP_END -msec RE_FLASH_PAGE_NBR -d 0x8000 --vb $update >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg setob --layout $preprocess_bl2_file -b wrpgrp2 -ms RE_BL2_WRP_START -me RE_BL2_WRP_END -msec RE_FLASH_PAGE_NBR -d 0x8000 --vb $update >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg setob --layout $preprocess_bl2_file -b hdp1_end -ms RE_BL2_HDP_START -me RE_BL2_HDP_END -msec RE_FLASH_PAGE_NBR -d 0x2000 --vb $update >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg setob --layout $preprocess_bl2_file -b hdp2_start -ms RE_BL2_HDP_START -me RE_BL2_HDP_END -msec RE_FLASH_PAGE_NBR -d 0x2000 --vb $update >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg setob --layout $preprocess_bl2_file -b hdp2_end -ms RE_BL2_HDP_START -me RE_BL2_HDP_END -msec RE_FLASH_PAGE_NBR -d 0x2000 --vb $update >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b s_data_image_number -m  RE_S_DATA_IMAGE_NUMBER --decimal --vb $update >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b loader_image_number -m  RE_LOADER_IMAGE_NUMBER --decimal --vb $update >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b app_image_number -m  RE_APP_IMAGE_NUMBER --decimal --vb $appli_postbuild >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b app_image_number -m  RE_APP_IMAGE_NUMBER --decimal --vb $provisioning >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b s_data_image_number -m  RE_S_DATA_IMAGE_NUMBER --decimal --vb $provisioning >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b loader_image_number -m  RE_LOADER_IMAGE_NUMBER --decimal --vb $provisioning >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b image_s_size -m  RE_IMAGE_FLASH_SECURE_IMAGE_SIZE --vb $appli_postbuild >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg xmlval --layout $preprocess_bl2_file -m RE_IMAGE_FLASH_SECURE_UPDATE -c x --vb $s_code_xml >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg xmlval --layout $preprocess_bl2_file -m RE_IMAGE_FLASH_DATA_SECURE_UPDATE -c x --vb $s_data_xml >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg xmlval --layout $preprocess_bl2_file -m RE_IMAGE_FLASH_SECURE_IMAGE_SIZE -c S --vb $s_code_xml >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg xmlval --layout $preprocess_bl2_file -m RE_IMAGE_FLASH_SECURE_DATA_IMAGE_SIZE -c S --vb $s_data_xml >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg definevalue --layout $preprocess_bl2_file -m RE_AREA_0_OFFSET -n S_CODE_OFFSET --vb $s_main >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg definevalue --layout $preprocess_bl2_file -m RE_IMAGE_FLASH_SECURE_IMAGE_SIZE -n S_CODE_SIZE --vb $s_main >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg xmlparam --layout  $preprocess_bl2_file -m RE_ENCRYPTION -n "Encryption key" -link GetPublic -t File -c -E -h 1 -d "" --vb $s_code_xml >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg xmlparam --layout  $preprocess_bl2_file -m RE_ENCRYPTION -n "Encryption key" -link GetPublic -t File -c -E -h 1 -d "" --vb $s_data_xml >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg xmlparam --layout  $preprocess_bl2_file -m RE_OVER_WRITE -n "Write Option" -t Data -c --overwrite-only -h 1 -d "" --vb $s_code_xml >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg xmlparam --layout  $preprocess_bl2_file -m RE_OVER_WRITE -n "Write Option" -t Data -c --overwrite-only -h 1 -d "" --vb $s_data_xml >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg flash --layout $preprocess_bl2_file -b FLASH_SIZE -m RE_FLASH_SIZE $map_properties --vb >> $current_log_file

if [ $2 != "primary_only" ]; then
    $python"$applicfg" xmlval --layout "$preprocess_bl2_file" -m RE_FLASH_AREA_SCRATCH_SIZE -n "$scratch_sector_number" --decimal "$s_code_xml" --vb >> "$current_log_file"
    if [ $? != 0 ]; then error; fi

    $python"$applicfg" xmlval -xml "$s_code_xml" -nxml "$code_size" -nxml "$scratch_sector_number" --decimal -e "(((val1+1)/val2)+1)" -cond "val2" -c M "$s_code_xml" --vb >> "$current_log_file"
    if [ $? != 0 ]; then error; fi

    $python"$applicfg" xmlval --layout "$preprocess_bl2_file" -m RE_FLASH_AREA_SCRATCH_SIZE -n "$scratch_sector_number" --decimal "$s_data_xml" --vb >> "$current_log_file"
    if [ $? != 0 ]; then error; fi

    $python"$applicfg" xmlval -xml "$s_data_xml" -nxml "$data_size" -nxml "$scratch_sector_number" --decimal -e "(((val1+1)/val2)+1)" -cond "val2" -c M "$s_data_xml" --vb >> "$current_log_file"
    if [ $? != 0 ]; then error; fi
fi

# Bypass configuration of appli_flash_layout file if not present
if [ -f $appli_flash_layout ]; then

    $python$applicfg setdefine --layout $preprocess_bl2_file -m RE_OVER_WRITE -n MCUBOOT_OVERWRITE_ONLY -v 1 --vb $appli_flash_layout  >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg definevalue --layout $preprocess_bl2_file -m RE_APP_IMAGE_NUMBER -n MCUBOOT_APP_IMAGE_NUMBER --vb $appli_flash_layout >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg definevalue --layout $preprocess_bl2_file -m RE_S_DATA_IMAGE_NUMBER -n MCUBOOT_S_DATA_IMAGE_NUMBER --vb $appli_flash_layout >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg definevalue --layout $preprocess_bl2_file -m RE_AREA_0_OFFSET -n FLASH_AREA_0_OFFSET --vb $appli_flash_layout >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg definevalue --layout $preprocess_bl2_file -m RE_AREA_0_SIZE -n FLASH_AREA_0_SIZE --vb $appli_flash_layout >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg definevalue --layout $preprocess_bl2_file -m RE_AREA_2_OFFSET -n FLASH_AREA_2_OFFSET --vb $appli_flash_layout >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg definevalue --layout $preprocess_bl2_file -m RE_AREA_2_SIZE -n FLASH_AREA_2_SIZE --vb $appli_flash_layout >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg definevalue --layout $preprocess_bl2_file -m RE_AREA_4_OFFSET -n FLASH_AREA_4_OFFSET --vb $appli_flash_layout >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg definevalue --layout $preprocess_bl2_file -m RE_AREA_4_SIZE -n FLASH_AREA_4_SIZE --vb $appli_flash_layout >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg definevalue --layout $preprocess_bl2_file -m RE_AREA_6_OFFSET -n FLASH_AREA_6_OFFSET --vb $appli_flash_layout >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg definevalue --layout $preprocess_bl2_file -m RE_AREA_6_SIZE -n FLASH_AREA_6_SIZE --vb $appli_flash_layout >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg definevalue --layout $preprocess_bl2_file -m RE_S_NS_PARTITION_SIZE -n FLASH_PARTITION_SIZE --vb $appli_flash_layout >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg definevalue --layout $preprocess_bl2_file -m RE_IMAGE_NON_SECURE_IMAGE_SIZE -n FLASH_NS_PARTITION_SIZE --vb $appli_flash_layout >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg definevalue --layout $preprocess_bl2_file -m RE_IMAGE_FLASH_SECURE_IMAGE_SIZE -n FLASH_S_PARTITION_SIZE --vb $appli_flash_layout >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg definevalue --layout $preprocess_bl2_file -m RE_IMAGE_FLASH_SECURE_DATA_IMAGE_SIZE -n FLASH_S_DATA_PARTITION_SIZE --vb $appli_flash_layout >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

	$python$applicfg definevalue --layout $preprocess_bl2_file -m RE_FLASH_B_SIZE -n FLASH_B_SIZE --vb $appli_flash_layout >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

fi

cp $project_dir/$config/OEMiROT_Boot.bin $project_dir/../Binary/OEMiROT_Boot.bin >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

#xml for init image generation

cp $s_code_xml $s_code_init_xml >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

cp $s_data_xml $s_data_init_xml >> $current_log_file 2>&1

if [ $? != 0 ]; then error; fi
    if [ $2 != "primary_only" ]; then
    $python$applicfg xmlparam --option add -n "Clear" -t Data -c -c -h 1 -d "" $s_code_init_xml --vb >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg xmlparam --option add -n "Confirm" -t Data -c --confirm -h 1 -d "" $s_code_init_xml --vb >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi
fi

$python$applicfg xmlname -n "$firmware_execution_offset" -c x $s_code_init_xml --vb >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg xmlval --layout $preprocess_bl2_file -m RE_IMAGE_FLASH_ADDRESS_SECURE -c x $s_code_init_xml --vb >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

if [ $2 != "primary_only" ]; then
    $python$applicfg xmlparam --option add -n "Clear" -t Data -c -c -h 1 -d "" $s_data_init_xml --vb >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    $python$applicfg xmlparam --option add -n "Confirm" -t Data -c --confirm -h 1 -d "" $s_data_init_xml --vb >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi
fi
$python$applicfg xmlname -n "$firmware_execution_offset" -c x $s_data_init_xml --vb >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

$python$applicfg xmlval --layout $preprocess_bl2_file -m RE_IMAGE_FLASH_ADDRESS_DATA_SECURE -c x $s_data_init_xml --vb >> $current_log_file 2>&1
if [ $? != 0 ]; then error; fi

exit 0
