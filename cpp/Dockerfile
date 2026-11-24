# Stage 1: Build the C++ collector
FROM debian:trixie-slim AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    ca-certificates \
    autoconf \
    automake \
    libtool \
    perl \
    python3 \
    bison \
    flex \
    && rm -rf /var/lib/apt/lists/*

# Install a pinned release of the vcpkg tool
ARG VCPKG_RELEASE=2026.10.16

RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg \
    && cd /opt/vcpkg \
    && git checkout ${VCPKG_RELEASE} \
    && ./bootstrap-vcpkg.sh -disableMetrics \
    && ./vcpkg version

WORKDIR /app

COPY vcpkg.json vcpkg-configuration.json ./

RUN --mount=type=cache,target=/root/.cache/vcpkg/archives \
    /opt/vcpkg/vcpkg install --triplet x64-linux

COPY CMakeLists.txt .
COPY monitor/ ./monitor/

RUN cmake -S . -B /build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=x64-linux \
    -DVCPKG_INSTALLED_DIR=/app/vcpkg_installed

RUN cmake --build /build --target server


# Stage 2: Run the collector
FROM debian:trixie-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /build/server /usr/local/bin/server

CMD ["/usr/local/bin/server"]