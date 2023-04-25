import os.path
import pathlib
from FlipperNested.main import FlipperNested


def test_calculate():
    keys = FlipperNested.calculate_keys(0x29c6824a, 0x292daf7a, 0xff3f91be, 1111, 0x48f9f977, 0xffbf94fd, 1100, [0, 0],
                                        False)
    assert keys == "ffffffffffff;"


def test_calculate_full():
    keys = FlipperNested.calculate_keys(0x9a22bf95, 0x60011fd9, 0x21098875, 1111, 0xd274da74, 0x6a12a32f, 1111,
                                        [805, 810], False)
    assert keys == "20b9f1ebffff;9088dcfc2ffe;45baf6bffeff;15f9fefeaffe;ffffffffffff;"


def test_calculate_hard():
    file = str(pathlib.Path(__file__).parent.resolve()) + "/.clean_nonces"
    if os.path.isfile(file):
        keys = FlipperNested.calculate_keys_hard(0x773D6B86, file)
        assert keys == "89eca97f8c2a;"
