import _queue
import re
from multiprocessing import Manager, Process
from FlipperNested.bridge import FlipperBridge


def wrapper(queue, *args):
    queue.put(FlipperNested.calculate_keys(*args))


class FlipperNested:
    VERSION = 2
    DEPTH_VALUES = {1: 25, 2: 50, 3: 100}
    FLIPPER_PATH = "/ext/nfc/nested/"

    def __init__(self):
        self.connection = None
        self.filename = None
        self.nonces = None
        self.found_keys = None
        self.bruteforce_distance = [0, 0]
        self.progress_bar = False
        self.save = False
        self.preserve = False
        self.uid = ""

    def run(self, args=None):
        if args:
            self.progress_bar = args.progress
            self.save = args.save
            self.preserve = args.preserve
            self.uid = args.uid
        if not args or args and not args.file:
            self.connection = FlipperBridge()
            self.extract_nonces_from_flipper()
        else:
            self.extract_nonces_from_file(args.file)

    def parse_file(self, contents):
        self.nonces = {"A": {}, "B": {}}
        self.found_keys = {"A": {}, "B": {}}
        self.bruteforce_distance = [0, 0]
        lines = contents.splitlines()[1:]
        version_string = lines.pop(0)
        if "Version" not in version_string:
            print("No version info in", self.filename)
            return False
        file_version = int(version_string.split(": ")[1])
        if file_version != self.VERSION:
            print("Invalid version for", self.filename)
            print("You should update " + "app" if file_version < self.VERSION else "recovery script")
            return False
        if "Nested: Delay" in contents:
            print("[Warning] Nested attack with delay was used, will try more PRNG values (will take more time)")
            result = re.search(r"Nested: Delay [0-9]*, distance ([0-9]*)", contents.strip())
            print("[Info] Please select depth of check")
            print("[1] Fast: +-25 values")
            print("[2] Normal: +-50 values")
            print("[3] Full: +-100 values [Recommended, ~2Gb RAM usage]")
            print("[-] Custom [..any other value..]")
            depth = int(input("[1-3/custom] > "))
            distance = int(result.groups()[0])
            if depth < 1:
                print("Invalid input, using Normal depth")
                depth = 2
            if depth < 4:
                self.bruteforce_distance = [distance - self.DEPTH_VALUES[depth], distance + self.DEPTH_VALUES[depth]]
            else:
                self.bruteforce_distance = [distance - depth, distance + depth]
            lines.pop()
        for line in lines:
            values = self.parse_line(line)
            sec, key_type = values[-2:]
            if not sec in self.nonces[key_type].keys():
                self.nonces[key_type][sec] = []
            self.nonces[key_type][sec].append(values)
        return len(self.nonces["A"]) + len(self.nonces["B"]) > 0

    def extract_nonces_from_flipper(self):
        for file in self.connection.get_files(self.FLIPPER_PATH[:-1]):
            if file["name"].endswith(".nonces"):
                if self.uid:
                    if file["name"].split(".")[0] != self.uid.upper():
                        continue
                self.filename = file["name"]
                print("Checking", file["name"])
                contents = self.connection.file_read(self.FLIPPER_PATH + file["name"]).decode()
                if not self.parse_file(contents):
                    print("Failed to parse", file["name"])
                    continue
                if self.save:
                    open(file["name"], "w+").write(contents)
                    print("Saved nonces to", file["name"])
                if self.recover_keys():
                    break
                self.save_keys_to_flipper()

    def extract_nonces_from_file(self, file):
        self.filename = file.name
        if not self.parse_file(file.read()):
            print("Failed to parse", self.filename)
            return
        self.recover_keys()
        self.save_keys_to_file()

    def recover_keys(self):
        for key_type in self.nonces.keys():
            for sector in self.nonces[key_type].keys():
                for info in self.nonces[key_type][sector]:
                    print("Recovering key type", key_type + ", sector", sector)
                    m = Manager()
                    q = m.Queue()

                    value = info[:-2]
                    value.append(self.bruteforce_distance)
                    value.append(self.progress_bar)
                    value.insert(0, q)

                    p = Process(target=wrapper, args=value)
                    p.start()

                    try:
                        p.join()
                    except KeyboardInterrupt:
                        print("Stopping...")
                        p.kill()
                        return True

                    try:
                        keys = q.get(timeout=1).split(";")
                    except _queue.Empty:
                        print("Something went VERY wrong in key recovery.\nYou MUST report this to developer!")
                        return
                    keys.pop()

                    print(f"Found {str(len(keys))} key(s):", keys)

                    if keys:
                        self.found_keys[key_type][sector] = keys
                        break
                    elif info == self.nonces[key_type][sector][-1]:
                        print("Failed to find keys for this sector, try running Nested attack again")

    def save_keys_to_string(self):
        output = ""
        for key_type in self.found_keys.keys():
            for sector in self.found_keys[key_type].keys():
                for key in self.found_keys[key_type][sector]:
                    output += f"Key {key_type} sector {str(sector).zfill(2)}: " + " ".join(
                        [key.upper()[i:i + 2] for i in range(0, len(key), 2)]) + "\n"

        keys = output.count("Key")
        if keys:
            print("Found potential {} keys, use \"Check found keys\" in app".format(keys))
        return output.strip()

    def save_keys_to_file(self):
        output = self.save_keys_to_string()
        if not output:
            print("No keys found!")
            return
        filename = self.filename + ".keys"
        open(filename, "w+").write(output)
        print("Saved keys to", filename)

    def save_keys_to_flipper(self):
        output = self.save_keys_to_string()
        if not output:
            print("No keys found!")
            return
        filename = self.filename.replace("nonces", "keys")
        if self.save:
            open(filename, "w+").write(output)
            print("Saved keys to", filename)
        try:
            self.connection.file_write(self.FLIPPER_PATH + filename, output.encode())
            if not self.preserve:
                self.connection.file_delete(self.FLIPPER_PATH + self.filename)
        except:
            if not self.save:
                open(filename, "w+").write(output)
                print("Lost connection to Flipper!")
                print("Saved keys to", filename)

    @staticmethod
    def parse_line(line):
        result = re.search(
            r"Nested: Key ([A-B]) cuid (0x[0-9a-f]*) nt0 (0x[0-9a-f]*) ks0 (0x[0-9a-f]*) par0 ([0-9a-f]*) nt1 (0x[0-9a-f]*) ks1 (0x[0-9a-f]*) par1 ([0-9a-f]*) sec (\d{1,2})",
            line.strip())
        groups = result.groups()

        key_type, sec = groups[0], int(groups[-1])
        values = list(map(lambda x: int(x, 16) if x.startswith("0x") else int(x), groups[1:-1]))
        values.append(sec)
        values.append(key_type)
        return values

    @staticmethod
    def calculate_keys(uid, nt0, ks0, par0, nt1, ks1, par1, bruteforce_distance, progress):
        import faulthandler
        faulthandler.enable()
        import nested
        if bruteforce_distance != [0, 0]:
            run = nested.run_full_nested(uid, nt0, ks0, par0, nt1, ks1, par1, bruteforce_distance[0],
                                         bruteforce_distance[1], progress)
        else:
            run = nested.run_nested(uid, nt0, ks0, nt1, ks1)
        return run
