FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libboost-dev \
    qt6-base-dev \
    qt6-base-dev-tools \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY shared ./shared
COPY server ./server

RUN cmake -S server -B build \
    -DCMAKE_BUILD_TYPE=Release

RUN cmake --build build --target Server -j2


FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/Server ./Server

RUN mkdir -p uploads

EXPOSE 12345

CMD ["./Server"]
