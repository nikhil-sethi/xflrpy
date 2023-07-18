""" ==================== examples/foil_io.py ====================== 
This example file shows the various methods available for manipulating
geometry of an airfoil in the direct design module.

Prerequisites: project_mgmt.py
Features:
    Set and get airfoils (camber, thickness etc.)
    print foil properties
    show and plot foil coordinates array
    
================================================================= """

from xflrpy import xflrClient, enumApp, Plane, WingSection
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
print(plane.elevator) # Show the foil object and it's properties


# Create a default plane in xflr
plane2 = miarex.plane_mgr.addDefaultPlane("default_plane")
print(plane2.name)

# Create a new custom plane

plane3 = Plane(name="custom_plane")

"""
sections: list of section tuples. Data order:

yPos(m): spanwise position of segment
chord(m): length of segment in x (longitudinal) direction
offset(m): x position from origin. Check XFLR wing design module
dihedral (deg): absolute dihedral angle till the next segment
twist(deg): absolute twist angle with respect to the x(longitudinal) axis
rightFoil(str): right airfoil name
leftFoil(str): left airfoil name
nXPanels: number of panels along the longitudinal directions
xPanelDist: distribution of panels in longitudinal direction
nYPanels: number of panels along the lateral directions
yPanelDist: distribution of panels in lateral direction

"""
# create the wing
sec0 = WingSection(chord=0.2, right_foil_name="fuselage center", left_foil_name="fuselage center")
sec1 = WingSection(y_position = 1, chord=0.1, offset=0.2, twist=5, dihedral=5, right_foil_name="MH 60 10.08%", left_foil_name="MH 60 10.08%")
plane3.wing.sections.append(sec0)
plane3.wing.sections.append(sec1)
# plane3.wing.sections.append((0.5, 0.1, 0.4, 2, 5, "MH 60  10.08%", "MH 60  10.08%"))

# # create the elevator
# plane3.elevator.sections.append(())

# # create the fin
# plane3.fin.sections.append(())
print(plane3)
miarex.plane_mgr.addPlane(plane3)