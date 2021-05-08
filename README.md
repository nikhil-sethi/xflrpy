## Note
This repository is under development and experimental. As a result there are not many features or refined code
practices as of yet. Any and every contribution/constructive criticism is welcome. 

# xflrpy
xflrpy is an attempt to generate python bindings for the powerful software called [xflr5](http://www.xflr5.tech/xflr5.htm) created
by André Deperrois. The original software is capable of the design and low-fidelity analysis of airfoils and 
model aircraft and is hosted at [sourceforge](https://sourceforge.net/projects/xflr5/).

# Why?
I undertook this project while learning a bit of C++. This repository is aimed at exposing a neat
and equally powerful python API for the original project to make it easier for scripting/automation 
and design optimization applications. I understand that are already  software like
openVSP/SUAVE which do similar stuff. But Hey. Making APIs is fun and there is a pandemic going around so I was bored.

# How?
Some standard ways of exposing C code to python include using wrappers like [SWIG](https://github.com/swig/swig), 
Boost and [PyBind11](https://github.com/pybind/pybind11). Luckily for us, xflr5 is written using [Qt](https://www.qt.io/) 
which means there exist even more tools like [shiboken](https://github.com/pyside/Shiboken)
and [PythonQt](https://github.com/MeVisLab/pythonqt) to make life easier. For this project 
I'm using PythonQt for now and did a bit of experimenting with Pybind11(never pushed the commits though). 

Exposing the entire software as a python library involves rewriting a lot of the code and is beyond the scope 
of this project. The current version involves compiling a new version which includes an embedded python 
interpreter.

## So..How to build it?
For Linux (tested on Ubuntu 20.04):
(These instructions are basic and just an extension of [this](https://github.com/polmes/xflr5-ubuntu) repo.

Setup
```
git clone https://github.com/nikhil-sethi/xflrpy.git
git submodule update --init --recursive

cd pythonqt
export PYTHON_VERSION = <Your_python_version> (eg: 3.8)
export PYTHON_DIR = <Your_python_directory> {eg: /usr/bin/)

sudo apt install build-essential mesa-common-dev mesa-utils libgl1-mesa-dev libglu1-mesa-dev
```

Build 
(You will need Qt>=5. Check out their page for installing it)
```
cd ..
qmake # or wherever it's located
make all 
```

Link
(PythonQt needs some linking of shared libraries which is not automated as of yet. This might depend on your Linux
distro so you might need to check for yourself.)
```
echo "<path_to_xflrpy>/pythonqt/lib" | sudo tee -a /etc/ld.so.conf.d/pythonqt.conf
sudo ldconfig
```

If everything worked correctly, you should be able to start xflr5
```
./xfr5-gui/xflr5
```

## Test

The current version has very limited features for basic optimization but will hopefully expand in the future. Currently you can:
- Get a plane object by it's name
- Get an "analysis" object from the plane. Note that this analysis object is a custom structure and doesn't really exist in XFLR. It's just a combination of the given plane, polar and the sequence limits for the angle/velocity sweep.
- Set the plane's geometry. Check the code out to see what is supported
- 

A brief sketch:
```
import random
miarex = MiarexWrapper  # exposes the "wing & plane" module
plane = miarex.getPlane('api_test')
analysis = plane.getAnalysis(0)    # use either name or index
analysis.setSeq(0,10,0.5)   # signature:(alpha_start, alpha_end, alpha_delta)
analysis.showDialog(False) # hides the progress +log  window

def randrange(a, b):
    return a + (b-a)*random.random()

for i in range(10):
    chord = randrange(0.1,0.4) # metres
    plane.setChord(chord)
    polar = analysis.analyze()
    CLCD = polar.getCLCD(10)
    print(i, chord, CLCD)

```
You can adapt the above sketch to have your own optimization class and whatnot.
There is also an "Execute Python script button" for offline scripts

## TODO
These are more features I have in mind. Please feel free to add/subtract/correct:

1. Write the optimization class and wrappers
2. Add more wrappers for geometry including other modules like airfoil. Inverse might be very helpful
3. Cleanup and move each wrapper to separate files
4. Use an RPC library to make calls from outside the program
5. Make the structure better and add deeper abstractions.