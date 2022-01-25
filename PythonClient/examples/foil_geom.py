from xflrpy import xflrClient, enumApp
import random 

project_name = "fuselage center.dat"
project_path = "/home/nikhil/Softwares/xflrpy/projects/"

xp = xflrClient(connect_timeout=100)
xp.loadProject(files=project_path+project_name, save_current=False)

xp.setApp(enumApp.DIRECTDESIGN)
afoil = xp.getApp()
foil = afoil.foil_mgr.getFoil("fuselage center")
# foil_mgr is a Foil Manager that helps in IO/general operations for airfoils
# Let's try to make an MH-60 from this foil
# Note that these values may not be exactly set due to foil 
# generation accuracy issues in xfoil

# Set location of maximum camber to 36%
foil.setGeom(camber_x = 0.36) 

# Set location of maximum thickness to 27% + max camber value to 1.82%
foil.setGeom(camber = 0.018,thickness_x = 0.27)

# Set Thickess to 10%
foil.setGeom(thickness = 0.10) 

print(afoil.foil_mgr.getFoil("fuselage center"))
