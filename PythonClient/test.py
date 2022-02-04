"""
TODO
1. Separate files  

5. make newproject save state common on both platforms
"""


from xflrpy import xflrClient, enumApp, Polar, AnalysisSettings2D, enumSequenceType
project_name = "test1.xfl"
project_path = "/home/nikhil/Softwares/xflrpy/projects/"

xp = xflrClient(connect_timeout=100)
xp.loadProject(files=project_path+project_name, save_current=False)

xp.setApp(enumApp.DIRECTDESIGN)
afoil = xp.getApp()

#afoil.createNACAFoil(8208, "1")
print(afoil.foil_mgr.getFoil())

foil_name = "fuselage center"
afoil.selectFoil(foil_name)
foil = afoil.foil_mgr.getFoil(foil_name)

xp.setApp(enumApp.XFOILANALYSIS)
xdirect = xp.getApp(enumApp.XFOILANALYSIS)

polar = Polar(name="somepolar", foil_name=foil.name)
polar.spec.reynolds = 200000.0
xdirect.define_analysis(polar)
settings = AnalysisSettings2D()
settings.sequence_type = enumSequenceType.ALPHA
settings.is_sequence = True
settings.keep_open_on_error = False
settings.sequence = (0,10,0.4)

polar.result = xdirect.analyze(analysis_settings=settings)

"""
xp.newProject(path)

afoil = xp.getApp(enumApp.DIRECTDESIGN)

foil_morpher = FoilMorpher(var_bounds)
foil_geom = FoilGeomHandler()

foil.setGeom(camber = 0.3, thick = 0.1)

for foil in afoil.foil_mgr.foilList():
    new_foil = foil_morpher.morph(foil) # random var mutation
    print(new_foil.name, new_foil.getVars(vars_bounds.keys()))
    
xdirect = xp.getApp(enumApp.XFOILANALYSIS)
analysis = xdirect.createMBAnalysis(type = enumAnalysisType.FIXEDLIFT, reynolds_num = 200,000)
xdirect.setBL(True)
analysis.setAOA(5)
result = analysis.run()
result.allPolars()


"""