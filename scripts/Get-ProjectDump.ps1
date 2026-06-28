param(
    [string]$ProjectRoot = (Resolve-Path "$PSScriptRoot\..").Path,
    [string]$OutputFile = "project_dump.txt"
)

$extensions = @(
    ".cpp",
    ".h",
    ".glsl",
    ".txt",
    ".md"
)

$excludedDirectories = @(
    "thirdparty",
    "vcpkg",
    "build"
)

$outputPath = Join-Path $ProjectRoot $OutputFile

Write-Host "Project root: $ProjectRoot"
Write-Host "Output file: $outputPath"
Write-Host "Searching for files..."

if (Test-Path $outputPath) {
    Write-Host "Removing existing output file..."
    Remove-Item $outputPath
}

$files = Get-ChildItem `
    -Path $ProjectRoot `
    -Recurse `
    -File |
        Where-Object {
            $parts = $_.FullName.Split('\')

            $exclude =
            ($parts -contains "thirdparty") -or
                    ($parts -contains "vcpkg") -or
                    ($parts -contains "build") -or
                    ($parts | Where-Object { $_ -like "cmake-build*" })

            (-not $exclude) -and ($extensions -contains $_.Extension.ToLower())
        } |
        Sort-Object FullName

Write-Host "Found $($files.Count) files."
Write-Host ""

$current = 0

foreach ($file in $files) {
    $current++

    $relativePath = Resolve-Path -Relative $file.FullName

    Write-Host "[$current/$($files.Count)] Processing $relativePath"

    Add-Content -Path $outputPath -Value "================================================================================"
    Add-Content -Path $outputPath -Value "FILE: $relativePath"
    Add-Content -Path $outputPath -Value "================================================================================"
    Add-Content -Path $outputPath -Value ""

    try {
        Get-Content $file.FullName -Raw -Encoding UTF8 | Add-Content -Path $outputPath
    }
    catch {
        Write-Warning "Failed to read $relativePath"
        Add-Content -Path $outputPath -Value "<ERROR READING FILE>"
    }

    Add-Content -Path $outputPath -Value ""
    Add-Content -Path $outputPath -Value ""
}

Write-Host ""
Write-Host "Dump completed successfully."
Write-Host "Output: $outputPath"
Write-Host "Files processed: $($files.Count)"