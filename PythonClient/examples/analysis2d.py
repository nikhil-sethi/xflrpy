""" ==================== examples/analysis.py ========================== 
This example file shows how to set and run a 2d analysis for airfoils

Prerequisites: project_mgmt.py, polar_io.py
Features: 
    Set current polar
    Introduce AnalysisSettings2D class
    Define polar analysis
    Run analysis and get results

================================================================= """

from xflrpy import xflrClient, enumApp, Polar, enumPolarType, AnalysisSettings2D, enumSequenceType

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
polar = Polar(name = "myPolar", foil_name=foil.name) # a polar needs a foil to associate to

polar.spec.polar_type = enumPolarType.FIXEDSPEEDPOLAR
polar.spec.reynolds = 200000.0  
# polar.spec is a PolarSpec class and it is equivalent to the "define analysis" dialog in xflr
# there are many other properties you can change 
# BUT keeping them sensible is your responsiblity.
# i.e. setting the the appropriate properties for your polar type

# define an analysis
xdirect.define_analysis(polar)

# set some analysis settings
settings = AnalysisSettings2D()
settings.keep_open_on_error = False
settings.is_sequence = True
settings.sequence_type = enumSequenceType.ALPHA
settings.sequence = (0.0, 10.0, 0.5) # start, end, delta
settings.init_BL = True
# Again, it's your responsibility to set logical settings

# analyze and store results
polar.result = xdirect.analyze(settings)