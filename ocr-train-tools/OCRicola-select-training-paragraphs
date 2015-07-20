#! /usr/bin/env python
# -*- coding: utf-8 -*-

# utf-8 input magic.
import sys
reload(sys)
sys.setdefaultencoding("utf-8")

from sys         import stdin, stderr
from collections import defaultdict 
from sets        import Set

paragraphs = stdin.read().split('\n')

char_dict = defaultdict(lambda : [])
para_set  = Set()

for paragraph in paragraphs:
    for char in unicode(paragraph):
        if len(char_dict[char]) < 10:
            char_dict[char].append(paragraph)
            para_set.add(paragraph)

for paragraph in para_set:
    print paragraph

stderr.write("Symbols with too few Training instances:\n")

for char, para_list in char_dict.iteritems():    
    if len(para_list) < 5:
        stderr.write("WARNING: char '%s': %u instances.\n" % (char.encode('utf-8'), 
                                                              len(para_list)))

