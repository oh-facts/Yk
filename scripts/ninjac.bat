@echo off
pushd ..\.out\

set "startTime=!TIME!"

powershell -Command "$startTime = Get-Date; ninja; $endTime = Get-Date; $elapsedTime = $endTime - $startTime; Write-Output ('{0:N2}' -f $elapsedTime.TotalSeconds)" > ninja_output.txt

echo Ninja Output:
type ninja_output.txt
echo.

popd
