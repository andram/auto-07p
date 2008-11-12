#!/usr/bin/env python
#    Visualization for Bifurcation Manifolds
#    Copyright (C) 1997 Randy Paffenroth and John Maddocks
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU  General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.
#
#    You should have received a copy of the GNU Library General Public
#    License along with this library; if not, write to the Free
#    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
#    MA 02111-1307, USA

import string
import os
import sys
import UserDict
import cStringIO
import AUTOExceptions

line1_comment="NUNSTAB,NSTAB,IEQUIB,ITWIST,ISTART"
line2_comment="NREV,/,IREV(I),I=1,NDIM)"
line3_comment="NFIXED,(/,I,IFIXED(I)),I=1,NFIXED)"
line4_comment="NPSI,(/,I,IPSI(I)),I=1,NPSI)"

# The parseH class parses an AUTO parameter file
# THESE EXPECT THE FILE TO HAVE VERY SPECIFIC FORMAT!
# it provides 4 methods:
# read and write take as an arguement either and input or output
#    stream (basically any object with has the method "readline"
#    for reading and "write" for writing)
#    
# readFilename and writeFilename take as an arguement a filename
#    in which to read/write the parameters (basically it opens the
#    file and then calles "read" or "write"
#    
# Once the data is read in the class provides a dictionary
# interface for manipulating the file.



class parseH(UserDict.UserDict):

    def __init__(self,filename=None):
        if filename is not None and type(filename) != type(""):
            UserDict.UserDict.__init__(self,filename)
            return
	UserDict.UserDict.__init__(self)
        for key in ['NUNSTAB', 'NSTAB', 'IEQUIB', 'ITWIST', 'ISTART',
                    'NREV', 'NFIXED', 'NPSI',
                    'IREV', 'IFIXED', 'IPSI']:
            self[key] = None
	if filename:
	    self.readFilename(filename)
#        self.dataString=""
        
    def __str__(self):
        string = cStringIO.StringIO()
        self.write(string)
        return string.getvalue()
        
    def readFilename(self,filename):
	inputfile = open(filename,"r")
	self.read(inputfile)
	inputfile.close()

    def writeFilename(self,filename):
	output = open(filename,"w")
	self.write(output)
	output.close()

    def read(self,inputfile):
#        self.dataString = inputfile.read()
	line = inputfile.readline()
	data = string.split(line)
	self["NUNSTAB"] = int(data[0])
	self["NSTAB"] = int(data[1])
	self["IEQUIB"] = int(data[2])
	self["ITWIST"] = int(data[3])
	self["ISTART"] = int(data[4])

	line = inputfile.readline()
	data = string.split(line)
        nrev = int(data[0])
        data = []
	if nrev > 0:
	    line = inputfile.readline()
	    data = string.split(line)
        self["IREV"] = map(int,data)
	self["NREV"] = nrev

	line = inputfile.readline()
	data = string.split(line)
        nfixed = int(data[0])
        data = []
	if nfixed > 0:
	    line = inputfile.readline()
	    data = string.split(line)
	self["IFIXED"] = map(int,data[:nfixed])
	self["NFIXED"] = nfixed

	line = inputfile.readline()
	data = string.split(line)
        npsi = int(data[0])
        data = []
	if npsi > 0:
	    line = inputfile.readline()
	    data = string.split(line)
        self["IPSI"] = map(int,data[:npsi])
	self["NPSI"] = npsi


    def write(self,output):
#        output.write(self.dataString)
	output.write(str(self["NUNSTAB"])+" "+str(self["NSTAB"])+" ")
	output.write(str(self["IEQUIB"]) +" "+str(self["ITWIST"])+" ")
	output.write(str(self["ISTART"]) +" ")
	output.write("          "+line1_comment+"\n")

        nrev = 0
        if len(self["IREV"]) > 0:
            nrev = 1
	output.write(str(nrev)+" ")
	output.write("          "+line2_comment+"\n")
	for d in self["IREV"]:
	    output.write(str(d)+" ")
	if nrev > 0:
	    output.write("\n")

	output.write(str(len(self["IFIXED"]))+" ")
	output.write("          "+line3_comment+"\n")
	for d in self["IFIXED"]:
	    output.write(str(d)+" ")
	if len(self["IFIXED"]) > 0:
	    output.write("\n")

	output.write(str(len(self["IPSI"]))+" ")
	output.write("          "+line4_comment+"\n")
	for d in self["IPSI"]:
	    output.write(str(d)+" ")
	if len(self["IPSI"]) > 0:
	    output.write("\n")
        
        output.flush()

def pointtest(a):
    keys = ['NUNSTAB', 'NSTAB', 'IEQUIB', 'ITWIST', 'ISTART',
            'NREV', 'IREV', 'NFIXED', 'IFIXED', 'NPSI', 'IPSI']		
    for key in keys:
        if not(a.has_key(key)):
            raise AUTOExceptions.AUTORegressionError("No %s label"%(key,))

def test():
    print "Testing reading from a filename"
    foo = parseH()
    foo.readFilename("test_data/h.cir")    
    pointtest(foo)

    print "Testing reading from a stream"
    foo = parseH()
    fp = open("test_data/h.cir","r")
    foo.read(fp)    
    pointtest(foo)

    print "parseH passed all tests"

if __name__ == '__main__' :
    test()








