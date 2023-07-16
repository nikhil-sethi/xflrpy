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
import matplotlib.pyplot as plt

# Change these values accordingly
# Using a valid path is your responsibility
project_name = "test1.xfl"
project_path = "/home/nikhil/Software/xflrpy/projects/"

xp = xflrClient(connect_timeout=100)

# Gives useful information about the mainframe class in xflr5
print(xp.state)
xp.setApp(enumApp.DIRECTDESIGN) # set to airfoil design application

# Load multiple airfoils; return the design application
afoil = xp.loadProject([project_path+'mh60.dat',project_path+'fuselage center.dat'])

afoil = xp.getApp() # Get the current application

print(afoil.foil_mgr.foilDict())  # a Dictonary of all airfoils present in the project

foil = afoil.foil_mgr.getFoil("fuselage center")
print(foil) # Show the foil object and it's properties

# create a duplicate and receive it
new_foil = foil.duplicate("duplicate fuselage center")   

foil.rename("new fuselage center")  # rename the original airfoil

print(foil) # check that shit
print(new_foil) 

# Export foil to .dat file
afoil.foil_mgr.exportFoil(foil.name, "/home/nikhil/foil.dat")

foil.delete()   # delete the original foil
coord_arr = np.array(foil.coords) 
print(coord_arr)

# Display the current airfoil
fig, ax = plt.subplots()
ax.set_aspect(1)

ax.plot(coord_arr[:,0], coord_arr[:,1], '-k')
plt.show(block=False)
