""" ==================== examples/polar_io.py ========================
This example file introduces the various input output classes
available for 2d Analyses 

Prerequisites: project_mgmt.py
Features: 
    Create, select polar
    Print the polar class
    Get already existing polars

================================================================= """

from xflrpy import xflrClient, enumApp, Polar, enumPolarType

# Change these values accordingly
# Using a valid path is your responsibility
project_name = "test1.xfl"
project_path = "/home/nikhil/Softwares/xflrpy/projects/"

xp = xflrClient(connect_timeout=100)

# Load multiple airfoils; return the design application
xp.loadProject(project_path+project_name, save_current=False)

# get some apps
xdirect = xp.getApp(enumApp.XFOILANALYSIS) # Get the xdirect application


# Create a polar and set some specifications
foil = xdirect.foil_mgr.getFoil("MH 60  10.08%")
polar = Polar(name = "i name thee polar: myPolar", foil_name=foil.name) # a polar needs a foil to associate to

polar.spec.polar_type = enumPolarType.FIXEDSPEEDPOLAR
polar.spec.reynolds = 200000.0  
# polar.spec is a PolarSpec class and it is equivalent to the "define analysis" dialog in xflr
# there are many other properties you can change 
# BUT keeping them sensible is your responsiblity.
# i.e. setting the the appropriate properties for your polar type

polar2 = Polar(name = "myOtherPolar", foil_name=foil.name)  # default pr
print(polar2)

# get an already exisiting polar from an already exisiting foil
polar3 = xdirect.polar_mgr.getPolar(polar_name = "T1_Re0.100_M0.00_N9.0", foil_name="fuselage center")
print(polar3)
