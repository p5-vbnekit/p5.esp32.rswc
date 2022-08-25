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

        def _routine():
            _destination, _source = sys.argv[1:]
            assert bool(_source)
            assert bool(_destination)
            _destination = pathlib.PurePosixPath(_destination)
            assert bool(_destination.as_posix())
            if not _destination.is_absolute(): _destination = pathlib.PurePosixPath("/", _destination)
            _destination = _normalize(value = _destination).as_posix()
            assert bool(_destination)
            print(
                json.dumps({"source": _source, "destination": _destination}),
                file = sys.stdout, flush = True
            )

        class _Result(object):
            routine = _routine

        return _Result

    try: _private().routine()
    finally: del _private
