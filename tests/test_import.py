def test_import():
    from FlipperNested.main import FlipperNested
    flipper = FlipperNested()
    assert flipper.__class__.__name__ == "FlipperNested"