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
import Points
import runAUTO

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
        self.name = ''
        if type(filename) == types.StringType:
            UserList.UserList.__init__(self)
            self.readFilename(filename)
        else:
            UserList.UserList.__init__(self,filename)
            if len(self.data) > 0:
                self.indepvarname = self.data[0].indepvarname
                self.coordnames = self.data[0].coordnames

    def __str__(self):
        rep = ""
        rep = rep + "Number of solutions :" + str(len(self)) + "\n"
        labels = self.getLabels()
        rep = rep + "Labels: "
        for label in labels:
            rep = rep + str(label) + " "
        rep = rep + "\n"
        return rep

    def __call__(self,label=None):
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
	    self.data.append(AUTOSolution(inputfile,name=self.name))
	except PrematureEndofData:
	    inputfile.seek(current_position)

    def read(self,inputfile):
        # We now go through the file and read the solutions.
        prev = None
        while inputfile.read(1) != "":
            solution = AUTOSolution(inputfile,prev,name=self.name)
            self.data.append(solution)
            prev = solution
        if len(self.data) > 0:
            self.indepvarname = self.data[0].indepvarname
            self.coordnames = self.data[0].coordnames

    def write(self,output):
        for x in self.data:
            x.write(output)
        output.flush()

    def readFilename(self,filename):
        try:
            inputfile = open(filename,"rb")
        except IOError:
            import gzip
            inputfile = gzip.open(filename+".gz","rb")
        self.name = filename
        self.read(inputfile)
        inputfile.close()

    def writeFilename(self,filename,append=False):
        if append:
            output = open(filename,"ab")
        else:
            output = open(filename,"wb")
        self.write(output)
	output.close()

    # Removes solutions with the given labels or type name
    def deleteLabel(self,label=None,keep=0):
        if label == None:
            label=['BP','LP','HB','PD','TR','EP','MX']
        if type(label) != types.ListType:
            label = [label]
        indices = []
        for i in range(len(self.data)):
            x = self.data[i]
            if ((not keep and (x["Label"] in label or x["Type name"] in label)
                 or (keep and not x["Label"] in label and 
                              not x["Type name"] in label))):
                indices.append(i)
        indices.reverse()
        for i in indices:
            del self.data[i]
            
    # Relabels the first solution with the given label
    def relabel(self,old_label,new_label):
        if type(old_label) == types.IntType:
            old_label = [old_label]
            new_label = [new_label]
        for j in range(len(old_label)):
            for d in self.data:
                if d["Label"] == old_label[j]:
                    d["Label"] = new_label[j]

    # Make all labels in the file unique and sequential
    def uniquelyLabel(self):
        i = 1
        for d in self.data:
            d["Label"] = i
            i = i + 1

    # Given a label, return the correct solution
    def getLabel(self,label):
        if label is None:
            return self
        if type(label) == types.IntType:
            for d in self.data:
                if d["Label"] == label:
                    return d
            return
        if type(label) == types.StringType:
            for d in self.data:
                if d["Type name"] == label:
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

# an old-style point and point keys within an AUTOSolution
class SLPointKey(UserList.UserList):
    def __init__(self, solution=None, index=None, coords=None):
        if coords=="u dot":
            self.solution = solution["udotps"]
        else:
            self.solution = solution
        self.index = index
    def __getattr__(self, attr):
        if attr == 'data':
            data = []
            for i in range(len(self.solution.coordarray)):
                data.append(self.solution.coordarray[i][self.index])
            return data
        raise AttributeError
    def __setitem__(self, i, item):
        self.solution.coordarray[i][self.index] = item
    def __str__(self):
        return str(self.data)
    def append(self, item):
        self.enlarge(1)
        self.solution.coordarray[-1][self.index] = item
        self.data.append(item)
    def extend(self, other):
        self.enlarge(len(other))
        for i in range(len(other)):
            self.solution.coordarray[-len(other)+i][self.index] = other[i]
    def enlarge(self, ext):
        # enlarges the dimension of coordarray and coordnames
        s = self.solution
        if s._dims is None:
            s._dims = [s.dimension]*len(s)
            c = []
            s0 = s.coordnames[0]
            for i in range(ext):
                c.append(s0[0:string.find(s0,'(')+1]+str(s.dimension+i+1)+')')
            s.extend(c)
        s._dims[self.index] = s.dimension
        if min(s._dims) == max(s._dims):
            s._dims = None

