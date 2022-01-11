from xflrpy import xflrClient, enumApp
import random 

project_name = "fuselage center.dat"
project_path = "/home/nikhil/Softwares/xflrpy/projects/"

xp = xflrClient(connect_timeout=100)
xp.loadProject(files=project_path+project_name, save_current=False)

xp.setApp(enumApp.DIRECTDESIGN)
afoil = xp.getApp()
foil = afoil.getFoil("fuselage center")

# geometry handling resides with the afoil/design workbench

# Let's try to make an MH-60 from this foil
# Note that these values may not be exactly set due to foil 
# generation accuracy issues in xfoil

# Set location of maximum camber to 36%
afoil.setCamberX(0.36, foil.name) 

# Set location of maximum thickness to 27%
afoil.setThickX(0.27, foil.name)

# Set max camber value to 1.82%
afoil.setCamber(0.018, foil.name)

# Set Thickess to 15%
afoil.setThickness(0.10, foil.name) 

print(afoil.getFoil("fuselage center"))

print(foil)