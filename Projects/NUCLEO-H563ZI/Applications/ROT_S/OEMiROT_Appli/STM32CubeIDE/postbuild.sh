#!/bin/bash -
# Getting the Trusted Package Creator CLI path
SCRIPT=$(readlink -f $0)
project_dir=`dirname $SCRIPT`
cd "$project_dir/../../../../ROT_Provisioning"
provisioningdir=$(pwd)
cd $project_dir
source "$provisioningdir/env_sonlyapp.sh" "$provisioningdir"

error()
{
    echo ""
    echo "====="
    echo "===== Error occurred."
    echo "===== See $current_log_file for details. Then try again."
    echo "====="
    exit 1
}

# Environment variable for log file
current_log_file="$project_dir/postbuild.log"
echo "" > $current_log_file

# arg1 is the binary type (1 nonsecure, 2 secure)
signing=$1
# arg2 is the config type (Debug, Release)
config=$2

s_code_xml="$provisioningdir/OEMiROT_SOnlyApp/Images/OEMiROT_S_Code_Image.xml"
s_code_init_xml="$provisioningdir/OEMiROT_SOnlyApp/Images/OEMiROT_S_Code_Init_Image.xml"
s_data_xml="$provisioningdir/OEMiROT_SOnlyApp/Images/OEMiROT_S_Data_Image.xml"
s_data_init_xml="$provisioningdir/OEMiROT_SOnlyApp/Images/OEMiROT_S_Data_Init_Image.xml"
bin_dest_dir="$project_dir/../Binary"
s_code_bin="$project_dir/../Binary/rot_tz_s_app.bin"

# Variables for image xml configuration
appli_dir="../../../Applications/ROT_S/OEMiROT_Appli"
fw_out_bin="Image output file"
fw_in_bin="Firmware binary input file"
s_app_bin="$appli_dir/Binary/rot_tz_s_app.bin"
s_app_enc_sign_hex="$appli_dir/Binary/rot_tz_s_app_enc_sign.hex"
s_app_enc_sign_bin="$appli_dir/Binary/rot_tz_s_app_enc_sign.bin"
s_app_init_sign_hex="$appli_dir/Binary/rot_tz_s_app_init_sign.hex"
s_data_enc_sign_hex="$provisioningdir/OEMiROT_SOnlyApp/Binary/s_data_enc_sign.hex"
s_data_enc_sign_bin="$provisioningdir/OEMiROT_SOnlyApp/Binary/s_data_enc_sign.bin"
s_data_init_sign_hex="$provisioningdir/OEMiROT_SOnlyApp/Binary/s_data_init_sign.hex"

#Variables updated by OEMiROT_Boot postbuild
app_image_number=1
image_s_size=0x2A000

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

echo "Postbuild $signing image" >> $current_log_file 2>&1

if  [ $signing == "secure" ]; then
    echo "Creating secure image"  >> $current_log_file 2>&1

    # update xml file : input file
    $python$applicfg xmlval -v $s_app_bin --string -n "$fw_in_bin" $s_code_xml --vb >> $current_log_file 2>&1
    if [ $? != "0" ]; then step_error; fi
    # update xml file : output file
    $python$applicfg xmlval -v $s_app_enc_sign_hex --string -n "$fw_out_bin" $s_code_xml --vb >> $current_log_file 2>&1
    if [ $? != "0" ]; then step_error; fi

    "$stm32tpccli" -pb $s_code_xml >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    # update xml file : input file
    $python$applicfg xmlval -v $s_app_bin --string -n "$fw_in_bin" $s_code_init_xml --vb >> $current_log_file 2>&1
    if [ $? != "0" ]; then step_error; fi
    # update xml file : output file
    $python$applicfg xmlval -v $s_app_init_sign_hex --string -n "$fw_out_bin" $s_code_init_xml --vb >> $current_log_file 2>&1
    if [ $? != "0" ]; then step_error; fi

    "$stm32tpccli" -pb $s_code_init_xml >> $current_log_file 2>&1
    if [ $? != 0 ]; then error; fi

    # update xml file : output file
    $python$applicfg xmlval -v $s_data_enc_sign_hex --string -n "$fw_out_bin" $s_data_xml --vb >> $current_log_file 2>&1
    if [ $? != "0" ]; then step_error; fi
    $python$applicfg xmlval -v $s_data_init_sign_hex --string -n "$fw_out_bin" $s_data_init_xml --vb >> $current_log_file 2>&1
    if [ $? != "0" ]; then step_error; fi
fi


exit 0
