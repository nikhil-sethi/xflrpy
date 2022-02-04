""" ==================== examples/afoil.py ========================== 
This example file introduces the various GUI control features
available in the direct design module 

Prerequisites: project_mgmt.py
Features: 
    Select, show and hide foils
    Change and create new styles for the airfoil

================================================================= """

from xflrpy import xflrClient, enumApp, enumLineStipple, LineStyle, enumPointStyle

# Change these values accordingly
# Using a valid path is your responsibility
project_name = "test1.xfl"
project_path = "/home/nikhil/Softwares/xflrpy/projects/"

xp = xflrClient(connect_timeout=100)

# Gives useful information about the mainframe class in xflr5
print(xp.state)
xp.setApp(enumApp.DIRECTDESIGN) # set to airfoil design application

# Load multiple airfoils; return the design application
afoil = xp.loadProject([project_path+'mh60.dat',project_path+'fuselage center.dat'])

afoil = xp.getApp() # Get the current application


# Manipulating controls on the GUI

afoil.selectFoil("MH 60  10.08%")   # change the current airfoil selection
foil = afoil.foil_mgr.getFoil()    # get the current airfoil i.e. mh60

afoil.showFoil("fuselage center", False) # hide the "fuselage center" airfoil


# Styling the airfoil
# Done using the LineStyle class in types.py

ls = afoil.getLineStyle(foil.name)  # get current linestyle

# make some cute changes
ls.color = [255,255,0,255]  # [r, g, b, a] yellow
ls.width = 2
ls.stipple = enumLineStipple.DASHDOT    # set a stipple style

# update the style finally
afoil.setLineStyle(foil.name, ls)

# You can also create your style here
ls = LineStyle(color = [255,0,255,255], point_style = enumPointStyle.TRIANGLE)
afoil.setLineStyle(foil.name, ls)