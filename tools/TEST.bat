@echo off

python -c "import diffimg" > nul 2> nul
if not errorlevel 0 (
    echo "diffimg" not found. Please install `pip install diffimg`.
    echo See: https://github.com/nicolashahn/diffimg
    pause
    exit /b 1
)

cd /d "%~dp0\.."

call make cook

mkdir tests > nul 2> nul
mkdir tests\out > nul 2> nul
mkdir tests\ref > nul 2> nul
mkdir tests\diff > nul 2> nul

SETLOCAL EnableDelayedExpansion

for %%x in (*.exe) do (
    
    echo Running %%x...
    start /wait "" "%%x" --capture=50 --mute --cook-jobs=0

    if not exist "tests\ref\%%~nx.png" (
        echo [ ok ] reference image not found. Copying %%~nx.png ...
        copy "tests\out\%%~nx.png" "tests\ref\%%~nx.png" 1>nul 2>nul
    ) else (
        set command=python -m diffimg "tests\ref\%%~nx.png" "tests\out\%%~nx.png" --ratio --filename "tests\diff\%%~nx.png"
        for /f "tokens=*" %%a in ('!command!') do (
            if %%a0 gtr 0.040 (
                echo [fail] %~dp0\..\tests\diff\%%~nx.png ^(Score: %%a^)
            ) else (
                echo [ ok ] %%~nx.exe ^(Score: %%a^)
                del "tests\diff\%%~nx.png"
            )
        )
    )
)
