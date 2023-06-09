import pandas as pd
import re
from os.path import basename

DOCMISC_PATH = 'C:/Users/HDO5HC/Desktop/comdia_swspsa.xls'
SIMPLE = 'swspsa'
COMPLEX = 'cswspsa'

class Base:
    def __init__(self, path:str):
        self._path = path
        self._fc_name = basename(self._path)

    def detect_FC_type(self):
        type = re.search(r'_(\w+)', self._fc_name).group(1)
        return type
        
    def process(self):
        df = pd.read_excel(self._path)
        filtered = df[df.iloc[:, 0] != 'Remove']
        COL_B = filtered.iloc[2:, 1].tolist()
        COL_C = filtered.iloc[2:, 2].tolist()
        COL_Q = filtered.iloc[2:, 16].tolist()
        COL_AF = filtered.iloc[2:, 31].tolist()

        column_dic = {
            'B': COL_B,  
            'C': COL_C, 
            'Q': COL_Q
        }

        if self.detect_FC_type() == SIMPLE:
            column_dic['simple'] = 'simple mapping'
        else:
            column_dic['AF'] = COL_AF

        output = pd.DataFrame(column_dic).sort_values(by='B')
        output.drop_duplicates(inplace=True)
        output.insert(0, 'index', range(1, len(output) + 1))
        output.to_excel('requirement.xlsx', header=None, index=None)
    
obj = Base(DOCMISC_PATH)
obj.process()