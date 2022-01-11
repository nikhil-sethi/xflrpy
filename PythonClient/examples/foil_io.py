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

print(afoil.foilDict)  # a Dictonary of all airfoils present in the project

foil = afoil.getFoil("fuselage center")
print(foil) # Show the foil object and it's properties
print(foil.coords)  # Print foil's coordinates

coord_arr = np.array(foil.coords) 

# Display the current airfoil
fig, ax = plt.subplots()
ax.set_aspect(1)

ax.plot(coord_arr[:,0], coord_arr[:,1], '-k')
plt.show()
"""

from xflrpy import xflrClient
import numpy as np
import time

# Change these values accordingly
# Using a valid path is your responsibility
project_name = "test1.xfl"
project_path = "/home/nikhil/Softwares/xflrpy/projects/"

xp = xflrClient(connect_timeout=100)

# Gives useful information about the mainframe class in xflr5
print(xp.state)

xp.setApp(enumApp.DIRECTDESIGN) 
afoil = xp.getApp()

print(afoil.foilDict) 

foil = afoil.getFoil("fuselage center")
print(foil) 
print(foil.coords)  

coord_arr = np.array(foil.coords) 

fig, ax = plt.subplots()
ax.set_aspect(num=0.1)
plt.plot(coord_arr[:,0], coord_arr[:,1], '-k') 
plt.show(block=False)

new_foil = Foil()   # able to initialise with 
afoil.add_foil(foil)
afoil.update()

af
"""