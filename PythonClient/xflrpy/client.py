


import msgpackrpc as rpc
from numpy.lib.npyio import save
import time
from .types import *
from .utils import *

class xflrClient:
    def __init__(self, ip = '127.0.0.1', port = 8080, connect_timeout = 100):
        self._client = rpc.Client(rpc.Address(ip, port), timeout=connect_timeout, pack_encoding='utf-8', unpack_encoding='utf-8')
        self.poll_timeout = 5 # seconds
        if self.ping():
            print(f"Xflr client connected at port: {port}")

    @property
    def state(self):
        """
        Returns a State object for the mainframe

        Returns:
            State()
        """
        state_raw = self._client.call("getState")
        return State.from_msgpack(state_raw)

    def ping(self):
        """
        Returns true is the server is connected to the client and data can be exchanged.
        """
        return self._client.call("ping")

    def loadProject(self, files, save_current = True):
        """
        Returns pointer to application.

        If false (which is default) then API calls would be ignored. After a successful call to `enableApiControl`, `isApiControlEnabled` should return true.

        Args:
            files: (str) Absolute project path to .xfl file / (list) List of .dat airfoil files to open 
            save_current (bool, optional): Flag to save the current project

        Returns:
            Currently open pointer to an application object.
        """
        if save_current:
            self.saveProject()
        if type(files) == str:
            files = [files]
        if files is None:
            print("{1}: Please provide valid file(s). Accepted file formats: .xfl, .dat, .wpa,  ".format(files))
        else:
            self._client.call('loadProject', files)
            return self.getApp()

    def newProject(self, projectPath = "", save_current = True):
        """
        Args:
            projectPath: (str, optional) Absolute project path to .xfl file
            save_current (bool, optional): Flag to save the current project

        Returns:
            Currently open pointer to an application object.
        """
        if save_current:
            self.saveProject()
        self._client.call("newProject")
        if projectPath != "":
            if projectPath[-4:]!=".xfl": 
                projectPath += ".xfl"
            self.saveProject(projectPath)
        return self.getApp()
    
    def saveProject(self, projectPath = "")->None:
        """
        Save the appropriate project.

        Args:
            projectPath: (str, optional) Absolute project path to .xfl file
                         if empty, the current project will be saved
        Returns:
            None
        """
        if self.state.saved:
            return    
        if projectPath != "":      
            self._client.call("setProjectPath", projectPath)
        elif self.state.projectPath =="":
            print("Current project is empty. Please save with a valid path")
            return
        self._client.call("saveProject")

    def getApp(self, app = None):
        """
        Returns pointer to application.

        Args:
            app: (enumApp/int, optional) Required enum for application

        Returns:
            Currently open pointer to an application object.
        """
        if app is None:
            app = self.state.app

        if app == enumApp.NOAPP:
            print("The current project is empty. Nothing to return")
            return
        elif app == enumApp.XFOILANALYSIS:
            return XDirect(self._client)
        elif app == enumApp.MIAREX:
            return Miarex(self._client)
        elif app == enumApp.DIRECTDESIGN:
            return Afoil(self._client)
        elif app == enumApp.INVERSEDESIGN:
            return XInverse(self._client)
    
    def setApp(self, app:enumApp)->None:
        """
        Set the required application on the gui

        Args:
            app: (enumApp/int) Required enum or integer for application

        Returns:
            None
        """
        if type(app) == enumApp:
            app = app.value
        self._client.call("setApp", app)
            
    def close(self):
        """
        Cleanly exit the server thread and close the gui as well. 

        Returns:
            None
        """
        print("Closing Xflr client and server")
        self._client.call("exit")