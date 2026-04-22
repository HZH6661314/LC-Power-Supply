@echo off
chcp 65001 >nul
echo ========================================
echo  STM32F334 程序下载脚本
echo ========================================
echo.

cd /d %~dp0

echo [1/4] 检查OpenOCD...
where openocd >nul 2>&1
if errorlevel 1 (
    echo 错误：找不到OpenOCD！
    echo 请确保OpenOCD已安装并添加到PATH
    pause
    exit /b 1
)
echo OpenOCD已找到

echo.
echo [2/4] 连接目标...
openocd -f openocd.cfg -c "init; halt; exit" 2>nul
if errorlevel 1 (
    echo 错误：无法连接到目标！
    echo.
    echo 请检查：
    echo   1. CMSIS-DAP是否连接到电脑
    echo   2. 目标板是否供电（3.3V）
    echo   3. SWDIO连接到PA13
    echo   4. SWCLK连接到PA14
    echo   5. GND连接正常
    echo.
    pause
    exit /b 1
)
echo 连接成功！

echo.
echo [3/4] 下载程序...
if exist "build\LC_Power.elf" (
    echo 使用ELF文件：build\LC_Power.elf
    openocd -f openocd.cfg -c "program build/LC_Power.elf verify reset exit"
) else if exist "LC_Power\LC_Power.hex" (
    echo 使用HEX文件：LC_Power\LC_Power.hex
    openocd -f openocd.cfg -c "program LC_Power/LC_Power.hex verify reset exit"
) else (
    echo 错误：找不到编译输出文件！
    echo 请先编译项目（Ctrl+Shift+B）
    pause
    exit /b 1
)

if errorlevel 1 (
    echo.
    echo 下载失败！尝试擦除Flash后重试...
    openocd -f openocd.cfg -c "init; halt; flash erase_sector 0 0 last; exit"
    if exist "build\LC_Power.elf" (
        openocd -f openocd.cfg -c "program build/LC_Power.elf verify reset exit"
    ) else (
        openocd -f openocd.cfg -c "program LC_Power/LC_Power.hex verify reset exit"
    )
)

echo.
echo [4/4] 完成！
echo ========================================
echo  程序已成功下载到STM32F334
echo  目标板应该开始运行
echo ========================================
pause
