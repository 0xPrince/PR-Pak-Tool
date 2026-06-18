<p align="center">
  <h1 align="center">PR Pak Tool</h1>
  <p align="center">
    <strong>A Cross-Platform Tool To Unpack and Repack Pak File of Multiple Games</strong>
  </p>
</p>

---

## Supported Games
| Title| Supported Version |
|------|-------------------|
| **PUBGM Global Variants** | Initial - v4.4 |
| **PUBGM Lite** | Initial - v0.27 |
| **Game For Peace** | Initial - v1.35 |

> [!note]
>Currently Supports Only Unpacking


## Supported Platforms
* Windows  
* Android  
* can also be built for Linux 

## How to use

#### Windows
**Command-Line:** ```PRPakTool.exe PakFilePath  options(Optional)```  
**OR** Drag & Drop PakFile Directly onto PRPakTool.exe  
**OR** Use [Helper Scripts](/Scripts/Windows), Supports Batch Processing **(Recommended)**  

#### Android
**Command-Line:** ```./PRPakTool PakFilePath options(Optional)```  
Example:  
``` 
chmod 755 PRPakTool
./PRPakTool /data/tmp/Test.pak -output data/tmp/
```

> use -help command to view available options  

> [!note]
>if the output path is not specified in cli, the processed files will be in the same place where the tool exists


### Download
[Check out Releases](https://github.com/0xPrince/PR-Pak-Tool/releases)


## How to compile

#### Required Tools
Windows
* Visual Studio

Android
* Android NDK  

#### Required Dependencies

* OpenSSL crypto  
* Oodle  
* ZLib  
* ZSTD  

> [!note]
>Mentioned Dependencies Static libraries are not provided with this project; To compile it, add them to the following path with respect to the platform you want to compile for OR see pch header file for "NO_FLAGS" to build without specific Dependency.  
>Path: include/lib/TARGET_ARCH/DependencyName/StaticLibrary

---

### Disclaimer
*This software is provided 'as-is', without any express or implied warranty. shared solely for educational purposes. It is not intended to harm any entity and In no event will the authors be held liable for any damages arising from the use of this software.*

