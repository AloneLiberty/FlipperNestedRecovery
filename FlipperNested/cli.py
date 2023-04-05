import argparse

from FlipperNested.main import FlipperNested


def main():
    parser = argparse.ArgumentParser(description='Calculate keys after Nested attack')
    parser.add_argument('--uid', type=str, help='Recover only for this UID', default='')
    parser.add_argument('--save', action='store_true', help='Debug: Save keys/nonces from Flipper')
    parser.add_argument('--file', type=argparse.FileType('r'), help='Debug: recover keys from local .nonces file')
    # parser.add_argument('--progress_bar', action='store_true', help='Debug: show key recovery progress')
    parser.set_defaults(debug=False)
    args = parser.parse_args()
    flipper = FlipperNested()
    flipper.run(args)
