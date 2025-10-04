# How?

Some standard ways of exposing C code to python include using wrappers like [SWIG](https://github.com/swig/swig), 
Boost and [PyBind11](https://github.com/pybind/pybind11). Luckily for us, xflr5 is written using [Qt](https://www.qt.io/) 
which means there exist even more tools like [shiboken](https://github.com/pyside/Shiboken)
and [PythonQt](https://github.com/MeVisLab/pythonqt) to make life easier. 

There are two approaches that I explored: 
### pythonqt:
This version comes with an embedded python interpreter within xflr5 itself. 

Merits:
- Has entire Python classes with signals and everything exposed in the custom interpreter. 
- It is easier and safer to expose more Qt objects to python.

Demerits:
- The main library is not well-maintained/documented and has been replaced by better tools like PySide6 (this doesn't have an embedded option tho).
- It requires PythonQt as dependency which is a fairly large library.
- Everything has to be exposed under the __main__ function which means there is little to no chance of multithreading, doing complex loops or IO operations. (These might be critical for optimization)
- The above problem also means there is no abstraction and well-defined objects which make the code difficult to read and extend.
- It is difficult to use external optimization libraries within the interpreter.

### rpc:
This is the stuff I started working on recently. The approach uses the very light and very fast [rpclib](https://github.com/rpclib/rpclib) library to establish a local server which can communicate with and external python process. This means that we finally have an ["xflrpy" python library](https://pypi.org/project/xflrpy/).

Merits:
- Solves everything wrong with pythonqt.
- Very neat.
- Can extend xflr5 to any programming language which uses rpc and msgpack.
- More code to write.

Demerits:
- More code to write.
- There might be concurrency problems in the future as I add more features.


Because of the above reasons, I won't be maintaining the pythonqt approach but it will be long-lived to help people learn something/anything from it. The rpc approach has been merged to the master branch.
Note that as of 28/12/21 v0.2.0, the pythonqt branch has more and better features than rpc. If you want to whip up something quick, I reccommend you use that. But this will change soon enough. I will be adding more features to rpc and hope to move this statement to the changelog ASAP :) 
