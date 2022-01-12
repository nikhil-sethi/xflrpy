import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="xflrpy",
    version="0.2.0",
    author="Nikhil Sethi",
    author_email="sethi.nirvil@gmail.com",
    description="A python client for design optimization and scripting with XFLR5",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/nikhil-sethi/xflrpy",
    packages=setuptools.find_packages(),
	license='GPLv3',
    classifiers=(
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
        "Operating System :: OS Independent",
    ),
    install_requires=[
          'msgpack-rpc-python'
    ]
)
