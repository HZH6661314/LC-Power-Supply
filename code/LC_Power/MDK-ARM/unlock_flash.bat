@echo off
chcp 65001 >nul
echo ========================================
echo  STM32F334 Flash解锁和擦除工具
echo ========================================
echo.

cd /d %~dp0

echo 警告：此操作将擦除整个Flash！
echo 按任意键继续，或关闭窗口取消...
pause >nul

echo.
echo [1/3] 连接目标...
openocd -f openocd.cfg -c "init; reset halt; exit" 2>nul
if errorlevel 1 (
    echo 错误：无法连接到目标！
    pause
    exit /b 1
)
echo 连接成功！

echo.
echo [2/3] 解锁Flash...
openocd -f openocd.cfg -c "init; reset halt; stm32f3x unlock 0; exit"
echo Flash已解锁

echo.
echo [3/3] 擦除Flash...
openocd -f openocd.cfg -c "init; reset halt; flash erase_sector 0 0 last; exit"
echo Flash已擦除

echo.
echo ========================================
echo  完成！现在可以尝试下载程序了
echo  运行 download.bat 下载程序
echo ========================================
pause
