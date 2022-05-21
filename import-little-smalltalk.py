#!/usr/bin/env python3

from xmltodict import parse, ParsingInterrupted
import collections
import unittest
import sys
import json
import pprint

try:
    from io import BytesIO as StringIO
except ImportError:
    from xmltodict import StringIO

from xml.parsers.expat import ParserCreate
from xml.parsers import expat


def test_custom_attrib():
    print(parse('<a href="xyz"/>', 
                            attr_prefix='_')) 


classes = {}


with open("imageSource", "r") as read_file:
    comment = 0
    line = read_file.readline()
    while line:
        if line.startswith("COMMENT"):
            comment += 1
        else:
            parts = line.split()
            if len(parts) > 0:
                if parts[0] == "CLASS":
                    classes[parts[1]] = {   'super': parts[2], 
                                            'methods': [],
                                            'args': parts[3:]}                
                    classes["Meta" + parts[1]] = {   'super': "Meta" + parts[2], 
                                            'methods': [],
                                            'args': []}                
                elif parts[0] == "RAWCLASS":
                    classes[parts[1]] = {   'super': parts[2], 
                                            'methods': [],
                                            'args': parts[3:]}                
                elif parts[0] == "METHOD":
                    o = classes[parts[1]]
                    meth = []
                    line = read_file.readline().rstrip()
                    while line and line != '!':
                        meth.append(line)
                        line = read_file.readline().rstrip()
                    o['methods'].append(meth[0] + ' [\n' + '\n'.join(meth[1:]) + ']')
        line = read_file.readline()

#pp = pprint.PrettyPrinter()
#pp.pprint(classes)

for cname in classes:
    c = classes[cname]
    print(cname + ' [\n')
    for m in c['methods']:
        print(m)
    print(']\n')
