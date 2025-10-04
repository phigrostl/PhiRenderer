# RTL
> [中文](./README.md)
### Introduction
`PhigrosRenderer` is a Phigros renderer written in `C++` for the `Windows` platform.

Drawing inspiration from [`papFri`](https://www.bilibili.com/video/BV1YJ7uzGE9x)'s software architecture, it implements basic rendering functionalities:

- Reading and rendering of official Phigros charts.

### Usage Instructions

##### Operating Environment
```
CPU: Intel E5-2673 v3
Operating System: Windows 10
Compiler: MSVC 2019
```

#### Downloading the `Release` Version
Click on `PGR-x64 or x86-Release.exe` under `Release` to download the latest compressed package of `PGR`.
Double-click `PGR.exe` to run.

#### Building from Source
```
git clone https://github.com/phigrostl/PhigrosRenderer.git  // Clone the repository
cd PhigrosRenderer                                          // Navigate to the project directory
mkdir build                                                 // Create a build directory
cd build                                                    // Navigate to the build directory
cmake .. --DCMAKE_BUILD_TYPE=Release                        // Configure the project
cmake --build . --config Release                            // Build the project
cd x86 or x64-Release                                       // Navigate to the Release directory
.\PGR.exe                                                   // Run PGR
```

#### Editing Code with `VS2019`
```
git clone https://github.com/phigrostl/PhigrosRenderer.git  // Clone the repository
cd PhigrosRenderer                                          // Navigate to the project directory
mkdir build                                                 // Create a build directory
cd build                                                    // Navigate to the build directory
.\PGR.sln                                                   // Open the VS2019 project
```

### Showcase
- `Distorted Fate` | Music: `Sakuzyo` | Artwork: `knife美工刀` | Chart: `unDefined Future` (`JKy`, `NerSAN`, `Rikko`, `TangScend`, `百九十八`, `晨`)
<div>
	<img src="./DF1.png" width="640px" height="360px" />
</div>

<div>
	<img src="./DF2.png" width="640px" height="360px" />
</div>

<div>
	<img src="./DF3.png" width="640px" height="360px" />
</div>
