# XFLR5 for Ubuntu

*This repo provides the latest [XFLR5](http://www.xflr5.tech/xflr5.htm) (currently v6.56) patched for Ubuntu 20.04 LTS*

## Installation

**1.** First of all, start by installing the required dependencies:
```
sudo apt install build-essential libgl1-mesa-dev qt5-default
```

**2.** Download the files from this repo and extract them:
```
cd /path/to/repo/
git clone https://github.com/polmes/xflr5-ubuntu.git
```

**3.** Follow the standard install procedure:
```
cd /path/to/repo/xflr5/
qmake
make # will take a while
sudo make install # create executable
sudo ldconfig # make the new shared libraries available
```

At this point, you should be all ready to go. You can check by running XFLR5 from the command line:
```
/usr/local/bin/xflr5
```

**4.** Finally, in order to run the software more easily, link the provided `*.desktop` file to your preferred location:
```
ln -sf /usr/local/share/xflr5/xflr5.desktop ~/.local/share/applications/
```
And now you should be able to launch *XFLR5* from the Activities Overview.

## Credits

Original code by techwinder on [SourceForge](https://sourceforge.net/projects/xflr5/). All credits to them for building such an awesome piece of software!

Distributed under the GNU [General Public License](https://www.gnu.org/licenses/gpl.html) (GPL) v3.
