# Build, run the checks, then run every play test against the rebuild, one at a
# time. Prints one line per scenario and the tail of any that fails.
#
#   tools\play.ps1 > somewhere\play.log 2>&1      (about 10 minutes)
#
# The play tests take over the mouse and keyboard: never run two at once, and
# don't change the main tree while this runs.
Set-Location (Split-Path $PSScriptRoot -Parent)
& .\tools\check.ps1 2>&1 | Select-Object -Last 8
if ($LASTEXITCODE -ne 0) { Write-Output "CHECK FAILED"; exit 1 }
$failed = 0
foreach ($f in (Get-ChildItem tests\e2e\scenarios\test_s*.py | Sort-Object Name)) {
    $out = & C:\iwd2-tools\venv\Scripts\python.exe -m pytest $f.FullName --target reimpl -q 2>&1
    $last = ($out | Select-Object -Last 1)
    Write-Output "$($f.BaseName): $last"
    if ($LASTEXITCODE -ne 0) { $failed++; $out | Select-Object -Last 40 }
}
exit $failed
