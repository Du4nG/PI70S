
import re

SCHEDULER_PATH = '/_targets/default/tpt_scheduler.c'

class Path:
    def __init__(self, sedge_path, fc_path):
        self._sedge_path = sedge_path
        self._fc_path = fc_path

    @property
    def sedge_path(self):
        return self._sedge_path

    @sedge_path.setter
    def sedge_path(self, new_path):
        self._sedge_path = new_path


class SEDGe(Path):
    def __init__(self,sedge_path, fc_path):
        super().__init__(sedge_path, fc_path)
        self._indexU_path = self._sedge_path + '\\testdata\FUSION_Platform\ctcReport\indexU.html'

    def find_missing_inits(self) -> list:
        with open(self._indexU_path, 'r') as file:
            content = file.read()

        pattern = r'FUNCTION\s+(\w+)\('
        missing_inits = re.findall(pattern, content)

        print('\n',missing_inits)
        return missing_inits

    def add_missing_inits(self) -> None:
        missing_inits = self.find_missing_inits()
        PATH = self._sedge_path + SCHEDULER_PATH

        with open(PATH, 'r') as file:
            lines = file.readlines()

        with open(PATH, 'w') as file:
            for line in lines:
                if '//Added' in line:
                    continue
                if line.strip() == 'SEDGe_Post_Proc_10ms();':
                    for init in missing_inits:
                        file.write('\t'+ init + '(); //Added\n')
                file.write(line)

class FC(Path):
    def __init__(self,sedge_path, fc_path):
        super().__init__(sedge_path, fc_path)

    def copy(self) -> None:
        pass