from setuptools import setup, Extension

module = Extension(
    "fwlib",
    sources=["fwlib.c", "gcode_map.c"],
    include_dirs=["."],
    libraries=["fwlib32"],
)

setup(
    name="fwlib",
    version="1.0",
    description="FANUC FOCAS library Python wrapper",
    ext_modules=[module],
)
