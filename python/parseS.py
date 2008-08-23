#! /usr/bin/env python
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
import UserList
import AUTOExceptions
import types
import copy
import parseB

# End of data exception definition
class PrematureEndofData(Exception):
    pass

class IncorrectHeaderLength(Exception):
    pass

# This is the number of parameters.  Should be read from auto.h...
# This is not required anymore in AUTO97, since the last entry in
# the header line is this number
NPAR = 20

# The parseS class parses an AUTO fort.8 file
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
# Once the data is read in the class provides a list all the points
# in the fort.8 file.

class parseS(UserList.UserList):
    def __init__(self,filename=None):
        if type(filename) == types.StringType:
            UserList.UserList.__init__(self)
            self.readFilename(filename)
        else:
            UserList.UserList.__init__(self,filename)

    def __str__(self):
        rep = ""
        rep = rep + "Number of solutions :" + str(len(self)) + "\n"
        labels = self.getLabels()
        rep = rep + "Labels: "
        for label in labels:
            rep = rep + str(label) + " "
        rep = rep + "\n"
        return rep

    def __call__(self,label):
        return self.getLabel(label)

    # This function needs a little explanation
    # It trys to read a new point from the input file, and if
    # it cannot (because the file ends prematurely) is sets the
    # file pointer back to the way it was when it started and returns
    # This will be used to check a file to see if it has a new point
    # and it will ignore a partially created solution.
    # Basically it for an VBM kind of program.
    def tryNextPointRead(self,inputfile):
	current_position = inputfile.tell()
	try:
	    self.data.append(AUTOSolution(inputfile))
	except PrematureEndofData:
	    inputfile.seek(current_position)

    def read(self,inputfile):
        # We now go through the file and read the solutions.
        prev = None
        while inputfile.read(1) != "":
            solution = AUTOSolution(inputfile,prev)
            self.data.append(solution)
            prev = solution

    def write(self,output):
        for x in self.data:
            x.write(output)
        output.flush()

    def readFilename(self,filename):
	inputfile = open(filename,"rb")
        self.read(inputfile)
        inputfile.close()

    def writeFilename(self,filename):
	output = open(filename,"w")
        self.write(output)
	output.close()

    # Removes solutions with the given labels or type name
    def deleteLabel(self,label=None,keep=0):
        if label == None:
            label=['BP','LP','HB','PD','TR','EP','MX']
        if type(label) != types.ListType:
            label = [label]
        for x in self.data[:]:
            if ((not keep and (x["Label"] in label or x["Type name"] in label)
                 or (keep and not x["Label"] in label and 
                              not x["Type name"] in label))):
                self.data.remove(x)
            
    # Relabels the first solution with the given label
    def relabel(self,old_label,new_label):
        if type(old_label)  == types.IntType:
            for i in range(len(self)):
                if self.data[i]["Label"] == old_label:
                    self.data[i]["Label"] = new_label
        else:
            for j in range(len(old_label)):
                for i in range(len(self)):
                    if self.data[i]["Label"] == old_label[j]:
                        self.data[i]["Label"] = new_label[j]

    # Make all labels in the file unique and sequential
    def uniquelyLabel(self):
        for i in range(len(self)):
            self.data[i]["Label"] = i+1

    # Given a label, return the correct solution
    def getLabel(self,label):
        if type(label) == types.IntType:
            for d in self.data:
                if d["Label"] == label:
                    return d
            return
        if type(label) != types.ListType:
            label = [label]        
        data = []
        for d in self.data:
            if d["Label"] in label or d["Type name"] in label:
                data.append(d)
        return self.__class__(data)

    def getIndex(self,index):
        return self.data[index]

    # Return a list of all the labels in the file.
    def getLabels(self):
        labels = []
        for x in self.data:
            labels.append(x["Label"])
        return labels

# The AUTOsolution class parses an AUTO fort.8 file
# THESE EXPECT THE FILE TO HAVE VERY SPECIFIC FORMAT!
# it provides 4 methods:
# read and write take as an argument either and input or output
#    stream (basically any object with has the method "readline"
#    for reading and "write" for writing)
#    
# readFilename and writeFilename take as an arguement a filename
#    in which to read/write the parameters (basically it opens the
#    file and then calles "read" or "write"
#    
# Used by itself is only reads in ONE solution from the file
# for example readFilename will only read the first solution
# Commonly it will be used in a container class only using the
# read and write methods and letting the outside class take care
# of opening the file.

