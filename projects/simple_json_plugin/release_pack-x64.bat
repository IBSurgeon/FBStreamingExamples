set PROJECT_NAME=simple_json_plugin
set VERSION=1.6.0
set ARCH=Win-x64
set ARCHIVATOR_DIR=C:\Program Files\7-Zip
set WORK_DIR=%1
set OUTPUT_DIR=%WORK_DIR%build
set DOC_DIR=%WORK_DIR%doc
set BUILD_DIR=%WORK_DIR%build\%PROJECT_NAME%\windows-x64\Release
set PROJECT_DIR=%WORK_DIR%projects\%PROJECT_NAME%
set TMP_PACK_DIR=%BUILD_DIR%\tmp_pack_dir
set ARCHIVE_FILENAME=%PROJECT_NAME%-%VERSION%-%ARCH%.zip

del "%OUTPUT_DIR%\%ARCHIVE_FILENAME%" /q

mkdir "%TMP_PACK_DIR%"
mkdir "%TMP_PACK_DIR%\stream_plugins"
mkdir "%TMP_PACK_DIR%\doc"

copy "%BUILD_DIR%\simple_json_plugin.dll" "%TMP_PACK_DIR%\stream_plugins\simple_json_plugin.dll"
copy "%DOC_DIR%\simple_json_plugin.md" "%TMP_PACK_DIR%\doc\simple_json_plugin.md"
copy "%DOC_DIR%\simple_json_plugin_ru.md" "%TMP_PACK_DIR%\doc\simple_json_plugin_ru.md"
copy "%PROJECT_DIR%\fb_streaming.conf" "%TMP_PACK_DIR%\fb_streaming.conf"

"%ARCHIVATOR_DIR%\7z.exe" a -tzip -r0 "%OUTPUT_DIR%\%ARCHIVE_FILENAME%" "%TMP_PACK_DIR%/*"

rmdir "%TMP_PACK_DIR%" /s /q
