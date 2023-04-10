# Flipper Nested Recovery script

Script recovers keys from collected authorization attempts (nonces).
You can collect nonces on Flipper Zero with https://github.com/AloneLiberty/FlipperNested

#### Flipper Zero should be connected with USB cable and not used by ANY other software (./fbt log, qFlipper, lab.flipper.net)

## Installation

```bash
pip install --upgrade FlipperNested
```

or, install from sources:
```bash
pip install --upgrade pyserial protobuf wheel setuptools
python setup.py sdist bdist_wheel
pip install --user --upgrade --find-links=./dist FlipperNested
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