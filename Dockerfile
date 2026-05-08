# ── Builder Stage ──────────────────────────────────────────
FROM ubuntu:24.04 AS builder

# 系统构建依赖（SFML 3.0 编译所需的所有原生库）
RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake g++ git curl zip unzip tar pkg-config \
    libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev \
    libgl1-mesa-dev libegl1-mesa-dev libfreetype-dev \
    libopenal-dev libflac-dev libvorbis-dev \
    libxcb1-dev libx11-xcb-dev libxcb-randr0-dev libxcb-image0-dev \
    libxcb-util0-dev libxcb-keysyms1-dev libxcb-render0-dev \
    libxcb-shape0-dev libxcb-sync-dev libxcb-xfixes0-dev \
    libxcb-render-util0-dev libxcb-xkb-dev libxcb-icccm4-dev libxcb-glx0-dev \
    && rm -rf /var/lib/apt/lists/*

# 克隆并 bootstrap vcpkg
RUN git clone --depth 1 https://github.com/Microsoft/vcpkg.git /opt/vcpkg \
    && /opt/vcpkg/bootstrap-vcpkg.sh
ENV VCPKG_ROOT=/opt/vcpkg

# 先复制依赖清单，利用 Docker 缓存 — 依赖不变则跳过重装
WORKDIR /app
COPY vcpkg.json CMakePresets.json ./
RUN /opt/vcpkg/vcpkg install --triplet x64-linux

# 复制源码并构建
COPY CMakeLists.txt ./
COPY src/ src/
COPY assets/ assets/
RUN cmake --preset vcpkg -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --config Release

# ── Runtime Stage ─────────────────────────────────────────
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    libx11-6 libxrandr2 libxcursor1 libxi6 libudev1 \
    libgl1 libegl1 libfreetype6 \
    libopenal1 libflac12 libvorbis0a libvorbisenc2 libvorbisfile3 \
    fonts-dejavu-core \
    && rm -rf /var/lib/apt/lists/*

# 复制 vcpkg 编译产物（SFML + curl 动态库）
COPY --from=builder /opt/vcpkg/vcpkg_installed/x64-linux/lib /usr/local/lib
RUN ldconfig

# 复制二进制和资源
COPY --from=builder /app/build/CampusLifeSimulator /app/CampusLifeSimulator
COPY --from=builder /app/assets /app/assets

WORKDIR /app
ENTRYPOINT ["./CampusLifeSimulator"]
