""" ==================== examples/foil_io.py ====================== 
This example file shows the various methods available for manipulating
geometry of an airfoil in the direct design module.

Prerequisites: project_mgmt.py
Features:
    Set and get airfoils (camber, thickness etc.)
    print foil properties
    show and plot foil coordinates array
    
================================================================= """

from xflrpy import xflrClient, enumApp
import numpy as np

# Change these values accordingly
# Using a valid path is your responsibility
project_name = "test1.xfl"
project_path = "/home/nikhil/Software/xflrpy/projects/"

xp = xflrClient(connect_timeout=100)

# Gives useful information about the mainframe class in xflr5
print(xp.state)
xp.setApp(enumApp.MIAREX) # set to airfoil design application

# Load multiple airfoils; return the design application
xp.loadProject(project_path + project_name)

miarex = xp.getApp() # Get the current application

# print(miarex.plane_mgr))  # a Dictonary of all airfoils present in the project

plane = miarex.plane_mgr.getPlane("Plane Name")
print(plane.wing.sections) # Show the foil object and it's properties

