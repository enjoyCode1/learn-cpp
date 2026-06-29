$files = @(
    'D:\learn\C++\code\data_structures\advanced\ring_buffer.cpp',
    'D:\learn\C++\code\data_structures\advanced\time_wheel.cpp'
)
$enc = New-Object System.Text.UTF8Encoding($true)
foreach ($f in $files) {
    $txt = Get-Content $f -Raw -Encoding UTF8
    [System.IO.File]::WriteAllText($f, $txt, $enc)
    Write-Host "BOM ok: $f"
}
