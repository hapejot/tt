#!/usr/bin/env python3

from xmltodict import parse, ParsingInterrupted
import collections
import unittest
import sys
import json

try:
    from io import BytesIO as StringIO
except ImportError:
    from xmltodict import StringIO

from xml.parsers.expat import ParserCreate
from xml.parsers import expat


def test_custom_attrib():
    print(parse('<a href="xyz"/>', 
                            attr_prefix='_')) 


with open(sys.argv[1], "r") as read_file:
    x = read_file.read()
d = parse(x, attr_prefix = '' )
json.dump(d, sys.stdout)
