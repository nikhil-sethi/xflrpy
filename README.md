# XFLR5 for Ubuntu

*This repo provides the latest [XFLR5](http://www.xflr5.com/xflr5.htm) (currently v6.50) patched for Ubuntu 20.04 LTS*

## Installation

**1.** First of all, start by installing the required dependencies:
```
sudo apt install build-essential libgl1-mesa-dev qt5-default
```
Note: You will need Qt>=5.14 as QColorSpace has been included in the new xflr5 version.

**2.** Download the files from this repo and extract them:
```
git clone https://github.com/nikhil-sethi/xflr5-ubuntu.git
cd xflr5-ubuntu
```

**3.** Follow the standard install procedure:
```
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
ln -s /usr/local/share/xflr5/xflr5.desktop ~/.local/share/applications/
```
And now you should be able to launch *XFLR5* from the Activities Overview.

## Credits

Original code by techwinder on [SourceForge](https://sourceforge.net/projects/xflr5/). All credits to them for building such an awesome piece of software!

Distributed under the GNU [General Public License](https://www.gnu.org/licenses/gpl.html) (GPL) v3.
