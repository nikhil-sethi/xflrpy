from xflrpy import xflrClient, enumApp
import numpy as np
import matplotlib.pyplot as plt

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

print(afoil.foil_mgr.foilDict())  # a Dictonary of all airfoils present in the project

foil = afoil.foil_mgr.getFoil("fuselage center")
print(foil) # Show the foil object and it's properties

# create a duplicate and receive it
new_foil = foil.duplicate("duplicate fuselage center")   

foil.rename("new fuselage center")  # rename the original airfoil

print(foil) # check that shit
print(new_foil) 

foil.delete()   # delete the original foil
coord_arr = np.array(foil.coords) 
print(coord_arr)

# Display the current airfoil
fig, ax = plt.subplots()
ax.set_aspect(1)

ax.plot(coord_arr[:,0], coord_arr[:,1], '-k')
plt.show()