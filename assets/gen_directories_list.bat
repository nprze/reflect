@echo off
setlocal enabledelayedexpansion

set "root=%cd%"

>directories.txt (
    for /d /r %%D in (*) do (
        set "rel=%%D"
        set "rel=!rel:%root%\=!"
        set "rel=!rel:\=/!"
        echo !rel!
    )
)

echo directories.txt
pause
