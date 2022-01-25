# ======================= examples/afoil.py ====================== #

# This example file introduces the various GUI control features
# available in the direct design module 

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

afoil.selectFoil("MH 60  10.08%")   # change the current airfoil selection
foil = afoil.foil_mgr.getFoil()    # get the current airfoil i.e. mh60

afoil.showFoil("fuselage center", False) # hide the "fuselage center" airfoil
