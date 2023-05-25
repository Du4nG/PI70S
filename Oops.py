import re
from os.path import isdir

class SEDGe:
    def __init__(self, sedge_path):
        self._sedge_path = sedge_path

    @property
    def sedge_path(self):
        return self._sedge_path

    @sedge_path.setter
    def sedge_path(self, new_path):
        assert(isdir(new_path)), 'Invalid path'
        self._sedge_path = new_path


class Validation(SEDGe):
    def __init__(self, parent=None):
        super().__init__(parent)
        self._indexU_path = self.sedge_path + '\\testdata\FUSION_Platform\ctcReport\indexU.html'

    def find_missing_inits(self) -> list:
        with open(self._indexU_path, 'r') as file:
            content = file.read()
        pattern = r'FUNCTION\s+(\w+)\('
        missing_inits = re.findall(pattern, content)
        return missing_inits

    def add_missing_inits(self) -> None:
        missing_inits = self.find_missing_inits()
        with open('PI70S/test.c', 'r') as file:
            lines = file.readlines()

        with open('PI70S/test.c', 'w') as file:
            for line in lines:
                if '//Added' in line:
                    continue
                if line.strip() == 'SEDGe_Post_Proc_10ms();':
                    for init in missing_inits:
                        file.write('\t'+ init + '(); //Added\n')
                file.write(line)


obj = Validation('C:\\Users\\HDO5HC\\SEDGe\\2019.1.1\\VehC_SwSPSA_38_53_0_rev1')
print(obj.sedge_path)
print(obj.add_missing_inits())


# Multicondition coverage 	    :	100%
# Statement coverage 		    :	100%