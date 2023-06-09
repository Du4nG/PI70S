
import re
from shutil import copy
from abc import ABC, abstractmethod


SCHEDULER_PATH = '/_targets/default/tpt_scheduler.c'
UNTESTED_CODE_PATH = '/testdata/FUSION_Platform/ctcReport/indexU.html'

class IPath(ABC):
    @abstractmethod
    def set_path(self, path):
        ...

class SEDGe(IPath):
    def __init__(self,sedge_path, fc_path):
        pass
    def find_missing_inits(self) -> list:
        pass
    def add_missing_inits(self) -> None:
        pass

class FC(IPath):
    def __init__(self,sedge_path, fc_path):
        super().__init__(sedge_path, fc_path)

    def copy(self) -> None:
        pass