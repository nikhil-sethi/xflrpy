
ARG RPC_VERSION=2.3.0
ARG QT_VERSION=5.15.2
ARG QT_BIN_PATH=/opt/qt/$QT_VERSION/gcc_64/bin
ARG QT_PLUGIN_PATH=/opt/qt/$QT_VERSION/gcc_64/plugins/
ARG QML_IMPORT_PATH=/opt/qt/$QT_VERSION/gcc_64/qml/
ARG QML2_IMPORT_PATH=/opt/qt/$QT_VERSION/gcc_64/qml/

FROM ubuntu:22.04 AS xflrpy-deps

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

ARG RPC_VERSION
ARG QT_VERSION
ARG QT_MODULES=
ARG QT_HOST=linux
ARG QT_TARGET=desktop
ARG QT_ARCH=
RUN aqt install-qt ${QT_HOST} ${QT_TARGET} ${QT_VERSION}  ${QT_ARCH}  --outputdir /opt/qt -m ${QT_MODULES}

ENV PATH=$QT_BIN_PATH:$PATH
ENV QT_PLUGIN_PATH=$QT_PLUGIN_PATH
ENV QML_IMPORT_PATH=$QML_IMPORT_PATH
ENV QML2_IMPORT_PATH=$QML2_IMPORT_PATH 

# install rpclib
RUN wget -qO- https://github.com/rpclib/rpclib/archive/refs/tags/v$RPC_VERSION.tar.gz | tar -xz -C /opt/

RUN mkdir -p /opt/rpclib-$RPC_VERSION/build 
WORKDIR /opt/rpclib-$RPC_VERSION/build
# hack this file into the download because rpclib is not up-to date
COPY rpclib.pc.in /opt/rpclib-$RPC_VERSION/
RUN cmake .. && cmake --build . 
RUN cmake --install . && ldconfig

FROM xflrpy-deps as xflrpy-builder

# declare shared global args
ARG QT_BIN_PATH
ARG QT_PLUGIN_PATH
ARG QML_IMPORT_PATH
ARG QML2_IMPORT_PATH

# set qt paths 
ENV PATH=$QT_BIN_PATH:${PATH}
ENV QT_PLUGIN_PATH=$QT_PLUGIN_PATH
ENV QML_IMPORT_PATH=$QML_IMPORT_PATH
ENV QML2_IMPORT_PATH=$QML2_IMPORT_PATH 
    
COPY . /xflrpy

# # build xflrpy
WORKDIR /xflrpy
RUN qmake
RUN make all -j8

# build xfoil
WORKDIR /xflrpy/XFoil-lib
RUN make install && ldconfig

# install xflrpy pythonclient
WORKDIR /xflrpy/PythonClient
RUN pip install -e .

# FROM xflrpy-builder as xflrpy-service
