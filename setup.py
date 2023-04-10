from distutils.core import Extension
import setuptools
import os

include_dirs = []
extra_compile_args = []
libraries = []

if os.name == 'nt':
    include_dirs = ['build\\pthreads']
    extra_compile_args = ['-DNEED_FTIME']
    libraries = ['pthreadVC2']

nested_solver = Extension('nested',
                          sources=['NestedSolver/python.c', 'NestedSolver/crapto1.c', 'NestedSolver/library.c',
                                   'NestedSolver/bucketsort.c', 'NestedSolver/crypto1.c'], include_dirs=include_dirs,
                          library_dirs=include_dirs, extra_compile_args=extra_compile_args, libraries=libraries)

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="FlipperNested",
    version="1.7.1",
    author="AloneLiberty",
    description="Recover keys from collected nonces",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/AloneLiberty/FlipperNestedRecovery",
    entry_points={'console_scripts': ['FlipperNested = FlipperNested.cli:main']},
    install_requires=['protobuf>4', 'pyserial'],
    ext_modules=[nested_solver],
    packages=["FlipperNested", "FlipperNested.proto"],
    python_requires='>=3.8',
    classifiers=[
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
        "Operating System :: MacOS",
        "Operating System :: POSIX :: Linux",
        "Operating System :: Microsoft :: Windows"
    ],
    headers=['NestedSolver/library.h', 'NestedSolver/parity.h',
             'NestedSolver/crapto1.h', 'NestedSolver/bucketsort.h'],
)