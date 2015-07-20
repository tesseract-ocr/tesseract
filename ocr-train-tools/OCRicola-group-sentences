#! /usr/bin/env python

PARAGRAPH_LEN = 5

from sys import stdin, stdout

sentences = stdin.read().split('\n')

for counter in xrange(len(sentences)):

    if counter > 0:
        if counter % PARAGRAPH_LEN == 0:
            print
        else: 
            stdout.write(' ')

    stdout.write(sentences[counter])

