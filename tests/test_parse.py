from FlipperNested.main import FlipperNested


def test_parse():
    values = FlipperNested.parse_line(
        "Nested: Key B cuid 0x9a22bf95 nt0 0x60011fd9 ks0 0x21098875 par0 1111 nt1 0xd274da74 ks1 0x6a12a32f par1 "
        "1111 sec 15")
    assert values == [0x9a22bf95, 0x60011fd9, 0x21098875, 1111, 0xd274da74, 0x6a12a32f, 1111, 15, 'B']
