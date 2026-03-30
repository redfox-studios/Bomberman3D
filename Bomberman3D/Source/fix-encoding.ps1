# i created this file because visual studio kept converting my source files from UTF-8 to UTF-16 LE BOM

Get-ChildItem -Recurse -Include *.cpp,*.h | ForEach-Object {
    $bytes = [System.IO.File]::ReadAllBytes($_.FullName)
    # Strip UTF-16 LE BOM (FF FE) and convert properly
    if ($bytes[0] -eq 0xFF -and $bytes[1] -eq 0xFE) {
        $content = [System.Text.Encoding]::Unicode.GetString($bytes, 2, $bytes.Length - 2)
        [System.IO.File]::WriteAllText($_.FullName, $content, [System.Text.UTF8Encoding]::new($false))
        Write-Host "Converted: $($_.FullName)"
    }
}