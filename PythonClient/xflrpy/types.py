import enum

class MsgpackMixin:
    def __repr__(self):
        from pprint import pformat
        return "<" + type(self).__name__ + "> " + pformat(vars(self), indent=4, width=1)
    def to_msgpack(self, *args, **kwargs):
        return self.__dict__
    @classmethod
    def from_msgpack(cls, encoded, client = None):
        if client is not None:
            obj = cls(client)
        else:
            obj=cls()
        # obj.__dict__ = {k.decode('utf-8'): (v.__class__.from_msgpack(v.__class__, v) if hasattr(v, "__dict__") else v) for k, v in encoded.items()}
        obj.__dict__.update({ k : (v if not isinstance(v, dict) else getattr(getattr(obj, k).__class__, "from_msgpack")(v)) for k, v in encoded.items()})
        #return cls(**msgpack.unpack(encoded))
        return obj

class enumApp(enum.IntEnum):
    NOAPP = 0
    XFOILANALYSIS = 1
    DIRECTDESIGN = 2
    INVERSEDESIGN = 3
    MIAREX = 4

class State(MsgpackMixin):
    projectPath = ""
    projectName = ""
    app = enumApp
    saved = False
    display = True

class Polar(MsgpackMixin):
    alpha=list()
    Cl=list()

class Foil(MsgpackMixin):
    name = ""
    camber = 0.0
    camber_x = 0.0
    thickness = 0.0
    thickness_x = 0.0
    n = 0

    def __init__(self, client) -> None:
        self._client=client
    
    @property
    def coords(self):
        return self._client.call("foilCoords", self.name)

    # def getPolar(self, polar_name):
    #     polar_raw = self.client.call('getPolar', self.name, polar_name) 
    #     return Polar.from_msgpack(polar_raw)

    # def getCurrPolar(self, polar_name):
    #     polar_raw = self.client.call('getCurrPolar', self.name) 
    #     return Polar.from_msgpack(polar_raw)

class Objects2d:
    """
    All 2D object management: Foils, Polars, Operating points
    All functions are just a replica of xflr5 Objects2d
    """
    def __init__(self, client) -> None:
        self._client = client
    
    def getFoil(self, name) -> Foil:
        if not self.foilExists(name):
            print(f"Airfoil with name {name} does not exist.")
            return    
        foil_raw = self._client.call("getFoil", name)  
        return Foil.from_msgpack(foil_raw, self._client)
        
    def foilExists(self, name) -> bool:
        return self._client.call("foilExists", name)
    
    def foilList(self) -> list:
        foil_list_raw = self._client.call("foilList") 
        return [Foil.from_msgpack(item, self._client) for item in foil_list_raw]

    def deleteFoil(self, foil:Foil) -> None:
        self._client.call("deleteFoil", foil.__dict__)

class FoilManager:
    """
    Manager for airfoils. 
    This class can be inherited to perform global operaions on airfoils.
    """
    def __init__(self, client) -> None:
        self.objects2d = Objects2d(client)

    def getFoil(self, name) -> Foil:
        return self.objects2d.getFoil(name)

    def foilExists(self, name) -> bool:
        return self.objects2d.foilExists(name)
    
    def foilDict(self) -> dict:
        return {foil.name: foil for foil in self.objects2d.foilList()}
    
    def loadFoils(self, paths):
        if type(paths) == str:
            paths = [paths] 
        if paths[-4:]!=".dat": 
            print("Please provide a valid .dat file")
            return
        self._client.call("loadFoils", paths)
    

class Afoil:
    """
    Manage the direct design GUI.
    """
    def __init__(self, client) -> None:
        self._client = client
        self.foil_mgr = FoilManager(client)

    @property
    def currFoil(self)->Foil:
        return self.foil_mgr.getFoil()

    @property
    def foilDict(self)->dict:
        return self.foil_mgr.foilDict()

    def getFoil(self, name) -> Foil:
        return self.foil_mgr.getFoil(name)

    def foilExists(self, name) -> bool:
        return self.foil_mgr.foilExists(name)

    def loadFoils(self, paths) -> list:
        return self.foil_mgr.loadFoils(paths)
    
    def setFoil(self, foil):
        pass

    def setCamber(self, val, name = None):
        self._client.call("setCamber", val, name)
        
    def setThickness(self, val, name = None):
        self._client.call("setThickness", val, name)

    def setCamberX(self, val, name = None):
        self._client.call("setCamberX", val, name)
        
    def setThickX(self, val, name = None):
        self._client.call("setThickX", val, name)

class Miarex(MsgpackMixin):
    """
    to manage the plane design application
    """
    def __init__(self, client) -> None:
        self._client = client

class XDirect(MsgpackMixin):
    """
    to manage the xfoil application
    """
    def __init__(self, client) -> None:
        self._client = client

class XInverse(MsgpackMixin):
    """
    to manage the plane design application
    """
    def __init__(self, client) -> None:
        self._client = client

