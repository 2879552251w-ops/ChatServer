# ---------------- 构建阶段 ----------------
FROM ubuntu:22.04 AS builder

# 1. 安装编译环境和依赖库
# 装载程序所依赖的库
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libmysqlclient-dev \
    libhiredis-dev \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# 2. 拷贝源码
WORKDIR /app
COPY . .

# 3. 编译
#执行我写好的一键编译脚本
RUN ./autobuild.sh 
#RUN mkdir build && cd build && cmake .. && make -j$(nproc)

# ---------------- 运行阶段 ----------------
FROM ubuntu:22.04

# 1. 安装运行时依赖 (Runtime Libraries)
# 只要安装 .so 库，不需要安装 -dev 头文件，体积更小
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    libmysqlclient21 \
    libhiredis0.14 \
    libssl3 \
    net-tools \
    iputils-ping \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# 2. 从构建阶段拷贝二进制文件
COPY --from=builder /app/build/ChatServer .
# 如果有配置文件，也拷贝过去
# COPY --from=builder /app/config.json .

# 3. 启动命令
# 我们使用 shell 脚本或直接传参，以便不同容器可以传入不同的 ID，但是端口要跟nginx对应
ENTRYPOINT ["./ChatServer localhost 8000"]