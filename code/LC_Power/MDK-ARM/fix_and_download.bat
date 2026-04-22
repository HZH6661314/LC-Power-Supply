@echo off
chcp 65001 >nul
echo ========================================
echo  STM32F334 完整修复和下载流程
echo ========================================
echo.

cd /d %~dp0

echo [步骤1/5] 连接测试...
openocd -f openocd.cfg -c "init; exit" 2>nul
if errorlevel 1 (
    echo 错误：无法连接！请检查硬件连接
    pause
    exit /b 1
)
echo ✓ 连接正常

echo.
echo [步骤2/5] 停止目标...
openocd -f openocd.cfg -c "init; reset halt; exit" 2>nul
echo ✓ 目标已停止

echo.
echo [步骤3/5] 解锁Flash...
openocd -f openocd.cfg -c "init; reset halt; stm32f3x unlock 0; shutdown" 2>nul
timeout /t 2 >nul
echo ✓ Flash已解锁

echo.
echo [步骤4/5] 擦除Flash...
openocd -f openocd.cfg -c "init; reset halt; flash erase_address 0x08000000 0x10000; exit"
echo ✓ Flash已擦除

echo.
echo [步骤5/5] 下载程序...
if exist "build\LC_Power.elf" (
    echo 使用文件：build\LC_Power.elf
    openocd -f openocd.cfg -c "init; reset halt; flash write_image erase build/LC_Power.elf; verify_image build/LC_Power.elf; reset run; exit"
) else if exist "LC_Power\LC_Power.hex" (
    echo 使用文件：LC_Power\LC_Power.hex
    openocd -f openocd.cfg -c "init; reset halt; flash write_image erase LC_Power/LC_Power.hex; verify_image LC_Power/LC_Power.hex; reset run; exit"
) else (
    echo 错误：找不到编译输出文件！
    echo 请先编译项目
    pause
    exit /b 1
)

if errorlevel 1 (
    echo.
    echo ✗ 下载失败！
    echo.
    echo 请尝试：
    echo 1. 检查目标板供电
    echo 2. 重新连接CMSIS-DAP
    echo 3. 查看详细日志
    pause
    exit /b 1
)

echo.
echo ========================================
echo  ✓ 成功！程序已下载并运行
echo ========================================
pause
