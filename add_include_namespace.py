import re
import os
import glob
import sys
import platform

processlib_includes = set()
processlib_dirs = ['core',
                   'tasks',
               ]

def init() :
    base_script_path,_ = os.path.split(os.path.realpath(__file__))

    for processlib_include_path in processlib_dirs:
        includes = glob.glob(os.path.join(base_script_path,processlib_include_path,
                                          'include','processlib','*.h'))
        processlib_includes.update([os.path.split(x)[1] for x in includes])

def sed(file_list) :
    match_include = re.compile('^\s*#include\s+["<](.+?)[">]\s*$')
    for file in file_list:
        with open(file,'r+') as f:
            w_buffer = ''
            modify = False
            for line in f:
                match_group = match_include.match(line)
                if match_group and match_group.group(1) in processlib_includes:
                    w_buffer += '#include "processlib/%s"\n' % match_group.group(1)
                    modify = True
                else:
                    w_buffer += line
            if modify:
                f.seek(0,0)
                f.write(w_buffer)


if __name__ == '__main__':
    
    if platform.system() == 'Windows':
        file_list = []
        for arg in sys.argv[1:]:
            file_list.extend(glob.glob(arg))
    else:
        file_list = sys.argv[1:]

    init()
    sed(file_list)
