@echo off
setlocal enabledelayedexpansion

:: ==========================================
:: DETECT CMAKE AND CONFIGURING MINGW
:: ==========================================
:: 1. Detect CMake
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    if exist "C:\Program Files\CMake\bin" (
        set "PATH=C:\Program Files\CMake\bin;%PATH%"
    ) else if exist "C:\Program Files (x86)\CMake\bin" (
        set "PATH=C:\Program Files (x86)\CMake\bin;%PATH%"
    ) else (
        echo [ERROR] CMake was not found in PATH or default installation directories.
        echo Please make sure CMake is properly installed.
        exit /b 1
    )
)

:: 2. Force the path of your new MinGW distribution
:: NOTE: If you extracted MinGW to a folder other than C:\mingw64, change the path below:
set "MINGW_BIN=C:\mingw64\bin"
set "PATH=%MINGW_BIN%;%PATH%"

:: 3. Define the direct absolute compiler paths for CMake
set "MY_CC=%MINGW_BIN%\gcc.exe"
set "MY_CXX=%MINGW_BIN%\g++.exe"

:: ==========================================
:: CONFIGURATION OF PATHS AND PARAMETERS
:: ==========================================
set "PROJECT_DIR=%~dp0"
set "BUILD_DIR=%PROJECT_DIR%build-dist"
set "VCPKG_TOOLCHAIN=%PROJECT_DIR%engine\vcpkg\scripts\buildsystems\vcpkg.cmake"
set "VCPKG_TRIPLET=x64-mingw-dynamic"
set "BUILD_TYPE=RelWithDebInfo"

echo ====================================================
echo  Starting Independent Build for Leadwort
echo ====================================================
echo Root Directory: %PROJECT_DIR%
echo Build Type:     %BUILD_TYPE%
echo Vcpkg Triplet:   %VCPKG_TRIPLET%
echo ====================================================

:: 1. Check if vcpkg.exe exists (run Bootstrap if required)
if not exist "%PROJECT_DIR%engine\vcpkg\vcpkg.exe" (
    echo [vcpkg] vcpkg.exe not found. Running bootstrap...
    pushd "%PROJECT_DIR%engine\vcpkg"
    call bootstrap-vcpkg.bat
    if !errorlevel! neq 0 (
        echo [ERROR] vcpkg bootstrap failed. Aborting.
        popd
        exit /b 1
    )
    popd
)

:: 2. Create the build directory if it doesn't exist
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

:: 3. Configure the project with CMake and Ninja
echo.
echo [CMake] Configuring the build system...

cmake -G Ninja -S . -B "%BUILD_DIR%" ^
      -DCMAKE_C_COMPILER="%MY_CC%" ^
      -DCMAKE_CXX_COMPILER="%MY_CXX%" ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCMAKE_TOOLCHAIN_FILE=%VCPKG_TOOLCHAIN% ^
      -DVCPKG_TARGET_TRIPLET=%VCPKG_TRIPLET% ^
      -DVCPKG_MANIFEST_MODE=ON ^
      -DVCPKG_MANIFEST_DIR="%PROJECT_DIR%engine"

if !errorlevel! neq 0 (
    echo [ERROR] CMake configuration failed.
    exit /b 1
)

:: 4. Build the project
echo.
echo [CMake] Building the project...
cmake --build "%BUILD_DIR%" --config %BUILD_TYPE%

if !errorlevel! neq 0 (
    echo [ERROR] Build failed.
    exit /b 1
)

echo.
echo ====================================================
echo  Build completed successfully in: %BUILD_DIR%
echo ====================================================

:: 5. Execute the compiled application
if exist "%BUILD_DIR%\application\Leadwort.exe" (
    "%BUILD_DIR%\application\Leadwort.exe"
) else (
    echo [WARNING] Executable target not found in expected binary directories.
    pause
)