class SLPoint(Points.Point):
    def __init__(self, p, solution=None, index=None):
        Points.Point.__init__(self, p)
        self.index = index
        self.solution = solution
        
    def has_key(self, key):
        return key in ["u", "u dot", "t"] or Points.Point.has_key(self,key)

    def __getitem__(self, coords):
        if coords == "t":
            return self.solution.indepvararray[self.index]
        if coords in ["u", "u dot"]:
            return SLPointKey(self.solution, self.index, coords)
        return Points.Point.__getitem__(self, coords)

    def __setitem__(self, coords, item):
        if coords == 't':
            self.solution.indepvararray[self.index] = item
        Points.Point.__setitem__(self, coords, item)

    def __str__(self):
        return str({ "t" : self["t"], "u" : self["u"], "u dot" : self["u dot"]})

    __repr__ = __str__

class AUTOParameters(Points.Point):
    def __init__(self, kwd=None, **kw):
        self.coordnames = []
        self.parnames = None
        self.dimension = 0
        if kwd is None and kw == {}:
            return
        if kw.has_key("coordarray") and not kw.has_key("coordnames"):
            parnames = []
            for i in range(len(kw["coordarray"])):
                parnames.append("PAR("+str(i+1)+")")
            if kw.has_key("PAR"):
                self.parnames = kw["PAR"]
                parnames[:len(kw["PAR"])] = kw["PAR"]
            kw["coordtype"] = Points.float64
            kw["coordnames"] = parnames
        apply(Points.Point.__init__,(self,kwd),kw)

    def __call__(self,index):
        return self.coordarray[index-1]

    def __str__(self):
        parnames = self.parnames
        rep = ""
        for i in range(1,len(self)+1,5):
            j = min(i+4,len(self))
            rep = rep+"\nPAR(%-7s "%(str(i)+':'+str(j)+"):")
            if parnames is not None and i<=len(parnames):
                j2 = min(j,len(parnames))
                for k in range(i,j2+1):
                    rep = rep+"    %-15s"%parnames[k-1]
                rep = rep+"\n            "
            for k in range(i,j+1):
                rep = rep+"%19.10E"%self(k)
        return rep[1:]

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

