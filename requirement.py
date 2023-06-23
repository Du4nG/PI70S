import pandas as pd
import re
from os.path import basename

DOCMISC_PATH = 'C:\\Users\\HDO5HC\\Desktop\\tdev_cswspsa.xls'

class Type:
    SIMPLE = 'swspsa'
    COMPLEX = 'cswspsa'

class Base:
    def __init__(self, path:str):
        self._path = path
        self._fc_name = basename(self._path)

    def detect_fc_type(self):
        type = re.search(r'_(\w+)', self._fc_name).group(1)
        return type
        
    def find_urt(self):
        df = pd.read_excel(self._path)
        filtered = df[df.iloc[:, 0] != 'Remove']
        COL_B = filtered.iloc[1:, 1].tolist()
        COL_C = filtered.iloc[1:, 2].tolist()
        COL_Q = filtered.iloc[1:, 16].tolist()
        COL_AF = filtered.iloc[1:, 31].tolist()

        columns: dict = {
            'B': COL_B,  
            'C': COL_C, 
            'Q': COL_Q
        }

        fc_type = self.detect_fc_type()
        match fc_type:
            case Type.SIMPLE:
                columns['simple'] = 'simple mapping'
            case Type.COMPLEX:
                columns['AF'] = COL_AF

        output = pd.DataFrame(columns).sort_values(by='B')
        output.drop_duplicates(inplace=True)
        output.insert(0, 'index', range(1, len(output) + 1))
        output.to_excel('D:\\Code\\PI70S\\requirement.xlsx', header=None, index=None)
    
obj = Base(DOCMISC_PATH)
obj.find_urt()