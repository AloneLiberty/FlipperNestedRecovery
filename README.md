# Flipper Nested Recovery script

Script recovers keys from collected authorization attempts (nonces).
You can collect nonces on Flipper Zero with https://github.com/AloneLiberty/FlipperNested

Flipper Zero should be connected with USB cable and not used by other software (./fbt log, qFlipper)

You should use [MSYS2](https://www.msys2.org/) and install from sources to run on Windows.

## Installation

```bash
pip install --upgrade FlipperNested
```

or, install from sources:
```bash
pip install pyserial protobuf
python setup.py sdist bdist_wheel
pip install --user --no-index --upgrade --find-links=./dist FlipperNested
```

## Usage

```bash
$ FlipperNested
Checking 12345678.nonces
Calculating for key type A, sector 0
Found 1 key(s): ['ffffffffffff']
...
Found potential 32 keys, use "Check found keys" in app

$ FlipperNested --help
usage: FlipperNested [-h] [--uid UID] [--save | --no-save]

Calculate keys after Nested attack

options:
  -h, --help         show this help message and exit
  --uid UID          Recover only for this UID
  --save, --no-save  Debug: Save keys/nonces from Flipper (default: False)

```