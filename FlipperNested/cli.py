import argparse

from FlipperNested.main import FlipperNested


def main():
    parser = argparse.ArgumentParser(description='Calculate keys after Nested attack')
    parser.add_argument('--uid', type=str, help='Recover only for this UID', default='')
    parser.add_argument('--save', action=argparse.BooleanOptionalAction, help='Debug: Save keys/nonces from Flipper',
                        default=False)
    args = parser.parse_args()
    flipper = FlipperNested()
    flipper.crack_nonces(args)
