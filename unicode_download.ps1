$url = "https://www.unicode.org/Public/14.0.0/ucd"
$emoji_url="$url/emoji/emoji-data.txt"

$files="CaseFolding.txt DerivedNormalizationProps.txt PropList.txt " +
"SpecialCasing.txt CompositionExclusions.txt ScriptExtensions.txt "  +
"UnicodeData.txt DerivedCoreProperties.txt NormalizationTest.txt Scripts.txt " +
"PropertyValueAliases.txt"

if (-not (Test-Path "unicode")) {
    New-Item -ItemType Directory -Path "unicode"
}

# Download the files
# space splited list of files
foreach($file in $files.Split(" ")) {
    $file_url="$url/$file"
    $file_path="unicode/$file"
    if (-not (Test-Path $file_path)) {
        Write-Output "Downloading $file_url to $file_path"
        Invoke-WebRequest -Uri $file_url -OutFile $file_path
    }
}

if (-not (Test-Path "unicode/emoji-data.txt")) {
    Write-Host "Downloading $emoji_url to unicode/emoji-data.txt"
    Invoke-WebRequest -Uri $emoji_url -OutFile "unicode/emoji-data.txt"
}

Write-Output "Done"
