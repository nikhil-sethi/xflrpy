import enum

class MsgpackMixin:
    def __repr__(self):
        from pprint import pformat
        return "<" + type(self).__name__ + "> " + pformat(vars(self), indent=4, width=1)
    def to_msgpack(self, *args, **kwargs):
        return self.__dict__
    @classmethod
    def from_msgpack(cls, encoded):
        obj = cls()
        # obj.__dict__ = {k.decode('utf-8'): (v.__class__.from_msgpack(v.__class__, v) if hasattr(v, "__dict__") else v) for k, v in encoded.items()}
        obj.__dict__ = { k : (v if not isinstance(v, dict) else getattr(getattr(obj, k).__class__, "from_msgpack")(v)) for k, v in encoded.items()}
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
    def __init__(self, client) -> None:
            self._client = client

    # def getPolar(self, polar_name):
    #     polar_raw = self.client.call('getPolar', self.name, polar_name) 
    #     return Polar.from_msgpack(polar_raw)

    # def getCurrPolar(self, polar_name):
    #     polar_raw = self.client.call('getCurrPolar', self.name) 
    #     return Polar.from_msgpack(polar_raw)
 
class Afoil(MsgpackMixin):
    """
    to manage the airfoil design application
    """
    def __init__(self, client) -> None:
        self._client = client
    
    def getFoil(self):
        pass
    def setFoil(self, foil):
        pass

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

