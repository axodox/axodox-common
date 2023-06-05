## Introduction

This repository contains helper classes for my projects, which mostly target Windows.

## Licensing

The source code of this library is provided under the MIT license.

## Integrating the component

Prebuilt versions of the project can be retrieved from Nuget under the name `Axodox.Common` and added to Visual Studio C++ projects (both desktop and UWP projects are supported) with the x64 platform.

Basic integration:
- Add the `Axodox.Common` packages to your project
- Ensure that your compiler is set to **C++20**, we also recommend enabling all warnings and conformance mode
- Add the following include statement to your code file or precompiled header: `#include "Include/Axodox.*.h"`

# Building the project

Building the library is required to make and test changes. You will need to have the following installed to build the library:

- [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/)
  - Select the following workloads:
    - Desktop development with C++    
    - Game development with C++
    - Universal Windows Platform development
    - C++ (v143) Universal Windows Platform tools

You can either run `build_nuget.ps1` or open `Axodox.Common.sln` and build from Visual Studio.

Once you have built the library, you override your existing nuget package install by setting the `AxodoxCommon-Location` environment variable to point to your local build. 

> For example `C:\dev\axodox-common\Axodox.Common.Universal` for an UWP app and `C:\dev\axodox-common\Axodox.Common.Desktop` for a desktop app.

This allows to add all projects into the same solution and make changes on the library and your app seamlessly without copying files repeatedly.