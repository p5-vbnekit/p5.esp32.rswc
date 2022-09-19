#!/usr/bin/env python3


if "__main__" == __name__:
    def _private():
        import sys
        import json
        import pathlib

        def _normalize(value: pathlib.PurePosixPath):
            _collector = []
            for _part in value.parts:
                if "." == _part: continue
                if ".." == _part: _collector.pop(-1)
                else: _collector.append(_part)
            _collector = pathlib.PurePosixPath("/".join(_collector))
            if (_collector.is_absolute() and (not _collector.is_relative_to("/"))): _collector = "/" / _collector.relative_to("//")
            return _collector

        def _split(value: memoryview, chunk_size: int = 16):
            for offset_ in range(0, len(value), chunk_size):
                yield value[offset_ : offset_ + chunk_size]

        def _read_manifest(path: str):
            with open(path, "r") as _stream:
                while True:
                    _line = _stream.readline()
                    if not _line: break
                    _line = _line.strip()
                    assert bool(_line)
                    _line = json.loads(_line)
                    assert isinstance(_line, dict)
                    _source, _destination = _line.pop("source"), _line.pop("destination")
                    assert not _line
                    assert isinstance(_source, str) and bool(_source)
                    assert isinstance(_destination, str) and bool(_destination)
                    assert _normalize(value = pathlib.PurePosixPath(_source)).is_absolute()
                    _destination = _normalize(value = pathlib.PurePosixPath(_destination))
                    assert _destination.is_absolute()
                    _destination = _destination.relative_to("/").as_posix()
                    assert bool(_destination)
                    yield _destination, _source

        def _routine():
            _manifest, _output = sys.argv[1:]
            assert bool(_manifest)
            assert bool(_output)

            _keys = set()
            _image = [[], b"\0"]

            for _key, _source in _read_manifest(path = _manifest):
                with open(_source, "rb") as _source: _source = _source.read()
                assert _key not in _keys
                _keys.add(_key)
                _image[0].extend((_key, len(_source)))
                _image.append(_source)

            _image[0] = json.dumps(_image[0], separators = (",", ":")).encode("utf-8")
            _image = memoryview(bytes().join(_image))
            with open(_output, "w") as _output:
                print("""
.data
.section .rodata.embedded
.global p5_rswc_romfs_image_layout_end
.global p5_rswc_romfs_image_layout_begin
p5_rswc_romfs_image_layout_begin:
                    """.strip(), file = _output, flush = True
                )

                for _bytes in _split(value = _image): print(
                    ".byte {}".format(", ".join(["0x{:02x}".format(_byte) for _byte in _bytes])),
                    file = _output, flush = True
                )

                print("""
p5_rswc_romfs_image_layout_end:
                """.strip(), file = _output, flush = True
                )

        class _Result(object):
            routine = _routine

        return _Result

    try: _private().routine()
    finally: del _private
