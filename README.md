# XFLR5 for Ubuntu

*This repo provides the latest [XFLR5](http://www.xflr5.com/xflr5.htm) (currently v6.42) patched for Ubuntu 16.04 LTS using Qt 5.11.0*

## Installation

**1** First of all, start by installing the required dependencies:
```
sudo apt-get install build-essential mesa-common-dev mesa-utils libgl1-mesa-dev libglu1-mesa-dev lingl-dev libqt4-dev
```

**2** Next, head over to [Qt.io](https://www.qt.io/download-qt-installer) and download the Linux installer. Run it and make sure to install Qt 5.11.0 for GCC 64-bit.
If you don't change the default options, it will be installed in `~/Qt/5.11.0`.

**3** Download the files from this repo and extract them:
```
cd /path/to/repo
git clone https://github.com/polmes/xflr5-ubuntu.git
```

**4** Move the main `xflr5` folder to your preferred install location:
```
cd /path/to/repo
sudo mv xflr5/ /opt/
```

**5** Follow the standard install procedure:
```
cd /opt/xflr5/
~/Qt/5.11.0/gcc_64/bin/qmake
make # will take a while
sudo make install # create executable
sudo ldconfig # make the new shared libraries available
```

At this point, you should be all ready to go. You can check by running XFLR5 from the command line:
```
/usr/local/bin/xflr5
```

**6** Finally, in order to run the software more easily, move the provided `*.desktop` file to your preferred location:
```
cd /path/to/repo
mv xflr5.desktop ~/.local/share/applications/
```
And now you should be able to launch *XFLR5* from the Dash.

## Credits

Original code by techwinder on [SourceForge](https://sourceforge.net/projects/xflr5/). All credits to them for building such an awesome piece of software!

Distributed under the GNU [General Public License](https://www.gnu.org/licenses/gpl.html) (GPL) v3.
