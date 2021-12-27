## Note
This repository is under initial development phase. As a result there are not many features or refined code practices as of yet. There might be breaking changes without updating the major version until the first release. Any and every contribution/constructive criticism is welcome. Use github issues to roast me.

# xflrpy
xflrpy is a python enabled version for design optimization and scripting for the powerful software called [xflr5](http://www.xflr5.tech/xflr5.htm) created by André Deperrois. The original software is capable of the design and low-fidelity analysis of airfoils and 
model aircraft and is hosted at [sourceforge](https://sourceforge.net/projects/xflr5/).

![Optimizing a BWB UAV](https://github.com/nikhil-sethi/xflrpy/blob/pythonqt/xflrpy.gif)

# Why?
I undertook this project while learning a bit of C++. This repository is aimed at exposing a neat
and equally powerful python API for the original project to make it easier for scripting/automation 
and design optimization applications. I understand that are already  software like
openVSP/SUAVE which do similar stuff. But Hey. Making APIs is fun and there is a pandemic going around so I was bored.

# How?
Some standard ways of exposing C code to python include using wrappers like [SWIG](https://github.com/swig/swig), 
Boost and [PyBind11](https://github.com/pybind/pybind11). Luckily for us, xflr5 is written using [Qt](https://www.qt.io/) 
which means there exist even more tools like [shiboken](https://github.com/pyside/Shiboken)
and [PythonQt](https://github.com/MeVisLab/pythonqt) to make life easier. 

There are two branches: 
### pythonqt:
This version comes with an embedded python interpreter within xflr5 itself. 

Merits:
- Has entire Python classes with signals and everything exposed in the custom interpreter. 
- It is easier and safer to expose more Qt objects to python.

Demerits:
- The main library is not well-maintained/documented and has been replaced by better tools like PySide6 (this doesn't have an embedded option tho).
- It requires PythonQt as dependency which is a fairly large library.
- Everything has to be exposed under the __main__ function which means there is little to no chance of multithreading, doing complex loops or IO operations. (These might be critical for optimization)
- The above problem also means there is no abstraction and well-defined objects which make code readable and easier to extend.
- It is difficult to use external optimization libraries within the interpreter.

### rpc:
This is the stuff I started working on recently. The branch uses the very light and very fast [rpclib](https://github.com/rpclib/rpclib) library to establish a local server which can communicate with and external python process. This means that we finally have an "xflrpy" python library. The structure on the python side is similar to [MS AirSim](https://github.com/microsoft/AirSim) but still naive. 

Merits:
- Solves everything wrong with pythonqt.
- Very neat.
- Can extend xflr5 to any programming language which uses rpc and msgpack.
- More code to write.

Demerits:
- More code to write.
- The entire server and client had to be deployed on a different thread to make non-blocking calls. This is actually a merit but can fall on it's face sometimes. It requires responsibility on the user's part to make sure that variables are updated in their own custom script. But it's a very small price to pay for better functionality. Hopefully it will become better as the architecture evolves.


Because of the above reasons, I won't be maintaining the pythonqt branch but it will be long-lived to help people learn something/anything from it.
Note that as of 28/12/21 v0.1.0, the pythonqt branch has more and better features than rpc. If you want to whip up something quick, I reccommend you use that. But this will change soon enough. I will be adding more features to rpc and hope to remove this statement ASAP :)

## So..How to build it?
For Linux (tested on Ubuntu 20.04, Python 3.8.10, Qt 5.13.2, msgpack-rpc-python 0.40.1, rpclib 2.3.0):
These instructions are basic and just an extension of [this](https://github.com/polmes/xflr5-ubuntu) repo.

Setup
```
git clone https://github.com/nikhil-sethi/xflrpy.git
git submodule update --init --recursive

sudo apt install build-essential mesa-common-dev mesa-utils libgl1-mesa-dev libglu1-mesa-dev

Build rpclib (If you already have rpclib, you can skip these steps and link the appropriate libraries in xflr-server.pro)
cd rpclib
mkdir build
cd build
cmake ..
cmake --build .
```

Build xflrpy 
(You will need Qt>=5. Check out their page for installing it)
```
cd ..
qmake # or give the complete path to your qmake location
make all -j8 (replace 8 with the number of cores you want)
```

If everything worked correctly, you should be able to start xflr5.
```
./xfr5-gui/xflr5
```

## Test
First install the python package xflrpy

```
pip3 install xflrpy
```

The current version (v0.1.0) has very limited features but is continuosly expanding. Check out the [changelog](https://github.com/nikhil-sethi/xflrpy/blob/rpc/CHANGELOG.MD). Currently you can:
- Create or load new projects
- Save projects
- Set and get apps (xfoil, plane-design, foil-design, inverse-design)

A brief sketch:
```
from xflrpy import xflrClient

# Change these values accordingly
# Using a valid path is your responsibility
project_name = "test1.xfl"
project_path = "xflrpy/projects/"

xp = xflrClient(connect_timeout=100)

# load an existing project without saving the current one
# returns the current application: Xfoil/ Airfoil-design/ plane-design/ inverse-design 
app = xp.loadProject(files = project_path + project_name, save_current = False)

# set the relevant application on the gui
xp.setApp(enumApp.DIRECTDESIGN)

# Gives useful information about the mainframe class in xflr5
print(xp.state)

# get a useable object for the current class
app = xp.getApp()

```

## TODO
These are more features I have in mind. Please feel free to add/subtract/correct:

1. Write the optimization class and wrappers
2. Add structs for geometry including other modules like airfoil. Inverse might be very helpful.
3. Cleanup and have give an xflrServer pointer to separate files
4. Make the structure better and add deeper abstractions.
5. Figure out better way of dealing with multithreading state update. Currently it's a while loop which works great but stresses out the user :|
