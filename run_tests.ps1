# Initialize build environment
Write-Host 'Finding Visual Studio...' -ForegroundColor Magenta
$vsPath = .\Tools\vswhere.exe -latest -property installationPath
Write-Host $vsPath

Write-Host 'Importing environment variables...' -ForegroundColor Magenta
cmd.exe /c "call `"$vsPath\VC\Auxiliary\Build\vcvars64.bat`" && set > %temp%\vcvars.txt"
Get-Content "$env:temp\vcvars.txt" | ForEach-Object {
  if ($_ -match "^(.*?)=(.*)$") {
    Set-Content "env:\$($matches[1])" $matches[2]
  }
}

# Build and run tests
$coreCount = (Get-CimInstance -Class Win32_ComputerSystem).NumberOfLogicalProcessors
$configurations = "Debug", "Release"
$platform = "x64"

foreach ($config in $configurations) {
  Write-Host "Building tests $platform $config..." -ForegroundColor Magenta
  MSBuild.exe .\Axodox.Common.Tests\Axodox.Common.Tests.vcxproj -p:Configuration=$config -p:Platform=$platform -m:$coreCount -v:m

  if ($LastExitCode -ne 0) {
    Write-Host "Building tests $platform $config failed!" -ForegroundColor Red
    throw "Building tests $platform $config failed!"
  }

  Write-Host "Running tests $platform $config..." -ForegroundColor Magenta
  $testDll = ".\Axodox.Common.Tests\bin\$platform\$config\Axodox.Common.Tests.dll"
  $vstest = Join-Path $vsPath 'Common7\IDE\Extensions\TestPlatform\vstest.console.exe'
  & $vstest $testDll /Platform:$platform

  if ($LastExitCode -ne 0) {
    Write-Host "Tests failed in $platform $config!" -ForegroundColor Red
    throw "Tests failed in $platform $config!"
  }

  Write-Host "Tests passed in $platform $config!" -ForegroundColor Green
}
