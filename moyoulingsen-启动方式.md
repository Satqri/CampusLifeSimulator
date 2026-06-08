# moyoulingsen 启动方式

这个项目在 **MSYS2 UCRT64** 终端下启动最方便。

## 日常启动

```bash
cd /c/Users/ASUS/Desktop/demo2
cmake -S . -B build-msys2 -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-msys2
./build-msys2/CampusLifeSimulator.exe
```

## 仅编译

```bash
cd /c/Users/ASUS/Desktop/demo2
cmake -S . -B build-msys2 -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-msys2
```

## 首次环境准备

如果依赖还没装，在 **MSYS2 UCRT64** 中先执行：

```bash
pacman -S --needed \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-sfml \
  mingw-w64-ucrt-x86_64-nlohmann-json \
  mingw-w64-ucrt-x86_64-curl
```

## 说明

- 推荐终端：**MSYS2 UCRT64**
- 项目目录：`/c/Users/ASUS/Desktop/demo2`
- 可执行文件：`./build-msys2/CampusLifeSimulator.exe`
- 如果只是改了 `.cpp/.h`，通常直接重新执行上面的“日常启动”三条命令即可
