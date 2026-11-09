$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot

$compiler = Get-Command g++ -CommandType Application -ErrorAction Stop | Select-Object -First 1

$sources = @(
    'src/main.cpp'
    'src/app/SchedulingService.cpp'
    'src/cli/Console.cpp'
    'src/core/Scheduler.cpp'
    'src/core/SchedulingRules.cpp'
    'src/core/SessionGenerator.cpp'
    'src/core/Validator.cpp'
    'src/data/DemoData.cpp'
    'src/io/TimetableCsv.cpp'
    'tests/SelfTests.cpp'
)

Push-Location $projectRoot
try {
    New-Item -ItemType Directory -Force -Path 'build' | Out-Null
    & $compiler.Source -std=c++17 -Wall -Wextra -Wpedantic -I include -I tests @sources -o 'build/timetablex.exe'
    if ($LASTEXITCODE -ne 0) {
        throw "g++ failed with exit code $LASTEXITCODE"
    }
    Write-Host 'Built build/timetablex.exe'
}
finally {
    Pop-Location
}
