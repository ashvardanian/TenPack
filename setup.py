import os
import re
import sys
import shutil
import subprocess
import platform
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

__version__ = open("VERSION", "r", encoding="utf-8").read()
__libname__ = "TenPack"

# Lets use README.md as `long_description`
this_directory = os.path.abspath(os.path.dirname(__file__))
with open(os.path.join(this_directory, "README.md")) as f:
    long_description = f.read()


class CMakeExtension(Extension):
    def __init__(self, name, source_dir=""):
        Extension.__init__(self, name, sources=[])
        self.source_dir = os.path.abspath(source_dir)


class CMakeBuild(build_ext):
    def build_extension(self, ext):
        extension_dir = os.path.abspath(os.path.dirname(self.get_ext_fullpath("tenpack")))

        if not extension_dir.endswith(os.path.sep):
            extension_dir += os.path.sep

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extension_dir}",
            f"-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY={extension_dir}",
            "-DCMAKE_BUILD_TYPE=Release",
            "-DTENPACK_BUILD_PYTHON=1",
        ]

        if "CMAKE_ARGS" in os.environ:
            cmake_args += [item for item in os.environ["CMAKE_ARGS"].split(" ") if item]

        if sys.platform.startswith("darwin"):
            # Cross-compile support for macOS - respect ARCHFLAGS if set
            archs = re.findall(r"-arch (\S+)", os.environ.get("ARCHFLAGS", ""))
            if archs:
                cmake_args += ["-DCMAKE_OSX_ARCHITECTURES={}".format(";".join(archs))]

        build_args = []
        if sys.platform.startswith("win32"):
            build_args += ["--config", "Release"]

        subprocess.check_call(["cmake", ext.source_dir] + cmake_args)
        subprocess.check_call(["cmake", "--build", ".", "--target", "tenpack_python"] + build_args)

        # Add these lines to copy the .so file to the expected directory
        if sys.platform.startswith("darwin"):
            suffix = "darwin.so"
        elif sys.platform.startswith("linux"):
            suffix = f"{platform.machine()}-linux-gnu.so"
        else:
            raise RuntimeError(f"Unsupported platform: {sys.platform}")

        expected_output = os.path.join(
            extension_dir,
            f"tenpack.cpython-{sys.version_info.major}{sys.version_info.minor}-{suffix}",
        )
        actual_output = os.path.join(extension_dir, "tenpack_python.so")
        shutil.copyfile(actual_output, expected_output)


setup(
    name=__libname__,
    version=__version__,
    author="Ash Vardanian",
    author_email="1983160+ashvardanian@users.noreply.github.com",
    url="https://github.com/ashvardanian/TenPack",
    description="Python bindings for TenPack, multimedia unpacking library.",
    long_description=long_description,
    long_description_content_type="text/markdown",
    zip_safe=False,
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "Intended Audience :: Information Technology",
        "License :: Other/Proprietary License",
        "Natural Language :: English",
        "Operating System :: POSIX",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: C",
        "Programming Language :: C++",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: Implementation :: CPython",
    ],
    py_modules=["tenpack"],
    extras_require={"test": "pytest"},
    ext_modules=[CMakeExtension("tenpack")],
    cmdclass={"build_ext": CMakeBuild},
)
