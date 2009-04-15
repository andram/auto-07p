#! /usr/bin/env python
try:
    from ConfigParser import ConfigParser
except ImportError: # Python 3
    from configparser import ConfigParser
import os
import array
N = array

# This file contains code from the Python distribution.  As
# per the Python License we include the following:

##  -----------------------------------------------------------------------
##  Copyright 1991-1995 by Stichting Mathematisch Centrum, Amsterdam,
##  The Netherlands.

##                          All Rights Reserved

##  Permission to use, copy, modify, and distribute this software and its
##  documentation for any purpose and without fee is hereby granted,
##  provided that the above copyright notice appear in all copies and that
##  both that copyright notice and this permission notice appear in
##  supporting documentation, and that the names of Stichting Mathematisch
##  Centrum or CWI or Corporation for National Research Initiatives or
##  CNRI not be used in advertising or publicity pertaining to
##  distribution of the software without specific, written prior
##  permission.

##  While CWI is the initial source for this software, a modified version
##  is made available by the Corporation for National Research Initiatives
##  (CNRI) at the Internet address ftp://ftp.python.org.

##  STICHTING MATHEMATISCH CENTRUM AND CNRI DISCLAIM ALL WARRANTIES WITH
##  REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
##  MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH
##  CENTRUM OR CNRI BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
##  DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
##  PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
##  TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
##  PERFORMANCE OF THIS SOFTWARE.
##  -----------------------------------------------------------------------

# for bool, False, and True, taken from SCons:
# __COPYRIGHT__
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

# Portions of the following are derived from the compat.py file in
# Twisted, under the following copyright:
#
# Copyright (c) 2001-2004 Twisted Matrix Laboratories

def findBaseClass(inputClass,baseClass):
    try:
        for base in inputClass.__bases__:
            if base == baseClass:
                return 1
            elif findBaseClass(base,baseClass) == 1:
                return 1
        return 0
        # Sometimes inputClass isn't really a class, if so we just return false
    except AttributeError:
        return 0

def getAUTORC(section=None):
    parser = ConfigParser()
    if section is not None:
        parser.add_section(section)
    path = os.path.expandvars("$AUTO_DIR/.autorc")
    if os.path.exists(path):
        parser.read(path)
    path = os.path.expandvars("$HOME/.autorc")
    if os.path.exists(path):
        parser.read(path)
    if os.path.exists("./autorc"):
        parser.read("./autorc")
    elif os.path.exists("./.autorc"):
        parser.read("./.autorc")
    return parser

try:
    import __builtin__
except ImportError:
    import builtins as __builtin__ # Python 3

try:
    raw_input
except NameError: # Python 3
    __builtin__.raw_input = input

try:
    all
except NameError:
    # Pre-2.5 Python has no all() function.
    def all(iterable):
        """
        Returns True if all elements of the iterable are true.
        """
        for element in iterable:
             if not element:
                return False
        return True
    __builtin__.all = all
    all = all
        
try:
    any
except NameError:
    # Pre-2.5 Python has no any() function.
    def any(iterable):
        """
        Returns True if any element of the iterable is true.
        """
        for element in iterable:
            if element:
                return True
        return False
    __builtin__.any = any
    any = any
        

try:
    bool
except NameError:
    # Pre-2.2 Python has no bool() function.
    def bool(value):
        """Demote a value to 0 or 1, depending on its truth value.

        This is not to be confused with types.BooleanType, which is
        way too hard to duplicate in early Python versions to be
        worth the trouble.
        """
        return not not value
    __builtin__.bool = bool
    bool = bool

try:
    dict
except NameError:
    # Pre-2.2 Python has no dict() keyword.
    def dict(seq=[], **kwargs):
        """
        New dictionary initialization.
        """
        d = {}
        for k, v in seq:
            d[k] = v
        d.update(kwargs)
        return d
    __builtin__.dict = dict

try:
    False
except NameError:
    # Pre-2.2 Python has no False keyword.
    setattr(__builtin__, "False", not 1)
    # Assign to False in this module namespace so it shows up in pydoc output.
    globals()["False"] = False

try:
    True
except NameError:
    # Pre-2.2 Python has no True keyword.
    setattr(__builtin__, "True", not 0)
    # Assign to True in this module namespace so it shows up in pydoc output.
    globals()["True"] = False

# very basic numpy emulation:
def array(l, code=None):
    if isinstance(l, N.ArrayType):
        if code is None:
            code = l.typecode
        return N.array(code, l.tolist())
    if code is None:
        code = 'd'
    if isinstance(l, list):
        if isinstance(l[0], N.ArrayType):
            return l
        elif isinstance(l[0], list):
            a2 = []
            for a1 in l:
                a2.append(N.array(code, a1))
            return a2
        return N.array(code, l)
    try:
        l=list(l)
        return N.array(code, l) 
    except TypeError:
        return N.array(code, [l])

def rank(a):
    if isinstance(a, list):
        return 2
    return 1

def take(a, idx, axis=0):
    b=[]
    if axis == 1:
        for j in a:
            b.append(take(j, idx))
        return b
    try:
        for i in idx:
            b.append(a[i])
    except TypeError:
        raise IndexError
    return array(b)

def array2string(a,precision=0):
    return '[ '+"  ".join(map(str, a))+']'

def shape(a):
    if isinstance(a, list):
        return (len(a), len(a[0]))
    else:
        return (len(a),)
        
def zeros(dim,code):
    if len(dim) == 1:
        return N.array(code,dim[0]*[0.0])
    a = []
    for i in range(dim[0]):
        a.append(N.array(code,dim[1]*[0.0]))
    return a

def less(a, val):
    cond = []
    for v in a:
        cond.append(v < val)
    return cond

def ravel(a):
    return N.array('d',a)

ArrayType = N.ArrayType

def test():
    a=array([1,2,3])
    b=array(a)
    b[0] = 4
    print(a,b)

if __name__ == "__main__":
    test()
