import enum

from matplotlib.pyplot import polar

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

# ============= Miscellaneous =============== # 
class QColor(MsgpackMixin, list):
# will be a list not dict  
        red = 0
        green = 0
        blue = 0
        alpha = 0

class enumLineStipple(enum.IntEnum):
# xflr5v6/xflcore/line_enums.h   
    SOLID = 0
    DASH = 1
    DOT = 2
    DASHDOT = 3
    DASHDOTDOT = 4
    NOLINE = 5

class enumPointStyle(enum.IntEnum):
# xflr5v6/xflcore/line_enums.h
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

class LineStyle(MsgpackMixin):
    visible = True
    stipple = enumLineStipple.SOLID
    point_style = enumPointStyle.NOSYMBOL
    width = 1
    color = []
    tag = ""

    def __init__(self, visible = True, stipple =  enumLineStipple.SOLID, point_style = enumPointStyle.NOSYMBOL, width = 1, color = [], tag ="") -> None:
        self.visible = visible
        self.stipple = stipple
        self.point_style = point_style
        self.width = width
        self.color = color
        self.tag = tag

# =========== AFoil classes ================ #
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
        return self._client.call("getFoilCoords", self.name)
    
    @coords.setter
    def coords(self, xy:list):
        self._client.call("setFoilCoords", self.name, xy)

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
        foil_raw = self._client.call("duplicateFoil", self.name, toName)
        return self.from_msgpack(foil_raw, self._client)

    def delete(self) -> None:
        self._client.call("deleteFoil", self.name)

    def rename(self, name):
        self.name = name    # python side
        self._client.call("renameFoil", self.name, name)    # cpp side
    
    def normalize(self):
        self._client.call("normalizeFoil", self.name)

    def derotate(self):
        self._client.call("derotateFoil", self.name)

    def normalize(self):
        self._client.call("normalizeFoil", self.name)

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

    def exportFoil(self, name, file_name):
        """
        name: name of the airfoil to export
        file_name: full path of the to be saved airfoil 
        """
        self._client.call("exportFoil", name, file_name)

# =========== XDirect classes ================ #
class enumSequenceType(enum.IntEnum):
    ALPHA = 0
    CL = 1
    REYNOLDS = 2

class enumPolarType(enum.IntEnum):
    FIXEDSPEEDPOLAR = 0 
    FIXEDLIFTPOLAR = 1
    RUBBERCHORDPOLAR = 2
    FIXEDAOAPOLAR = 3
    STABILITYPOLAR = 4
    BETAPOLAR = 5

class AnalysisSettings2D(MsgpackMixin):
    sequence_type = enumSequenceType.ALPHA
    sequence = (0,0,0)
    is_sequence = False
    init_BL = True
    store_opp = True
    viscous = True
    keep_open_on_error = False     

    def __init__(self, sequence_type = enumSequenceType.ALPHA, sequence = (0.,0.,0.), is_sequence = False, init_BL =  True, store_opp = True, viscous = True, keep_open_on_error = False) -> None:
        self.sequence_type = sequence_type
        self.sequence = sequence
        self.is_sequence = is_sequence
        self.init_BL = init_BL
        self.store_opp = store_opp
        self.viscous = viscous
        self.keep_open_on_error = keep_open_on_error
        
class PolarSpec(MsgpackMixin):
    polar_type = enumPolarType
    Re_type = 1
    ma_type = 1
    aoa = 0.0
    mach = 0.0
    ncrit = 9.0
    xtop = 1.0
    xbot = 1.0
    reynolds = 100000.0

    def __init__(self, polar_name = "", polar_type = enumPolarType.FIXEDSPEEDPOLAR, re_type = 1, ma_type = 1, aoa = 0.0, mach = 0.0, ncrit = 9.0, xtop = 1.0, xbot = 1.0, reynolds = 100000.0) -> None:
        self.polar_name = polar_name
        self.polar_type = polar_type
        self.Re_type = re_type
        self.ma_type = ma_type
        self.aoa = aoa
        self.mach = mach
        self.ncrit = ncrit
        self.xtop = xtop
        self.xbot = xbot
        self.reynolds = reynolds

class PolarResult(MsgpackMixin):
    alpha = []
    Cl = []
    XCp = []
    Cd = []
    Cdp = []
    Cm = []
    XTr1 = []
    XTr2 = []
    HMom = []
    Cpmn = []
    ClCd = []
    Cl32Cd = []
    RtCl = []
    Re = []

class Polar(MsgpackMixin):
    name = ""
    foil_name = ""
    spec = PolarSpec()
    result = PolarResult()

    def __init__(self, name="", foil_name="", spec = PolarSpec(), result = PolarResult()) -> None:
        self.name = name
        self.foil_name = foil_name
        self.spec = spec
        self.result = result

class PolarManager:
    """
    Manager for polars.
    """
    def __init__(self, client) -> None:
        self._client = client

    def getPolar(self, foil_name:str, polar_name:str) -> Polar:
        polar_raw = self._client.call("getPolar", foil_name, polar_name)
        return Polar.from_msgpack(polar_raw)

# =========== Mainframe classes ============ #
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

    def showFoil(self, name, flag):
        self._client.call("showFoil", name, flag)
    
    def getLineStyle(self, name):
        line_style_raw = self._client.call("getLineStyle", name)
        line_style = LineStyle.from_msgpack(line_style_raw)
        line_style.point_style = enumPointStyle(line_style.point_style)
        line_style.stipple = enumLineStipple(line_style.stipple)
        return line_style

    def setLineStyle(self, name, line_style:LineStyle):
        line_style.stipple = line_style.stipple.value
        line_style.point_style = line_style.point_style.value
        self._client.call("setLineStyle", name, line_style.to_msgpack())
    
class Miarex:
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
        self.polar_mgr = PolarManager(client)

    def define_analysis(self, polar:Polar):
        self._client.call("defineAnalysis", polar.to_msgpack())

    def analyze(self, polar, analysis_settings: AnalysisSettings2D):
        return self._client.call("analyzePolar", polar, analysis_settings)

class XInverse(MsgpackMixin):
    """
    to manage the plane design application
    """
    def __init__(self, client) -> None:
        self._client = client
