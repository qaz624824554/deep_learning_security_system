# 使用ubuntu22.04，配置c++开发环境
FROM ubuntu:22.04

# 设置时区
RUN ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime

# 设置环境变量
# ENV http_proxy=http://host.docker.internal:1087
# ENV https_proxy=http://host.docker.internal:1087
# ENV ALL_PROXY=socks5://host.docker.internal:1080

# 安装必要的工具
RUN sed -i s@/ports.ubuntu.com/@/mirrors.aliyun.com/@g /etc/apt/sources.list && apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    vim \
    tmux \
    curl \
    wget \
    zsh \
    clangd

RUN sed -i s@/ports.ubuntu.com/@/mirrors.aliyun.com/@g /etc/apt/sources.list && apt-get update && \
    apt-get install -y pkg-config \
    libopencv-dev \
    libavcodec-dev \
    libavformat-dev \
    libavfilter-dev \
    libavutil-dev \
    libswscale-dev \
    libfreetype-dev \
    libdrm-dev


CMD ["/bin/zsh"]