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

class enumLineStipple(enum.IntEnum):
    SOLID = 0
    DASH = 1
    DOT = 2
    DASHDOT = 3
    DASHDOTDOT = 4
    NOLINE = 5

class enumPointStyle(enum.IntEnum):
    NOSYMBOL = 0
    LITTLECIRCLE = 1  
    BIGCIRCLE = 2 
    LITTLESQUARE = 3
    BIGSQUARE = 4
    TRIANGLE = 5
    TRIANGLE_INV = 6
    LITTLECIRCLE_F = 7
    BIGCIRCLE_F = 8
    LITTLESQUARE_F = 9
    BIGSQUARE_F = 10
    TRIANGLE_F = 11
    TRIANGLE_INV_F = 12
    LITTLECROSS = 13
    BIGCROSS = 14

class State(MsgpackMixin):
    projectPath = ""
    projectName = ""
    app = enumApp
    saved = False
    display = True

class Polar(MsgpackMixin):
    alpha=list()
    Cl=list()

class FoilStyle(MsgpackMixin):
    colour = ""


class Foil(MsgpackMixin):
    name = ""   # Name of the airfoil 
    camber = 0.0   # Maximum camber range(0,1) 
    camber_x = 0.0  # Location of maximum camber
    thickness = 0.0 # Maximum thickness
    thickness_x = 0.0 
    n = 0

    def __init__(self, client) -> None:
        self._client=client

    @property
    def coords(self) -> list:
        return self._client.call("foilCoords", self.name)
    
    def setGeom(self, camber = 0., camber_x = 0., thickness=0., thickness_x=0.):
        # set on python side
        if camber != 0.:
            self.camber = camber
        if camber_x != 0.:
            self.camber_x = camber_x
        if thickness != 0.:
            self.thickness = thickness
        if thickness_x != 0.:
            self.thickness_x = thickness_x
        # set on cpp side
        self._client.call("setGeom", self.name, camber, camber_x, thickness, thickness_x)

    def duplicate(self, toName):
        """
        toName: new name for duplicated airfoil
        """
        return self._client.call("duplicateFoil", self.name, toName)

    def delete(self) -> None:
        self._client.call("deleteFoil")

class FoilManager:
    """
    Manager for airfoils.
    """
    def __init__(self, client) -> None:
        self._client = client

    def getFoil(self, name=""):
        foil_raw = self._client.call("getFoil", name)
        return Foil.from_msgpack(foil_raw, self._client)

    def foilExists(self, name) -> bool:
        return self._client.call("foilExists", name)
    
    def foilDict(self) -> dict:
        foil_list_raw = self._client.call("foilList") 
        return {item["name"]:Foil.from_msgpack(item, self._client) for item in foil_list_raw}
    
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

    def createNACAFoil(self, digits, name=None):
        if name == None:
            name  = "NACA" + str(digits)
        self._client.call("createNACAFoil", digits, name)

    def selectFoil(self, name):
        self._client.call("selectFoil", name)

    def showFoil(self, foilName, flag):
        self._client.call("showFoil", foilName, flag)
    
    def setFoilStyle(self, foil_style:FoilStyle):
        self._client.call("setFoilStyle", foil_style)
    

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