class AUTOSolution(Points.Pointset,UserDict.UserDict,runAUTO.runAUTO):
    def __init__(self,input=None,offset=None,name=None):
	UserDict.UserDict.__init__(self)
        runAUTO.runAUTO.__init__(self)
        self.__start_of_header = None
        self.__start_of_data   = None
        self.__end              = None
        self.__fullyParsed     = False
        self._dims            = None
        self.name = name
        self.PAR = AUTOParameters()
        self.data_keys = ["PT", "BR", "TY number", "TY name", "LAB",
                          "ISW", "NTST", "NCOL", "Active ICP", "rldot",
                          "udotps"]
        self.long_data_keys = {
            "Parameters": "p",
            "parameters": "p",
            "Parameter NULL vector": "rldot",
            "Free Parameters": "Active ICP",
            "Point number": "PT",
            "Branch number": "BR",
            "Type number": "TY number",
            "Type name": "TY name",
            "Label": "LAB"}
	if input:
	    self.read(input,offset)            

    def __str__(self):
        if not(self.__fullyParsed):
            self.__readAll()
        keys = self.data.keys()
        for key in ["BR","PT","TY name","TY number","LAB","ISW","NTST","NCOL"]:
            keys.remove(key)
        keys.sort()
        rep=Points.Pointset.__repr__(self)
        rep=rep+ "\n  BR    PT  TY  LAB ISW NTST NCOL"
        rep=rep+ "\n%4d%6d%4s%5d%4d%5d%5d" % (self["BR"], self["PT"],
                                              self["TY name"], self["LAB"],
                                              self["ISW"], self["NTST"],
                                              self["NCOL"])
        for key in keys:
            v = self[key]
            if isinstance(v,Points.Pointset):
                v = repr(v)
            elif type(v) not in [type(1),type(1.0),Points.N.float64,type("")]:
                v = list(v)
            if type(v) == type([]) and type(v[0]) not in [type(1),type(1.0),
                                                          Points.N.float64]:
                v = map(str,v)
            rep=rep+"\n"+str(key)+": "+str(v)
        rep=rep+"\n"+str(self.PAR)
        return rep

    def __repr__(self):
        return self.__str__()

    def __setitem__(self,key,value):
        if (type(key) == type("") and not key in self.coordnames and
            key != self.indepvarname and not key in self.PAR.coordnames):
            shortkey = self.long_data_keys.get(key,key)
            if shortkey in self.data_keys:
                self.data[shortkey] = value
                return
            if shortkey == "p":
                self.PAR.__init__(coordarray=value)
                return
        try:
            Points.Pointset.__setitem__(self,key,value)
        except:
            self.PAR[key] = value

    def __getitem__(self,key):
        big_data_keys = ["data","Active ICP","rldot","p","udotps"]
        if (type(key) == type("") and not key in self.coordnames and
            key != self.indepvarname and not key in self.PAR.coordnames):
            shortkey = self.long_data_keys.get(key,key)
            if shortkey in big_data_keys and not(self.__fullyParsed):
                self.__readAll()
            if shortkey in self.data_keys:
                return self.data[shortkey]
            if shortkey == "p":
                return self.PAR
            if shortkey == "data":
                return self
        if not(self.__fullyParsed):
            self.__readAll()
        try:
            ret = Points.Pointset.__getitem__(self,key)
            if type(key) != types.IntType:
                return ret
            return SLPoint(ret, self, key)
        except:
            return self.PAR[key]

    def has_key(self,key):
        return (key == "data" or self.long_data_keys.has_key(key) or
                self.data.has_key(key) or
                Points.Pointset.has_key(self,key))

    def type(self):
	return parseB.type_translation(self["Type number"])["long name"]

    def readAllFilename(self,filename):
        try:
            inputfile = open(filename,"rb")
        except IOError:
            import gzip
            inputfile = gzip.open(filename+".gz","rb")
	self.readAll(inputfile)
	inputfile.close()

    def readFilename(self,filename):
        try:
            inputfile = open(filename,"rb")
        except IOError:
            import gzip
            inputfile = gzip.open(filename+".gz","rb")
	self.read(inputfile)
	inputfile.close()

    def writeFilename(self,filename):
	output = open(filename,"wb")
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
            input.seek(end)
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
            self.indepvarname = 't'
            self.__numEntriesPerBlock = int(data[7])
            self.coordnames = []
            for i in range(self.__numEntriesPerBlock-1):
                self.coordnames.append("U("+str(i+1)+")")

            self["BR"] = int(data[0])
            self["PT"] = int(data[1])
            self["TY number"] = int(data[2])
            self["TY name"] = parseB.type_translation(int(data[2]))["short name"]
            self["LAB"] = int(data[3])
            self.__numChangingParameters = int(data[4])
            self["ISW"] = int(data[5])
            self.__numSValues = int(data[6])
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
        if not Points.numpyimported:
            Points.importnumpy()        
        fromstring = Points.fromstring
        N = Points.N
        self.__fullyParsed = True
        n = self.__numEntriesPerBlock
        total = n * self.__numSValues + self.__numFreeParameters
        if self["NTST"] != 0:
            total = (total + 2 * self.__numChangingParameters +
                     (n-1) * self.__numSValues)
        solution = []
        j = 0
        nrows = self.__numSValues
        if hasattr(N,"transpose"):
            if fromstring:
                fdata = []
                if string.find(self.__data,"D") == -1:
                    fdata = fromstring(self.__data, dtype=float, sep=' ')
                if fdata == [] or len(fdata) > total:
                    fdata = N.array(map(parseB.AUTOatof,
                                        string.split(self.__data)), 'd')
            else:
                data = string.split(self.__data)
                try:
                    fdata = N.array(map(float, data), 'd')
                except:
                    fdata = N.array(map(parseB.AUTOatof, data), 'd')
            if total != len(fdata):
                raise PrematureEndofData
            ups = N.reshape(fdata[:n * nrows],(nrows,n))
            self.indepvararray = ups[:,0]
            self.coordarray = N.transpose(ups[:,1:])
        else: #no numpy
            data = string.split(self.__data)
            try:
                fdata = map(float, data)
            except:
                fdata = map(parseB.AUTOatof, data)
            if total != len(fdata):
                raise PrematureEndofData
            self.coordarray = []
            try:
                self.indepvararray = N.array(fdata[:n*nrows:n])
                for i in range(1,n):
                    self.coordarray.append(N.array(fdata[i:n*nrows:n]))
            except TypeError:
                self.indepvararray = N.array(map(lambda j, d=fdata: 
                                                 d[j], xrange(0,n*nrows,n)))
                for i in range(1,n):
                    self.coordarray.append(N.array(map(lambda j, d=fdata: 
                                                    d[j], xrange(i,n*nrows,n))))
        j = j + n * nrows

	# I am using the value of NTST to test to see if it is an algebraic or
	# ODE problem.
	if self["NTST"] != 0:
            nfpr = self.__numChangingParameters
            self["Active ICP"] = map(int,fdata[j:j+nfpr])
            j = j + nfpr
            self["rldot"] = fdata[j:j+nfpr]
            j = j + nfpr
            n = n - 1
            if hasattr(N,"transpose"):
                self["udotps"] = N.transpose(
                     N.reshape(fdata[j:j+n * self.__numSValues],(-1,n)))
            else:
                self["udotps"] = []
                try:
                    for i in range(n):
                        self["udotps"].append(N.array(fdata[i:n*nrows:n]))
                except TypeError:
                    for i in range(n):
                        self["udotps"].append(N.array(map(lambda j, d=fdata: 
                                                   d[j], xrange(i,n*nrows,n))))
            udotnames = []
            if self["NTST"] > 0:
                for i in range(self.__numEntriesPerBlock-1):
                    udotnames.append("UDOT("+str(i+1)+")")
            self["udotps"] = Points.Pointset({
                "coordarray": self["udotps"],
                "coordnames": udotnames,
                "name": self.name})
            self["udotps"]._dims = None
            j = j + n * nrows

        self.PAR.__init__(coordarray=fdata[j:j+self.__numFreeParameters])
        Points.Pointset.__init__(self,{
                "indepvararray": self.indepvararray,
                "indepvarname": self.indepvarname,
                "coordarray": self.coordarray,
                "coordnames": self.coordnames,
                "name": self.name})

    def __getattr__(self,attr):
        c = self.options["constants"]
        if attr == "c" and c is not None:
            return c
        if self.__fullyParsed:
            raise AttributeError
        self.__readAll()
        return getattr(self,attr)

    def write(self,output):
        nfpr = self.__numChangingParameters
        ndim = self.__numEntriesPerBlock-1
        npar = self.__numFreeParameters
        ntpl = self.__numSValues

        if self.__fullyParsed:
            ndim = len(self.coordarray)
            npar = len(self["Parameters"])
            ntpl = len(self["data"])

        if self["NTST"] != 0:
            if self.__fullyParsed:
                nfpr = len(self.get("Active ICP",[0]))
            nrd = 2 + ndim/7 + (ndim-1)/7
            nrowpr = (nrd * (self["NCOL"] * self["NTST"] + 1) +
                      (nfpr-1)/7+1 + (npar-1)/7+1 + (nfpr-1)/20+1)
        else:
            nrowpr = ndim/7+1 + (npar-1)/7+1
            
	line = "%6d%6d%6d%6d%6d%6d%8d%6d%8d%5d%5d%5d" % (self["BR"],
                                                         self["PT"],
                                                         self["TY number"],
                                                         self["LAB"],
                                                         nfpr,
                                                         self["ISW"],
                                                         ntpl,
                                                         ndim+1,
                                                         nrowpr,
                                                         self["NTST"],
                                                         self["NCOL"],
                                                         npar
                                                         )
	output.write(line+os.linesep)
        # If the file isn't already parsed, and we happen to know the position of
        # the end of the solution we can just copy from the raw data from the
        # input file into the output file.
        if not(self.__fullyParsed) and not(self.__end is None):
            output.write(self.__data)
            output.flush()
        # Otherwise we do a normal write.  NOTE: if the solution isn't already
        # parsed it will get parsed here.
        else:
            slist = []
            for i in range(len(self.indepvararray)):
                slist.append("    "+"%19.10E" % (self.indepvararray[i]))
                for j in range(1,len(self.coordarray)+1):
                    if j%7==0:
                        slist.append(os.linesep+"    ")
                    slist.append("%19.10E" % (self.coordarray[j-1][i]))
                slist.append(os.linesep)
            output.write(string.join(slist,""))
            # I am using the value of NTST to test to see if it is an algebraic or
            # ODE problem.
            if self["NTST"] != 0:
                j = 0
                for parameter in self.get("Active ICP",[0]):
                    output.write("%5d" % (parameter))
                    j = j + 1
                    if j%20==0:
                        output.write(os.linesep)
                if j%20!=0:
                    output.write(os.linesep)

                line = "    "
                i = 0
                for vi in self.get("rldot",[1.0]):
                    num = "%19.10E" % (vi)
                    if i != 0 and i%7==0:
                        line = line + os.linesep + "    "
                    line = line + num
                    i = i + 1
                output.write(line+os.linesep)

                # write UDOTPS
                slist = []
                for i in range(len(self.indepvararray)):
                    slist.append("    ")
                    c = self["udotps"].coordarray
                    l = len(c)
                    for j in range(len(self.coordarray)):
                        if j!=0 and j%7==0:
                            slist.append(os.linesep+"    ")
                        if j<l:
                            slist.append("%19.10E" %c[j][i])
                        else:
                            slist.append("%19.10E" %0)
                    slist.append(os.linesep)
                output.write(string.join(slist,""))

            line = "    "
            j = 0
            for parameter in self.PAR.toarray():
                num = "%19.10E" % (parameter)
                line = line + num 
                j = j + 1
                if j%7==0:
                    output.write(line+os.linesep)
                    line = "    "
            if j%7!=0:
                output.write(line+os.linesep)
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









