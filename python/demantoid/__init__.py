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

class DemantoidConn(ctypes.Structure):
    pass

DemantoidConnPtr = ctypes.POINTER(DemantoidConn)

_lib.demantoid_connect.argtypes = [ctypes.c_char_p]
_lib.demantoid_connect.restype = DemantoidConnPtr

_lib.demantoid_close.argtypes = [DemantoidConnPtr]
_lib.demantoid_close.restype = None

_lib.create_experiment.argtypes = [DemantoidConnPtr, ctypes.c_char_p]
_lib.create_experiment.restype = ctypes.c_bool

_lib.add_metric.argtypes = [DemantoidConnPtr, ctypes.c_int, ctypes.c_int, ctypes.c_char_p, ctypes.c_float]
_lib.add_metric.restype = ctypes.c_bool

def demantoid_connect(connection_info: str) -> DemantoidConnPtr:
    return _lib.demantoid_connect(connection_info.encode("utf-8"))

def demantoid_close(conn: DemantoidConnPtr) -> None:
    return _lib.demantoid_close(conn)

def create_experiment(conn: DemantoidConnPtr, experiment_info: str) -> bool:
    return _lib.create_experiment(
        conn,
        experiment_info.encode("utf-8")
    )


def add_metric(
    conn: DemantoidConnPtr,
    run_number: int,
    pop_number: int,
    metric_name: str,
    metric_value: float
) -> bool:
    return _lib.add_metric(
        conn,
        run_number,
        pop_number,
        metric_name.encode("utf-8"),
        metric_value
    )
