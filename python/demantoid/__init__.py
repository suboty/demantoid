import ctypes
import os
import platform
from pathlib import Path

_platform = platform.system()

match _platform:
    case "Darwin":
        lib_name = "libdemantoid.so"
    case "Windows":
        lib_name = "demantoid.dll"
    case _:
        lib_name = "libdemantoid.so"

_possible_paths = [
    Path(os.path.dirname(__file__), lib_name),
    Path(os.path.dirname(__file__), "..", "..", "build", lib_name),
]

_lib_path = None
for p in _possible_paths:
    if os.path.exists(p):
        _lib_path = os.path.abspath(p)
        break

if _lib_path is None:
    raise OSError(
        f"Could not find {lib_name}!"
    )

_lib = ctypes.CDLL(_lib_path)

_lib.test_print.argtypes = []
_lib.test_print.restype = None

class DemantoidConn(ctypes.Structure):
    pass

DemantoidConnPtr = ctypes.POINTER(DemantoidConn)

_lib.demantoid_connect.argtypes = [ctypes.c_char_p]
_lib.demantoid_connect.restype = DemantoidConnPtr

_lib.demantoid_close.argtypes = [DemantoidConnPtr]
_lib.demantoid_close.restype = None

def test_print():
    _lib.test_print()

def demantoid_connect(connection_info: str) -> DemantoidConnPtr:
    return _lib.demantoid_connect(connection_info.encode("utf-8"))

def demantoid_close(conn: DemantoidConnPtr) -> None:
    return _lib.demantoid_close(conn)