class AUTOSolution(UserDict.UserDict):
    def __init__(self,input=None,offset=None):
	UserDict.UserDict.__init__(self)
        self.__start_of_header = None
        self.__start_of_data   = None
        self.__end              = None
        self.__fullyParsed     = 0
	if input:
	    self.read(input,offset)

    def __str__(self):
        if not(self.__fullyParsed):
            self.__readAll()
        keys = self.keys()
        keys.sort()
        rep=""
        for key in keys:
            if key != "data":
                rep=rep+str(key)+": "+str(self[key])+"\n"
        return rep
    def __getitem__(self,key):
        big_data_keys = ["data","Free Parameters","Parameter NULL vector","Parameters","parameters","p"]
        if type(key) == types.IntType:
            if not(self.__fullyParsed):
                self.__readAll()
            return self["data"][key]
        else:
            if key in big_data_keys and not(self.__fullyParsed):
                self.__readAll()
            return UserDict.UserDict.__getitem__(self,key)
    
    def type(self):
	return parseB.type_translation(self["Type number"])["long name"]

    def readAllFilename(self,filename):
	inputfile = open(filename,"rb")
	self.readAll(inputfile)
	inputfile.close()

    def readFilename(self,filename):
	inputfile = open(filename,"rb")
	self.read(inputfile)
	inputfile.close()

    def writeFilename(self,filename):
	output = open(filename,"w")
	self.write(output)
        output.flush()
	output.close()

    def writeRawFilename(self,filename):
	output = open(filename,"w")
	self.writeRaw(output)
        output.flush()
	output.close()
        
    def toArray(self):
        array = []
        for vector in self["data"]:
            array.append([])
            array[-1].append(vector["t"])
            for point in vector["u"]:
                array[-1].append(point)
        return array

    def writeRaw(self,output):
        for vector in self["data"]:
            output.write(str(vector["t"])+" ")
            for point in vector["u"]:
                output.write(str(point)+" ")
            output.write("\n")
            
    def read(self,input,prev=None):
        if prev is None:
            self.__start_of_header = 0
        else:
            self.__start_of_header = prev._getEnd()
        self.__readHeader(input)
        end = None
        if not prev is None and self.__equalSize(prev):
            # guess the end from the previous solution
            end = input.tell() + prev._getEnd() - prev._getStartOfData()
            # See if the guess for the solution end is correct
            input.seek(end,0)
            data = input.readline()
            data = string.split(data)
            # This is where we detect the end of the file
            if len(data) == 0:
                data = input.read(1)
            if len(data) != 0:
                try:
                    # Check length of line...
                    if len(data) != 12:
                        raise IncorrectHeaderLength
                    # and the fact they are all integers
                    map(int,data)
                    # If it passes both these tests we say it is a header line
                    # and we can read quickly
                except:
                    # otherwise the guessed end is not valid
                    end = None
        input.seek(self.__start_of_data)
        if end is None:
            # We skip the correct number of lines in the entry to
            # determine its end.
            slist = []
            for i in range(self.__numLinesPerEntry):
                slist.append(input.readline())
            self.__data = string.join(slist,"")
            end = input.tell()
        else:
            self.__data = input.read(end - self.__start_of_data)
        self.__end = end
    
    def readAll(self,input,start=None,end=None):
        self.read(input,start,end)
        self.__readAll()

    def _setEnd(self,end):
        self.__end = end

    def _getEnd(self):
        return self.__end

    def _getStartOfData(self):
        return self.__start_of_data

    def __equalSize(self,other):
        return (
            self.__numEntriesPerBlock == other.__numEntriesPerBlock and
            self.__numFreeParameters == other.__numFreeParameters and
            self.__numChangingParameters == other.__numChangingParameters and
            self.__numSValues == other.__numSValues)

    def __readHeader(self,inputfile):
        inputfile.seek(self.__start_of_header)
	line = inputfile.readline()
	if not line: raise PrematureEndofData
	data = string.split(line)
        try:
            self["Branch number"] = int(data[0])
            self["Point number"] = int(data[1])
            self["Type number"] = int(data[2])
            self["Type name"] = parseB.type_translation(self["Type number"])["short name"]
            self["Label"] = int(data[3])
            self.__numChangingParameters = int(data[4])
            self["ISW"] = int(data[5])
            self.__numSValues = int(data[6])
            self.__numEntriesPerBlock = int(data[7])
            self.__numLinesPerEntry = int(data[8])
            self["NTST"] = int(data[9])
            self["NCOL"] = int(data[10])
            if len(data)==12:
                # This is the case for AUTO97 and beyond
                self.__numFreeParameters = int(data[11])
            else:
                # This is the case for AUTO94 and before
                self.__numFreeParameters = NPAR
        except IndexError:
            raise PrematureEndofData
        self.__start_of_data = inputfile.tell()
        
    def __readAll(self):
        self.__fullyParsed = 1
        data = string.split(self.__data)
        try:
            fdata = map(float, data)
        except:
            fdata = map(parseB.AUTOatof, data)
        n = self.__numEntriesPerBlock
        total = n * self.__numSValues + self.__numFreeParameters
        if self["NTST"] != 0:
            total = (total + 2 * self.__numChangingParameters +
                     (n-1) * self.__numSValues)
        if total != len(fdata):
            raise PrematureEndofData
        solution = []
        j = 0
        for i in range(self.__numSValues):
            solution.append({"t": fdata[j],"u": fdata[j+1:j+n]})
            j = j + n
	# I am using the value of NTST to test to see if it is an algebraic or
	# ODE problem.
	if self["NTST"] != 0:
            nfpr = self.__numChangingParameters
            self["Free Parameters"] = map(int,fdata[j:j+nfpr])
            j = j + nfpr
            self["Parameter NULL vector"] = fdata[j:j+nfpr]
            j = j + nfpr
            n = n - 1
            for i in range(self.__numSValues):
                solution[i]["u dot"] = fdata[j:j+n]
                j = j + n

        self["data"] = solution
        self["Parameters"] = fdata[j:j+self.__numFreeParameters]
        self["parameters"] = self["Parameters"]
        self["p"] = self["Parameters"]

    # just a simple way to get PAR(x) -- could be extended later to not
    # be read-only
    def PAR(self,index):
        return self["Parameters"][index-1]

    def write(self,output):
        nfpr = self.__numChangingParameters
        ndim = self.__numEntriesPerBlock-1
        npar = self.__numFreeParameters
        ntpl = self.__numSValues

        if self.__fullyParsed:
            ndim = len(self[0]['u'])
            npar = len(self["Parameters"])
            ntpl = len(self["data"])

        if self["NTST"] != 0:
            if self.__fullyParsed:
                nfpr = len(self["Free Parameters"])
            nrd = 2 + ndim/7 + (ndim-1)/7
            nrowpr = (nrd * (self["NCOL"] * self["NTST"] + 1) +
                      (nfpr-1)/7+1 + (npar-1)/7+1 + (nfpr-1)/20+1)
        else:
            nrowpr = ndim/7+1 + (npar-1)/7+1
            
	line = "%6d%6d%6d%6d%6d%6d%8d%6d%8d%5d%5d%5d" % (self["Branch number"],
                                                         self["Point number"],
                                                         self["Type number"],
                                                         self["Label"],
                                                         nfpr,
                                                         self["ISW"],
                                                         ntpl,
                                                         ndim+1,
                                                         nrowpr,
                                                         self["NTST"],
                                                         self["NCOL"],
                                                         npar
                                                         )
	output.write(line+"\n")
        # If the file isn't already parsed, and we happen to know the position of
        # the end of the solution we can just copy from the raw data from the
        # input file into the output file.
        if not(self.__fullyParsed) and not(self.__end is None):
            output.write(self.__data)
            output.flush()
        # Otherwise we do a normal write.  NOTE: if the solution isn't already
        # parsed it will get parsed here.
        else:
            for point in self["data"]:
                num = "%19.10E" % (point["t"])
                line = "    "+num
                j = 1
                for n in point["u"]:
                    num = "%19.10E" % (n)
                    line = line + num
                    j = j + 1
                    if j%7==0:
                        output.write(line+"\n")
                        line = "    "
                if j%7!=0:
                    output.write(line+"\n")
            # I am using the value of NTST to test to see if it is an algebraic or
            # ODE problem.
            if self["NTST"] != 0:
                j = 0
                for parameter in self["Free Parameters"]:
                    output.write("%5d" % (parameter))
                    j = j + 1
                    if j%20==0:
                        output.write("\n")
                if j%20!=0:
                    output.write("\n")

                line = "    "
                i = 0
                for vi in self["Parameter NULL vector"]:
                    num = "%19.10E" % (vi)
                    if i != 0 and i%7==0:
                        line = line + "\n    "
                    line = line + num
                    i = i + 1
                output.write(line+"\n")

                for point in self["data"]:
                    line = "    "
                    j = 0
                    for n in point["u dot"]:
                        num = "%19.10E" % (n)
                        line = line + num
                        j = j + 1
                        if j%7==0:
                            output.write(line+"\n")
                            line = "    "
                    if j%7!=0:
                        output.write(line+"\n")

            line = "    "
            j = 0
            for parameter in self["Parameters"]:
                num = "%19.10E" % (parameter)
                line = line + num 
                j = j + 1
                if j%7==0:
                    output.write(line+"\n")
                    line = "    "
            if j%7!=0:
                output.write(line+"\n")
            output.flush()

