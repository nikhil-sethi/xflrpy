from xflrpy import xflrClient, enumApp, Plane, WingSection, WPolar, enumPolarType, AnalysisSettings3D, enumWPolarResult, enumAnalysisMethod
import time

# Change these values accordingly
# Using a valid path is your responsibility
project_name = "test1.xfl"
project_path = "/home/nikhil/Software/xflrpy/projects/"

xp = xflrClient(connect_timeout=100)

# Gives useful information about the mainframe class in xflr5
print(xp.state)
xp.setApp(enumApp.MIAREX) # set to plane design application

# Load multiple airfoils; return the design application
xp.loadProject(project_path + project_name)

miarex = xp.getApp() # Get the current application


# Create a new custom plane

plane3 = Plane(name="custom_plane")

sec0 = WingSection(chord=0.2, right_foil_name="fuselage center", left_foil_name="fuselage center")
sec1 = WingSection(y_position = 1, chord=0.1, offset=0.2, twist=5, dihedral=5, right_foil_name="MH 60  10.08%", left_foil_name="MH 60  10.08%")
sec2 = WingSection(y_position = 1.1, chord=0.1, offset=0.2, twist=5, dihedral=5, right_foil_name="MH 60  10.08%", left_foil_name="MH 60  10.08%")
plane3.wing.sections.append(sec0)
plane3.wing.sections.append(sec1)
plane3.wing.sections.append(sec2)

# # create the elevator
# plane3.elevator.sections.append(())

# # create the fin
# plane3.fin.sections.append(())

miarex.plane_mgr.addPlane(plane3)

plane_data = miarex.plane_mgr.getPlaneData("custom_plane")


wpolar = WPolar(name="my_cute_polar", plane_name="custom_plane")
wpolar.spec.polar_type = enumPolarType.FIXEDSPEEDPOLAR
wpolar.spec.free_stream_speed = 12
wpolar.spec.analysis_method = enumAnalysisMethod.VLMMETHOD


miarex.define_analysis(wpolar=wpolar)

analysis_settings = AnalysisSettings3D(is_sequence=True, sequence=(0,10,1))
results = miarex.analyze("my_cute_polar", "custom_plane", analysis_settings, result_list=[enumWPolarResult.ALPHA, enumWPolarResult.CLCD])

print(results)