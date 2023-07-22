## Note
This repository is under initial development phase. As a result there are not many features or refined code practices as of yet. There might be breaking changes without updating the major version until the first release. Any and every contribution/constructive criticism is welcome. Use github issues to roast me.

# xflrpy v0.6.0
![](https://github.com/nikhil-sethi/xflrpy/blob/master/xflr5v6/resources/images/xp_github.jpeg)

xflrpy is a python enabled version of [xflr5](http://www.xflr5.tech/xflr5.htm) with support for scripting and design optimization using a [python package](https://pypi.org/project/xflrpy/). The [original software](https://sourceforge.net/projects/xflr5/) is capable of the design and low-fidelity analysis of airfoils and model aircraft and was created by created by André Deperrois.

The current version (v0.6.0) has limited features but is continuosly expanding. Check out the [changelog](https://github.com/nikhil-sethi/xflrpy/blob/master/CHANGELOG.md) and [Todo](https://github.com/nikhil-sethi/xflrpy/blob/master/TODO.md). Currently you can:

- Create, load and save projects
- Set and get apps (xfoil, plane-design, foil-design, inverse-design)
- Set and get airfoils from direct design
- Set geometry properties for airfoils including individual coordinates
- Changes display styles for airfoils
- Select, delete, rename, export airfoils
- Set and get polars from XDirect module 
- Analyse 2D polars
- Get targeted results from operating points
- Set analysis and display settings including animation

New in v0.6.0! 
- Create and modify planes
- Set and get wing polars from Miarex module 
- Analyse 3D wing polars

![Optimizing a BWB UAV](https://github.com/nikhil-sethi/xflrpy/blob/pythonqt/xflrpy.gif)

# Why?
I undertook this project while learning a bit of C++. This repository is aimed at exposing a neat
and equally powerful python API for xflr5 to make it easier for scripting/automation 
and design optimization applications. 

I understand that are already  software like openVSP/SUAVE which do similar stuff. But all these softwares either have good frontends or backends but not both. xflr5 has one of the most intuitive and responsive frontends while being feature rich at the same time. It would be very powerful with a good API and that is the goal of this project.

Also, making APIs is fun, there is a pandemic going around and I was bored.

# How?
Some standard ways of exposing C code to python include using wrappers like [SWIG](https://github.com/swig/swig), 
Boost and [PyBind11](https://github.com/pybind/pybind11). Luckily for us, xflr5 is written using [Qt](https://www.qt.io/) 
which means there exist even more tools like [shiboken](https://github.com/pyside/Shiboken)
and [PythonQt](https://github.com/MeVisLab/pythonqt) to make life easier. 

There are two approaches that I explored: 
### pythonqt:
This version comes with an embedded python interpreter within xflr5 itself. 

Merits:
- Has entire Python classes with signals and everything exposed in the custom interpreter. 
- It is easier and safer to expose more Qt objects to python.

Demerits:
- The main library is not well-maintained/documented and has been replaced by better tools like PySide6 (this doesn't have an embedded option tho).
- It requires PythonQt as dependency which is a fairly large library.
- Everything has to be exposed under the __main__ function which means there is little to no chance of multithreading, doing complex loops or IO operations. (These might be critical for optimization)
- The above problem also means there is no abstraction and well-defined objects which make the code difficult to read and extend.
- It is difficult to use external optimization libraries within the interpreter.

### rpc:
This is the stuff I started working on recently. The approach uses the very light and very fast [rpclib](https://github.com/rpclib/rpclib) library to establish a local server which can communicate with and external python process. This means that we finally have an ["xflrpy" python library](https://pypi.org/project/xflrpy/).

Merits:
- Solves everything wrong with pythonqt.
- Very neat.
- Can extend xflr5 to any programming language which uses rpc and msgpack.
- More code to write.

Demerits:
- More code to write.
- There might be concurrency problems in the future as I add more features.


Because of the above reasons, I won't be maintaining the pythonqt approach but it will be long-lived to help people learn something/anything from it. The rpc approach has been merged to the master branch.
Note that as of 28/12/21 v0.2.0, the pythonqt branch has more and better features than rpc. If you want to whip up something quick, I reccommend you use that. But this will change soon enough. I will be adding more features to rpc and hope to move this statement to the changelog ASAP :) 

## So..How to build it?
For Linux (tested on Ubuntu 22.04, Python 3.10.6, Qt 5.15.2, rpc-msgpack 0.6, rpclib 2.3.0):
These instructions are basic and just an extension of [this](https://github.com/polmes/xflr5-ubuntu) repo.

Setup
```
git clone https://github.com/nikhil-sethi/xflrpy.git 
cd xflrpy
git submodule update --init --recursive

sudo apt install build-essential mesa-common-dev mesa-utils libgl1-mesa-dev libglu1-mesa-dev
```
Build rpclib (If you already have rpclib, you can skip these steps and link the appropriate libraries in xflr5-gui.pro)
```
cd rpclib
mkdir build
cd build
cmake ..
cmake --build .
```

Build xflrpy 
(You will need Qt>=5.14. Check out their page for installing it)
```
cd ../..
qmake # or give the complete path to your qmake location
make all -j8 (replace 8 with the number of cores you want) 
```

Install and link some libraries
```
cd XFoil-lib
sudo make install
sudo ldconfig
```

If everything worked correctly, you should be able to start xflr5.
```
cd ..
./xfr5v6/xflrpy
```

> Note: If you're working on Visual Studio code (Linux) and see the following error: 

`symbol lookup error: /snap/core20/current/lib/x86_64-linux-gnu/libpthread.so.0: undefined symbol: __libc_pthread_init, version GLIBC_PRIVATE `
> Run the command: `unset GTK_PATH` in the terminal and try again.


Install the python client

```
cd PythonClient 
pip install -e .
```

## Test

A brief sketch:
```python
from xflrpy import xflrClient, enumApp, enumLineStipple, Plane, WingSection, WPolar, enumPolarType, AnalysisSettings3D, enumWPolarResult, enumAnalysisMethod


# ======== PROJECT MANAGEMENT ========= 

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


# =========== AIRFOIL MORPHING =============

# get a useable object for the current class
afoil = xp.getApp(enumApp.DIRECTDESIGN)

# get a specific airfoil from a dictionary of all airfoils
foilDict = afoil.foilDict["MH 60  10.08%"] 

# you can also get the airfoil through a straight function. This is faster
foil = afoil.getFoil("MH 60  10.08%")

# show some airfoil properties
print(foil) 

# Get airfoil coordinates and play with them (what?)
coords = foil.coords
print(foil.coords)
coords[10] = [0.1, 0.01]  # change the 11th coordinate to something
foil.coords = coords

# Modify foil geometry
foil.setGeom(camber = 0.03)
foil.setGeom(thickness = 0.15, camber_x = 0.27)

# change styles
ls = afoil.getLineStyle(foil.name)
ls.color = [255,0,0,255] # r,g,b,a  make it completely red
ls.stipple = enumLineStipple.DASHDOT
afoil.setLineStyle(foil.name, ls)   # set the style finally

# delete foil. why not. what's the point of all this. why are we here.
# foil.delete()


# =========== PLANE MORPHING =============

xp.setApp(enumApp.MIAREX) # set to plane design application

# Load multiple airfoils; return the design application
xp.loadProject(project_path + project_name)

miarex = xp.getApp() # Get the current application


# Create a new custom plane
plane3 = Plane(name="custom_plane")


# let's add sections to the primary wing
sec0 = WingSection(chord=0.2, right_foil_name="fuselage center", left_foil_name="fuselage center") # you might need to change airfoils accordingly
sec1 = WingSection(y_position = 1, chord=0.1, offset=0.2, twist=5, dihedral=5, right_foil_name="MH 60  10.08%", left_foil_name="MH 60  10.08%")
plane3.wing.sections.append(sec0)
plane3.wing.sections.append(sec1)


### You can also add elevators and fins, but i'll leave that for now 
# # create the elevator
# plane3.elevator.sections.append(())

# # create the fin
# plane3.fin.sections.append(())

# adds the plane to the list and tree view
miarex.plane_mgr.addPlane(plane3)

# get and print useful plane data
plane_data = miarex.plane_mgr.getPlaneData("custom_plane")
print(plane_data)


# =========== 3D ANALYSIS =============

# create a Wing Polar object
wpolar = WPolar(name="my_cute_polar", plane_name="custom_plane")
# set it's specfications
wpolar.spec.polar_type = enumPolarType.FIXEDSPEEDPOLAR
wpolar.spec.free_stream_speed = 12
wpolar.spec.analysis_method = enumAnalysisMethod.VLMMETHOD

# creates the analysis
miarex.define_analysis(wpolar=wpolar)

# these settings are on the right pane in the GUI
analysis_settings = AnalysisSettings3D(is_sequence=True, sequence=(0,10,1))

# get custom results from the analysis
results = miarex.analyze("my_cute_polar", "custom_plane2", analysis_settings, result_list=[enumWPolarResult.ALPHA, enumWPolarResult.CLCD])

print(results)

```


Check out the examples directory for more!


## Cite
If you find the work useful in your own projects or research, consider citing it! It'll help the software reach other researchers and keep your conscience clear.

## Bonus Gif for scrolling
![](200w.gif)