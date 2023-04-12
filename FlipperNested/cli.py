import argparse

from FlipperNested.main import FlipperNested


def main():
    parser = argparse.ArgumentParser(description="Recover keys after Nested attack")
    parser.add_argument("--uid", type=str, help="Recover only for this UID", default="")
    parser.add_argument("--progress", action="store_true", help="Show key recovery progress bar")
    parser.add_argument("--save", action="store_true", help="Debug: Save nonces/keys from Flipper")
    parser.add_argument("--preserve", action="store_true", help="Debug: Don't remove nonces after recovery")
    parser.add_argument("--file", type=argparse.FileType("r"), help="Debug: Recover keys from local .nonces file")
    parser.set_defaults(debug=False)
    args = parser.parse_args()
    flipper = FlipperNested()
    flipper.run(args)
