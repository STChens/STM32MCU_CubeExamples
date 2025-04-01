@ECHO OFF
:: arg1 is the binary type (1 nonsecure, 2 secure)
set "signing=%1"

:: Getting the Trusted Package Creator CLI path
set "projectdir=%~dp0"
pushd %projectdir%\..\..\..\..\ROT_Provisioning
set provisioningdir=%cd%
popd
call "%provisioningdir%\env_sonlyapp.bat"

if "%~2"=="" (
    set "primary_only=false"
) else if "%2" NEQ "primary_only" (
    set "primary_only=true"
) else (
    set "primary_only=false"
)

:: Enable delayed expansion
setlocal EnableDelayedExpansion

:: Environment variable for log file
set current_log_file="%projectdir%\postbuild.log"
echo. > %current_log_file%

set s_code_xml="%provisioningdir%\OEMiROT_SOnlyApp\Images\OEMiROT_S_Code_Image.xml"
set s_code_init_xml="%provisioningdir%\OEMiROT_SOnlyApp\Images\OEMiROT_S_Code_Init_Image.xml"
set s_data_xml="%provisioningdir%\OEMiROT_SOnlyApp\Images\OEMiROT_S_Data_Image.xml"
set s_data_init_xml="%provisioningdir%\OEMiROT_SOnlyApp\Images\OEMiROT_S_Data_Init_Image.xml"
if "%primary_only%" == "true" (
    set s_code_xml="%provisioningdir%\OEMiROT_SOnlyApp\Images\OEMiROT_Code_Image.xml"
    set s_code_init_xml="%provisioningdir%\OEMiROT_SOnlyApp\Images\OEMiROT_Code_Init_Image.xml"
    set s_data_xml="%provisioningdir%\OEMiROT_SOnlyApp\Images\OEMiROT_Data_Image.xml"
    set s_data_init_xml="%provisioningdir%\OEMiROT_SOnlyApp\Images\OEMiROT_Data_Init_Image.xml"
)
set s_code_bin="%projectdir%\..\Binary\rot_tz_s_app.bin"

:: Variables for image xml configuration
set appli_dir="..\..\..\Applications\ROT_S\OEMiROT_Appli"
set fw_out_bin="Image output file"
set fw_in_bin="Firmware binary input file"
set s_app_bin="%appli_dir%\Binary\rot_tz_s_app.bin"
set s_app_enc_sign_hex="%appli_dir%\Binary\rot_tz_s_app_enc_sign.hex"
set s_app_enc_sign_bin="%appli_dir%\Binary\rot_tz_s_app_enc_sign.bin"
set s_app_init_sign_hex="%appli_dir%\Binary\rot_tz_s_app_init_sign.hex"
set s_data_enc_sign_hex="%provisioningdir%\OEMiROT_SOnlyApp\Binary\s_data_enc_sign.hex"
set s_data_enc_sign_bin="%provisioningdir%\OEMiROT_SOnlyApp\Binary\s_data_enc_sign.bin"
set s_data_init_sign_hex="%provisioningdir%\OEMiROT_SOnlyApp\Binary\s_data_init_sign.hex"

::Variables updated by OEMiROT_Boot postbuild
set app_image_number=1
set image_s_size=0x1D8000

:start
goto exe:
goto py:
:exe
::line for window executable
set "applicfg=%cube_fw_path%\Utilities\PC_Software\ROT_AppliConfig\dist\AppliCfg.exe"
set "python="
if exist %applicfg% (
goto postbuild
)
:py
::called if we just want to use AppliCfg python (think to comment "goto exe:")
set "applicfg=%cube_fw_path%\Utilities\PC_Software\ROT_AppliConfig\AppliCfg.py"
set "python=python "

:postbuild
echo Postbuild %signing% image >> %current_log_file% 2>>&1
:continue
IF %signing% == "secure" (
:: Create final image with Trust Package Creator
echo Creating OEMiROT image  >> %current_log_file% 2>>&1

::update xml file : input file
%python%%applicfg% xmlval -v %s_app_bin% --string -n %fw_in_bin% %s_code_xml% --vb >> %current_log_file% 2>>&1
if !errorlevel! neq 0 goto :error

::update xml file : output file
%python%%applicfg% xmlval -v %s_app_enc_sign_hex% --string -n %fw_out_bin% %s_code_xml% --vb >> %current_log_file% 2>>&1
if !errorlevel! neq 0 goto :error

%stm32tpccli% -pb %s_code_xml% >> %current_log_file% 2>>&1
if !errorlevel! neq 0 goto :error

::update xml file : input file
%python%%applicfg% xmlval -v %s_app_bin% --string -n %fw_in_bin% %s_code_init_xml% --vb >> %current_log_file% 2>>&1
if !errorlevel! neq 0 goto :error

::update xml file : output file
%python%%applicfg% xmlval -v %s_app_init_sign_hex% --string -n %fw_out_bin% %s_code_init_xml% --vb >> %current_log_file% 2>>&1
if !errorlevel! neq 0 goto :error

%stm32tpccli% -pb %s_code_init_xml% >> %current_log_file% 2>>&1
if !errorlevel! neq 0 goto :error

echo Creating OEMiROT data  >> %current_log_file% 2>>&1

::update data xml file : output file
%python%%applicfg% xmlval -v %s_data_enc_sign_hex% --string -n %fw_out_bin% %s_data_xml% --vb >> %current_log_file% 2>>&1
if !errorlevel! neq 0 goto :error
%python%%applicfg% xmlval -v %s_data_init_sign_hex% --string -n %fw_out_bin% %s_data_init_xml% --vb >> %current_log_file% 2>>&1
if !errorlevel! neq 0 goto :error
)

exit 0

:error
echo.
echo =====
echo ===== Error occurred.
echo ===== See %current_log_file% for details. Then try again.
echo =====
exit 1