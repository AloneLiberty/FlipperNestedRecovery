# Flipper Nested Recovery script

Script recovers keys from collected authorization attempts (nonces).
You can collect nonces on Flipper Zero with [FlipperNested](https://github.com/AloneLiberty/FlipperNested)

#### Flipper Zero should be connected with USB cable and not used by ANY other software (./fbt log, qFlipper, lab.flipper.net)

## Installation

<details>
<summary>Linux</summary>
<br>
```bash
pip install --upgrade FlipperNested
```

or, install from sources:
```bash
pip install --upgrade pyserial protobuf wheel setuptools
python setup.py sdist bdist_wheel
pip install --user --upgrade --find-links=./dist FlipperNested
```
</details>

<details>
<summary>Windows</summary>
<br>
First, install Windows Subsystem for Linux (WSL) and Ubuntu. To do so,

1. Access the "Add or remove Windows features" menu by searching for it in the start menu.
2. Check the box next to "Windows Subsystem for Linux" and click "OK".
3. Open the terminal and run the following command to install Ubuntu:

```bash
wsl --install Ubuntu
```

4. Pick an username and a password when prompted.
5. Run the following command to update the package list and install the necessary packages:

```bash
sudo apt update && sudo apt upgrade -y && sudo apt install python3 python3-pip pipx git -y && pipx install FlipperNested && pipx ensurepath
```

6. Plug in your flipper, close the terminal and open a new *administrator* terminal, and install this utility to let your flipper connect to WSL: 

```bash
winget install usbipd && usbipd list
```

7. Your flipper will be listed as "Serial USB Device" or similar. Take note of its BUSID, then run the following command to connect it to WSL:

```bash
usbipd attach --wsl --busid <id>
```

8. Return to wsl by running `wsl` and run the following command to start the key recovery:

```bash
FlipperNested
```
</details>


## Usage

```bash
$ FlipperNested
[?] Checking 12345678.nonces
Recovering key type A, sector 0
Found 1 key(s): ['ffffffffffff']
...
[+] Found potential 32 keys, use "Check found keys" in app
```

```bash
$ FlipperNested --help
usage: FlipperNested [-h] [--uid UID] [--progress] [--save] [--preserve] [--file FILE]

Recover keys after Nested attack

options:
  -h, --help   show this help message and exit
  --uid UID    Recover only for this UID
  --port PORT  Port to connect
  --progress   Show key recovery progress bar
  --save       Debug: Save nonces/keys from Flipper
  --preserve   Debug: Don't remove nonces after recovery
  --file FILE  Debug: Recover keys from local .nonces file
```
