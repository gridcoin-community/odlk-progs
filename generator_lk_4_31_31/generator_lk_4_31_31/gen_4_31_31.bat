@echo off
set /a total = 0
for /f %%i in (names.txt) do (
echo **********************************************
generator_lk_4_31_31.exe %%i
set /a total += 1
)
echo **********************************************
echo.
set total
echo.
pause