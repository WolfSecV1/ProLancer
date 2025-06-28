@echo off
setlocal

:: --- CONFIG ---
set APP_NAME=Lancer
set BUILD_DIR=out\build\debug
set DEPLOY_DIR=deploy
set VERSION_FILE=assets\version.txt
set QT_BIN=C:\Qt2\6.9.1\mingw_64\bin

:: --- READ VERSION ---
set /p VERSION=<%VERSION_FILE%
echo Building %APP_NAME% %VERSION%

:: --- CLEAN DEPLOY FOLDER ---
rmdir /s /q %DEPLOY_DIR%
mkdir %DEPLOY_DIR%

:: --- COPY EXE + ASSETS ---
copy %BUILD_DIR%\%APP_NAME%.exe %DEPLOY_DIR%
xcopy assets %DEPLOY_DIR%\assets /E /I /Y

:: --- RUN WINDEPLOYQT ---
"%QT_BIN%\windeployqt.exe" %DEPLOY_DIR%\%APP_NAME%.exe --debug --force --compiler-runtime

:: --- MANUALLY COPY PLATFORMS ---
mkdir %DEPLOY_DIR%\platforms
copy "C:\Qt2\6.9.1\mingw_64\plugins\platforms\qwindows.dll" %DEPLOY_DIR%\platforms\

:: --- ZIP THE FOLDER ---
powershell -Command "Compress-Archive -Path '%DEPLOY_DIR%\*' -DestinationPath '%APP_NAME%_v%VERSION%.zip'"

echo Build and packaging complete: %APP_NAME%_%VERSION%.zip
pause