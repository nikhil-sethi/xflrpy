## Note

This repository is under initial development phase. As a result there are not many features or refined code practices as of yet. There might be breaking changes without updating the major version until the first release. Any and every contribution/constructive criticism is welcome. Use github issues to roast me.

# xflrpy v0.7.0

![xflrpy_logo](https://github.com/nikhil-sethi/xflrpy/blob/master/xflr5v6/resources/images/xp_github.jpeg)

xflrpy is a python enabled version of [xflr5](http://www.xflr5.tech/xflr5.htm) with support for scripting and design optimization using a [python package](https://pypi.org/project/xflrpy/). The [original software](https://sourceforge.net/projects/xflr5/) is capable of the design and low-fidelity analysis of airfoils and model aircraft and was created by created by André Deperrois.

The current version (v0.7.0) has limited features but is continuosly expanding. Check out the [changelog](https://github.com/nikhil-sethi/xflrpy/blob/master/CHANGELOG.md) and [Todo](https://github.com/nikhil-sethi/xflrpy/blob/master/TODO.md). Currently you can:

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
- Create and modify planes
- Set and get wing polars from Miarex module 
- Analyse 3D wing polars

New in **v0.7.0**! 
- Docker + Devcontainer support!

![Optimizing a BWB UAV](https://github.com/nikhil-sethi/xflrpy/blob/pythonqt/xflrpy.gif)

## Installation

Currently **only** Linux is supported.

If you _really_  want to use it in Windows AND you're the kind of person who hates to sleep (spoiler i'm also that person), you can try using the docker installation in Windows Subsystem for Linux ([WSL](https://learn.microsoft.com/en-us/windows/wsl/install)). But yeah best of luck, you warrior.

### With docker (recommended)

This step assumes you are have docker engine installed on your system.

**Terminal 1**: Spawn a container for the SW:
```bash
git clone https://github.com/nikhil-sethi/xflrpy.git
cd xflrpy
CURRENT_USER=$(id -u):$(id -g) docker compose up    # might take some time
```
You should see the XFLR5 gui.

**Terminal 2**: Install the pythonclient

```bash
cd xflrpy
pip install -e .
```
Now you are ready to run the examples from [this]() directory.

If you want to explore some steps yourself you can also pull images manually and test them out.

Available images:
- [`base`](https://hub.docker.com/r/niksethi/xflrpy/tags): A base image with all dependencies installed without the software installed.
- [`latest`](https://hub.docker.com/r/niksethi/xflrpy/tags)`: An image with everything installed and ready to go. The compose command uses this image.


### Without docker

You can also build the software on your host PC, but it will require more time debugging and spending less time building cool stuff. But some people like debugging.

Follow [these](./build_yourself.md) instructions to build on your host

> Note: If you're working on Visual Studio code (Linux) and see the following error: 

`symbol lookup error: /snap/core20/current/lib/x86_64-linux-gnu/libpthread.so.0: undefined symbol: __libc_pthread_init, version GLIBC_PRIVATE `
> Run the command: `unset GTK_PATH` in the terminal and try again.

## Development 

This section is only useful for developing or contributing code to this repository. Skip if you're only a user.

Assuming you have VScode Devcontainers installed on your PC:

```bash
cd xflrpy
code .
```
You should vscode open up an a dialog box in the bottom right saying "Reopen in container". Click this to open the repository in the container. 

You can now develop in this container. The development dockerfile uses the [`base`](https://hub.docker.com/r/niksethi/xflrpy/tags) image and expects you to perform the build yourself. See [Dockerfile.latest](./docker/Dockerfile.latest) how to do this.

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
foilDict = afoil.foil_mgr.foilDict["MH 60  10.08%"] 

# you can also get the airfoil through a straight function. This is faster
foil = afoil.foil_mgr.getFoil("MH 60  10.08%")

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
If you find the work useful in your own projects or research, consider [citing](CITATION.cff) it! It'll help the software reach other researchers and keep your conscience clear.

If you're more interested, checkout the [Why](why.md) and [How](how.md) of this project.

## Bonus Gif for scrolling
![](200w.gif)