import re
from multiprocessing import Queue, Process
from FlipperNested.bridge import FlipperBridge

Q = Queue()


def calculate_keys(uid, nt0, ks0, par0, nt1, ks1, par1, bruteforce, bruteforce_distance, bruteforce_depth):
    Q.put(FlipperNested.calculate_keys(uid, nt0, ks0, par0, nt1, ks1, par1, bruteforce, bruteforce_distance,
                                       bruteforce_depth))


class FlipperNested:
    def __init__(self, skip_connection=False):
        self.connection = None
        if not skip_connection:
            self.connection = FlipperBridge()

    def crack_nonces(self, args=None):
        bruteforce = False
        bruteforce_distance = 0
        bruteforce_depth = 0
        uid = 0
        target_uid = ''
        save = False
        if args:
            target_uid = args.uid
            save = args.save

        for file in self.connection.get_files("/ext/nfc/nested"):
            if file['name'].endswith(".nonces") and target_uid in file['name']:
                content = self.connection.file_read("/ext/nfc/nested/" + file['name']).decode()

                nonce = {
                    'A': {},
                    'B': {},
                }

                found_keys = {
                    'A': {},
                    'B': {},
                }

                print('Checking', file['name'])
                lines = content.splitlines()[1:]
                if lines.pop(0) != "Version: 2":
                    print("Invalid version for", file['name'])
                    print("Consider updating app or recovery script")
                    continue
                if save:
                    open(file['name'], 'w+').write('\n'.join(lines))
                    print("Saved nonces to", file['name'])

                if "Nested: Delay" in content:
                    print(
                        "[Warning] Nested attack with delay was used, will try more PRNG values (will take more time)")
                    result = re.search(
                        r'Nested: Delay [0-9]*, distance ([0-9]*)',
                        content.strip())
                    bruteforce = True
                    print("[Info] Please select depth of check")
                    print("[1] Fast: +-25 values [Not recommended]")
                    print("[2] Normal: +-50 values")
                    print("[3] Full: +-100 values [Recommended, ~4Gb RAM usage]")
                    bruteforce_depth = int(input("[1-3] > "))
                    if bruteforce_depth < 1 or bruteforce_depth > 3:
                        print("Invalid input, using Normal depth")
                        bruteforce_depth = 2
                    bruteforce_distance = int(result.groups()[0])
                    lines.pop()

                for line in lines:
                    uid, nt0, ks0, par0, nt1, ks1, par1, sec, key_type = self.parse_line(line)

                    try:
                        nonce[key_type][sec]
                    except:
                        nonce[key_type][sec] = []
                        found_keys[key_type][sec] = []

                    nonce[key_type][sec].append(
                        '{}:{}:{}:{}:{}:{}'.format(str(ks0), str(ks1), str(nt0), str(nt1), str(par0), str(par1)))

                if not uid:
                    continue

                for key_type in ['A', 'B']:
                    for sector in nonce[key_type].keys():
                        for info in nonce[key_type][sector]:
                            ks0, ks1, nt0, nt1, par0, par1 = list(
                                map(lambda x: x, list(map(lambda x: int(x), info.split(":")))))
                            print("Calculating for key type", key_type + ", sector", sector)

                            p = Process(target=calculate_keys,
                                        args=(
                                            uid, nt0, ks0, par0, nt1, ks1, par1, bruteforce, bruteforce_distance,
                                            bruteforce_depth))
                            p.start()
                            p.join()
                            keys_raw = Q.get()
                            keys = keys_raw.split(";")
                            keys.pop()

                            print(f"Found {str(len(keys))} key(s):", keys)

                            if keys:
                                found_keys[key_type][sector].extend(keys)
                                break
                            elif info == nonce[key_type][sector][-1]:
                                print(
                                    "Failed to find keys for this sector, try running Nested attack again" + ("or try "
                                                                                                              "increasing "
                                                                                                              "depth" if
                                                                                                              bruteforce_depth != 3 else ""))

                output = ""

                for key_type in ['A', 'B']:
                    for sector in found_keys[key_type].keys():
                        for key in found_keys[key_type][sector]:
                            output += f"Key {key_type} sector {str(sector).zfill(2)}: " + " ".join(
                                [key.upper()[i:i + 2] for i in range(0, len(key), 2)]) + "\n"

                key_count = output.count("\n")
                if not key_count:
                    print("No keys found")
                else:
                    if save:
                        open(file['name'].replace('nonces', 'keys'), 'w+').write(output)
                        print("Saved keys to", file['name'].replace('nonces', 'keys'))
                    try:
                        self.connection.file_write("/ext/nfc/nested/" + file['name'].replace('nonces', 'keys'),
                                                   output.encode())
                    except:
                        open(file['name'].replace('nonces', 'keys'), 'w+').write(output)
                        print("Lost connection to Flipper!")
                        print("Saved keys to", file['name'].replace('nonces', 'keys'))
                    print(f'Found potential {str(key_count)} keys, use "Check found keys" in app')

    @staticmethod
    def parse_line(line):
        result = re.search(
            r'Nested: Key ([A-B]) cuid (0x[0-9a-f]*) nt0 (0x[0-9a-f]*) ks0 (0x[0-9a-f]*) par0 ([0-9a-f]*) nt1 (0x[0-9a-f]*) ks1 (0x[0-9a-f]*) par1 ([0-9a-f]*) sec (\d{1,2})',
            line.strip())
        groups = result.groups()

        key_type, sec = groups[0], int(groups[-1])
        uid, nt0, ks0, par0, nt1, ks1, par1 = list(
            map(lambda x: int(x, 16) if x.startswith("0x") else int(x), groups[1:-1]))
        return uid, nt0, ks0, par0, nt1, ks1, par1, sec, key_type

    @staticmethod
    def calculate_keys(uid, nt0, ks0, par0, nt1, ks1, par1, bruteforce, bruteforce_distance, bruteforce_depth):
        import nested
        if bruteforce:
            to_add = 100
            if bruteforce_depth == 1:
                to_add = 25
            elif bruteforce_depth == 2:
                to_add = 50
            run = nested.run_full_nested(uid, nt0, ks0, par0, nt1, ks1, par1, bruteforce_distance - to_add,
                                         bruteforce_distance + to_add)
        else:
            run = nested.run_nested(uid, nt0, ks0, nt1, ks1)
        return run
