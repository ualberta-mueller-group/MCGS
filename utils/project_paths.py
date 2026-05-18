"""
Utilities for finding various paths within the project, relative to this
file. Used by other scripts to simplify execution of scripts from a
CMake build.

`build_name` is the name of the build directory i.e. "build"

"""
import os
from pathlib import Path

def get_project_root():
    this_file = os.path.realpath(__file__)
    project_root = Path(this_file).parent.parent
    return project_root

def get_mcgs_path(build_name):
    assert type(build_name) is str
    project_root = get_project_root()
    mcgs_path = project_root / build_name / "MCGS"
    return mcgs_path
