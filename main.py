import os
from tools import *
from os.path import isdir

os.system('cls')

EXIT = 0
SEDGE_PATH = input('Enter SEDGe path: ')
assert isdir(SEDGE_PATH), 'Invalid path'

path = SEDGe(SEDGE_PATH)
tool_list = {
    1 : path.find_missing_inits,
    2 : path.add_missing_inits,
}


# if __name__ == 'main':
while 1:
    os.system('cls')
    for numb, tool in tool_list.items():
        print(f'{numb} : {tool.__name__}')

    selection = int(input('Choose method: '))
    if selection == EXIT:
        break

    selected_method = tool_list.get(selection)

    selected_method()
    input("Press any key to continue...")