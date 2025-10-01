ARG USERNAME
ARG USER_UID=1000
ARG USER_GID=$USER_UID

ARG RPC_VERSION=2.3.0
ARG QT_VERSION=5.15.2
ARG QT_HOST=linux
ARG QT_TARGET=desktop

FROM ubuntu:22.04 AS xflrpy-base

ARG USERNAME
ARG USER_UID
ARG USER_GID

ARG RPC_VERSION
ARG QT_VERSION
ARG QT_HOST
ARG QT_TARGET

SHELL ["/bin/bash", "-c"]

RUN apt-get -y update && DEBIAN_FRONTEND=noninteractive apt-get -y install \
    git \
    cmake \
    python3 \
    python3-pip \
    build-essential \
    libdbus-1-3 \
    libpulse-mainloop-glib0 \
    mesa-common-dev \
    mesa-utils  \
    libgl1-mesa-dev  \
    libglu1-mesa-dev \
    '^libxcb.*-dev'  \
    libx11-xcb-dev  \
    libxrender-dev  \
    libxi-dev  \
    libxkbcommon-dev  \
    libxkbcommon-x11-dev \
    wget \
    pkg-config \
    && apt-get autoremove \
    && rm -rf /var/lib/apt/lists/**

RUN pip3 install aqtinstall

RUN aqt install-qt ${QT_HOST} ${QT_TARGET} ${QT_VERSION}  ${QT_ARCH}  --outputdir /opt/qt -m ${QT_MODULES}

ENV PATH=/opt/qt/$QT_VERSION/gcc_64/bin:$PATH
ENV QT_PLUGIN_PATH=/opt/qt/$QT_VERSION/gcc_64/plugins/
ENV QML_IMPORT_PATH=/opt/qt/$QT_VERSION/gcc_64/qml/
ENV QML2_IMPORT_PATH=/opt/qt/$QT_VERSION/gcc_64/qml/ 

# install rpclib
RUN wget -qO- https://github.com/rpclib/rpclib/archive/refs/tags/v$RPC_VERSION.tar.gz | tar -xz -C /opt/ \
    && mkdir -p /opt/rpclib-$RPC_VERSION/build

# hack this file into the download because rpclib is not up-to date
# build rpclib
RUN --mount=type=bind,source=rpclib.pc.in,target=/opt/rpclib-$RPC_VERSION/rpclib.pc.in \ 
    cd /opt/rpclib-$RPC_VERSION/build \ 
    && cmake ..  \ 
    && cmake --build . \
    && cmake --install .  \
    && ldconfig

# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    # [Optional] Add sudo support. Omit if you don't need to install software after connecting.
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME \
    && chsh -s /usr/bin/bash

# Change user
USER $USERNAME

FROM xflrpy-base as xflrpy-builder

WORKDIR /xflrpy

# build xflrpy
RUN --mount=type=bind,target=/xflrpy \ 
    && qmake \
    && make all -j8 \
    && cd /xflrpy/XFoil-lib \
    && sudo make install \ 
    && sudo ldconfig \
    && cd /xflrpy/PythonClient \
    && pip install -e .
