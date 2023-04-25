from distutils.core import Extension
import setuptools
import os

include_dirs = []
extra_compile_args = []
libraries = ['lzma']

if os.name == 'nt':
    include_dirs = ['build\\pthreads', 'build\\lzma']
    extra_compile_args = ['-DNEED_FTIME']
    libraries.extend(['pthreadVC2', 'Ws2_32'])

nested_solver = Extension('nested',
                          sources=['NestedSolver/python.c', 'NestedSolver/crapto1.c', 'NestedSolver/library.c',
                                   'NestedSolver/bucketsort.c', 'NestedSolver/crypto1.c', 'NestedSolver/progress.c'],
                          include_dirs=include_dirs, library_dirs=include_dirs, extra_compile_args=extra_compile_args,
                          libraries=libraries)

hardnested_solver = Extension('hardnested',
                              sources=['HardNestedSolver/pm3/crc32.c', 'HardNestedSolver/pm3/uart/uart_posix.c',
                                       'HardNestedSolver/pm3/uart/uart_win32.c', 'HardNestedSolver/pm3/crc.c',
                                       'HardNestedSolver/pm3/ui.c', 'HardNestedSolver/pm3/fileutils.c',
                                       'HardNestedSolver/pm3/crc16.c', 'HardNestedSolver/pm3/util.c',
                                       'HardNestedSolver/pm3/util_posix.c', 'HardNestedSolver/pm3/commonutil.c',
                                       'HardNestedSolver/pm3/crc64.c', 'HardNestedSolver/cmdhfmfhard.c',
                                       'HardNestedSolver/crapto1.c', 'HardNestedSolver/bucketsort.c',
                                       'HardNestedSolver/crypto1.c', 'HardNestedSolver/library.c',
                                       'HardNestedSolver/hardnested/hardnested_bf_core.c',
                                       'HardNestedSolver/hardnested/hardnested_bruteforce.c',
                                       'HardNestedSolver/hardnested/hardnested_bitarray_core.c',
                                       'HardNestedSolver/hardnested/tables.c',
                                       'HardNestedSolver/python.c'],
                              include_dirs=include_dirs, library_dirs=include_dirs,
                              extra_compile_args=extra_compile_args,
                              libraries=libraries)

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="FlipperNested",
    version="2.0.1",
    author="AloneLiberty",
    description="Recover keys from collected nonces",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/AloneLiberty/FlipperNestedRecovery",
    entry_points={'console_scripts': ['FlipperNested = FlipperNested.cli:main']},
    install_requires=['protobuf>4', 'pyserial'],
    ext_modules=[nested_solver, hardnested_solver],
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
             'NestedSolver/crapto1.h', 'NestedSolver/bucketsort.h',
             'NestedSolver/progress.h', 'HardNestedSolver/pm3/util_darwin.h', 'HardNestedSolver/pm3/uart/uart.h',
             'HardNestedSolver/pm3/pm3_cmd.h', 'HardNestedSolver/pm3/util.h', 'HardNestedSolver/pm3/ui.h',
             'HardNestedSolver/pm3/crc64.h', 'HardNestedSolver/pm3/common.h', 'HardNestedSolver/pm3/fileutils.h',
             'HardNestedSolver/pm3/util_posix.h', 'HardNestedSolver/pm3/ansi.h', 'HardNestedSolver/pm3/crc32.h',
             'HardNestedSolver/pm3/crc.h', 'HardNestedSolver/pm3/commonutil.h', 'HardNestedSolver/pm3/comms.h',
             'HardNestedSolver/pm3/emojis_alt.h', 'HardNestedSolver/pm3/crc16.h', 'HardNestedSolver/pm3/emojis.h',
             'HardNestedSolver/bucketsort.h', 'HardNestedSolver/crapto1.h', 'HardNestedSolver/parity.h',
             'HardNestedSolver/hardnested/hardnested_benchmark_data.h',
             'HardNestedSolver/hardnested/hardnested_bf_core.h',
             'HardNestedSolver/hardnested/hardnested_bitarray_core.h',
             'HardNestedSolver/hardnested/hardnested_bruteforce.h', 'HardNestedSolver/hardnested/tables.h',
             'HardNestedSolver/library.h', 'HardNestedSolver/cmdhfmfhard.h'],
)
