#!/usr/bin/env python
import sys
import os
import hashlib
import timeit

arguments    = sys.argv[1:]

if len(arguments) == 0:
    print 'Usage: ./test.py Please enter path to the directory'
    exit(1)

count      = 0
array      = []
tree_hash  = hashlib.md5()
#time_start = timeit.default_timer()

for arg in sys.argv:
    for rootdir, dirs, files in os.walk( arg ):
        for file in files:   
            if file.endswith( ('.c','.cpp','.h','.hpp','.m','.mm','.ui','.qrc','.a','.so','.qrc','.lib')  ): 
                array.append( os.path.relpath(( rootdir + file ), ( arg + '/..' ) )  ) 
#                print os.path.relpath(( rootdir + file ), ( arg + '/..' ) ) 
#                count= count + 1  

tree_hash.update( repr(array).encode('utf-8') )
#time_stop = timeit.default_timer()

print tree_hash.hexdigest()
#print time_stop - time_start 
#print count