def pointtest(a,b):
    keys = ['Type number', 'Type name', 'Parameter NULL vector',
            'Free Parameters', 'Branch number',
            'data', 'NCOL', 'Label', 'ISW', 'NTST',
            'Point number', 'Parameters']

    # make sure the solutions are fully parsed...
    scratch=a['Parameters']
    scratch=b['Parameters']
    for key in keys:
        if not(a.has_key(key)):
            raise AUTOExceptions.AUTORegressionError("No %s label"%(key,))
    if not(len(a["data"]) == len(b["data"])):
        raise AUTOExceptions.AUTORegressionError("Data sections have different lengths")


def test():
    print "Testing reading from a filename"
    foo = parseS()
    foo.readFilename("test_data/fort.8")    
    if len(foo) != 5:
        raise AUTOExceptions.AUTORegressionError("File length incorrect")
    pointtest(foo.getIndex(0),foo.getIndex(3))

    print "Testing reading from a stream"
    foo = parseS()
    fp = open("test_data/fort.8","rb")
    foo.read(fp)    
    if len(foo) != 5:
        raise AUTOExceptions.AUTORegressionError("File length incorrect")
    pointtest(foo.getIndex(0),foo.getIndex(3))

    
    
    print "parseS passed all tests"

if __name__ == '__main__' :
    test()









