## Build on host

(tested on Ubuntu 22.04, Python 3.10.6, Qt 5.15.2, rpc-msgpack 0.6, rpclib 2.3.0).

## Build
These instructions are basic and just an extension of [this](https://github.com/polmes/xflr5-ubuntu) repo.

1. Clone the repo
```bash
git clone https://github.com/nikhil-sethi/xflrpy.git 
cd xflrpy
git submodule update --init --recursive

sudo apt install build-essential mesa-common-dev mesa-utils libgl1-mesa-dev libglu1-mesa-dev
```

2. Build rpclib (If you already have rpclib, you can skip these steps)

```bash
RPC_VERSION=2.3.0

wget -qO- https://github.com/rpclib/rpclib/archive/refs/tags/v$RPC_VERSION.tar.gz | tar -xz -C . 

mkdir -p rpclib-$RPC_VERSION/build

# hack this file into the download because rpclib is not up-to date
cp </path/to/xflrpy>/rpclib.pc.in rpclib-$RPC_VERSION/ 

# build rpclib
cd /opt/rpclib-$RPC_VERSION/build \ 
cmake ..  \ 
cmake --build . \
cmake --install .  \
sudo ldconfig
```

3. Build xflrpy 
(You will need Qt>=5.14. Check out their page for installing it)

```bash
cd xflrpy/xflr5
qmake       # or give the complete path to your qmake location
make all -j8 (replace 8 with the number of cores you want) 
sudo make install
sudo ldconfig
```


## Run
If everything worked correctly, run `xflrpy` in the terminal to start the GUI.