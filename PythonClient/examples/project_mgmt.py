from xflrpy import xflrClient, enumApp
import time

# Change these values accordingly
# Using a valid path is your responsibility
project_name = "test1.xfl"
project_path = "/home/nikhil/Softwares/xflrpy/projects/"

xp = xflrClient(connect_timeout=100)

# Gives useful information about the mainframe class in xflr5
print(xp.state)

# load an existing project without saving the current one
# returns the relevant app: Xfoil/ Airfoil-design/ plane-design/ inverse-design 
app = xp.loadProject(files = project_path + project_name, save_current = False)

print(xp.state)
# The current project is empty. Nothing to return
# <State> {   'app': 1,
#     'display': True,
#     'projectName': 'test1',
#     'projectPath': '/home/nikhil/Softwares/xflrpy/projects/test1.xfl',
#     'saved': True}


# create a new project after saving current one
app = xp.newProject(projectPath = project_path + "test2", save_current = True)

# set the relevant application on the main-window
xp.setApp(enumApp.DIRECTDESIGN)

# get a useable object for the current class
app = xp.getApp()
print(app)          # <Afoil> {'_client': <msgpackrpc.client.Client object at 0x7f2e44575070>}
print(xp.state)
# <State> {   'app': 2,
#     'display': True,
#     'projectName': 'test2',
#     'projectPath': '/home/nikhil/Softwares/xflrpy/projects/test2.xfl',
#     'saved': True}


# get a useable object for the Miarex (plane design) class
app = xp.getApp(enumApp.MIAREX)
print(app)          # <Miarex> {'_client': <msgpackrpc.client.Client object at 0x7f2e44575070>}
print(xp.state) 
# <State> {   'app': 2,
#     'display': True,
#     'projectName': 'test2',
#     'projectPath': '/home/nikhil/Softwares/xflrpy/projects/test2.xfl',
#     'saved': True}


# close gui and server
# xp.close()


