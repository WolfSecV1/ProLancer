@echo off
setlocal

:: --- CONFIG ---
set APP_NAME=Lancer
set BUILD_DIR=out\build\release
set DEPLOY_DIR=deploy
set VERSION_FILE=assets\version.txt
set QT_BIN=C:\Qt\6.9.1\msvc2022_64\bin

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
"%QT_BIN%\windeployqt.exe" %DEPLOY_DIR%\%APP_NAME%.exe --release --dir %DEPLOY_DIR%

:: --- ZIP THE FOLDER ---
powershell -Command "Compress-Archive -Path '%DEPLOY_DIR%\*' -DestinationPath '%APP_NAME%_%VERSION%.zip'"

echo ✅ Build and packaging complete: %APP_NAME%_%VERSION%.zip
pause
