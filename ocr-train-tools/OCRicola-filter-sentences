#! /usr/bin/env python

import sys
reload(sys)
sys.setdefaultencoding('utf-8')

from sys  import argv, stdin
from sets import Set

valid_chars = Set(map(lambda x: unicode(x), open(argv[1]).read().split('\n')))

data = stdin.read().split('\n')

for line in data:
    
    invalid_line = 0

    for char in unicode(line):
        if not char in valid_chars:
            invalid_line = 1
            break

    if not invalid_line:
        print line
