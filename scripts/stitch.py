#!/usr/bin/env python

# This is an initial cut of a general header stitching script.
# It is currently hard-coded to work with Clara, but that is purely
# a path thing. The next step is the genericise this further and
# apply it to Catch. There will undoubtedly be issues to fix there.
# After that there are further tweaks to make such as suppressing initial
# comments in stitched headers and adding a single new header block to
# the output file, suppressing conditional blocks where the identifiers
# are known and moving all headers not in conditional blocks to the top
# of the file

import os
import sys
import re
import datetime
import string

preprocessorRe = re.compile( r'\s*#.*' )

includesRe = re.compile( r'\s*#\s*include.*' ) # all #includes
sysIncludesRe = re.compile( r'\s*#\s*include\s*<(.*)>' ) # .e.g #include <vector>
prjIncludesRe = re.compile( r'\s*#\s*include\s*"(.*)"' ) # e.g. #include "myheader.h"

fdefineRe = re.compile( r'\s*#\s*define\s*(\S*)\s*\(' ) # #defines that take arguments
defineRe = re.compile( r'\s*#\s*define\s*(\S*)\s+(\S*)' ) # all #defines
undefRe = re.compile( r'\s*#\s*undef\s*(\S*)' ) # all #undefs

ifdefCommonRe = re.compile( r'\s*#\s*if' ) # all #ifdefs
ifdefRe = re.compile( r'\s*#\s*ifdef\s*(\S*)' )
ifndefRe = re.compile( r'\s*#\s*ifndef\s*(\S*)' )
endifRe = re.compile( r'\s*#\s*endif(.*)' )
elseRe = re.compile( r'\s*#\s*else' )
ifRe = re.compile( r'\s*#\s*if\s+(.*)' )

rootPath = os.path.dirname(os.path.realpath( os.path.dirname(sys.argv[0])))
srcsPath = os.path.join( rootPath, 'src' )
includePath = os.path.join( rootPath, 'include' )
singleIncludePath = os.path.join( rootPath, 'single_include' )
extIncludePath = os.path.join( includePath, "external" )
thirdPartyPath = os.path.join( rootPath, 'third_party' )
firstHeaderPath = os.path.join( includePath, 'clara.hpp' )
outPath = os.path.join( singleIncludePath, 'clara.hpp' )

o = open( outPath, 'w' )

systemHeaders = set([])
projectHeaders = set([])
defines = set([])
LevelMax = 9999
suppressUntilLevel = LevelMax
level = 0

class FileParser:
    filename = ""

    def __init__( self, filename ):
        self.filename = filename

    def findHeader( self, headerFile ):

        # !TBD: make this array based

        # First check next to current file
        dir, _ = os.path.split( self.filename )
        fullPath = os.path.join( dir, headerFile )
        if os.path.isfile(fullPath):
            return fullPath

        # Next check include folder
        fullPath = os.path.join( includePath, headerFile )
        if os.path.isfile(fullPath):
            return fullPath

        # Then ext folder
        fullPath = os.path.join( extIncludePath, headerFile )
        if os.path.isfile(fullPath):
            return fullPath

        # Finally check ThirdParty folder
        fullPath = os.path.join( thirdPartyPath, headerFile )
        if os.path.isfile(fullPath):
            return fullPath

        raise FileNotFoundError( "Cannot locate include file: '" + filename + "'" )


    def parse( self ):
        with open( self.filename, 'r' ) as f:
            for line in f:
                if preprocessorRe.match( line ):
                    self.handlePreprocessor( line )
                else:
                    self.handleNonPP( line )

    def handleNonPP( self, line ):
        if suppressUntilLevel > level:
            self.writeLine( line )

    def handlePreprocessor( self, line ):
        if includesRe.match( line ):
            if suppressUntilLevel > level:
                self.handleInclude( line )
        else:
            self.handleNonIncludePP( line )

    def handleInclude( self, line ):
        global systemHeaders
        global projectHeaders
        m = sysIncludesRe.match( line )
        if m:
            headerFile = m.group(1)
            if not headerFile in systemHeaders:
                self.writeLine( "#include <{0}>".format( headerFile ) )
                systemHeaders.add( headerFile )

        m = prjIncludesRe.match( line )
        if m:
            headerFile = m.group(1)
            if not headerFile in projectHeaders:            
                self.writeLine( '\n// ----------- #included from {0} -----------'.format( headerFile ) )
                self.writeLine( "" )
                projectHeaders.add( headerFile )

                _, filename = os.path.split( self.filename )
                headerPath = self.findHeader( headerFile )
                p = FileParser( headerPath )
                p.parse()
                self.writeLine( '\n// ----------- end of #include from {0} -----------'.format( headerFile ) )
                self.writeLine( '// ........... back in {0}'.format( filename ) )
                self.writeLine( "" )

    def handleNonIncludePP( self, line ):
        m = endifRe.match( line )
        if m:
            self.handleEndif( m.group(1) )
        m = elseRe.match( line )
        if m:
            self.handleElse()

        if ifdefCommonRe.match( line ):
            self.handleIfdefCommon( line )

        if suppressUntilLevel > level:
            m = defineRe.match( line )
            if m:
                if fdefineRe.match( line ):
                    self.writeLine( line )
                else:
                    self.handleDefine( m.group(1), m.group(2) )
            m = undefRe.match( line )
            if m:
                self.handleUndef( m.group(1) )
        
    def handleDefine( self, define, value ):
        self.writeLine( "#define {0} {1}".format( define, value ) )
        defines.add( define )

    def handleUndef( self, define ):
        self.writeLine( "#undef {0}".format( define ) )
        defines.remove( define )

    def handleIfdefCommon( self, line ):
        global level
        level = level + 1
        m = ifndefRe.match( line )
        if m:
            self.handleIfndef( m.group(1) )
        else:
            m = ifdefRe.match( line )
            if m:
                self.handleIfdef( m.group(1) )
            else:
                m = ifRe.match( line )
                if m:
                    self.handleIf( m.group(1) )
                else:
                    print "****** error ***** " + line

    def handleEndif( self, trailing ):
        global level
        global suppressUntilLevel
        self.writeLine( "#endif{0}".format( trailing ) )
        level = level - 1
        if level == suppressUntilLevel:
            suppressUntilLevel = LevelMax

    def handleElse( self ):
        global suppressUntilLevel
        self.writeLine( "#else" )
        if level == suppressUntilLevel+1:
            suppressUntilLevel = LevelMax

    def handleIfndef( self, define ):
        self.writeLine( "#ifndef {0}".format( define ) )
        if define not in defines:
            suppressUntilLevel = level
    
    def handleIfdef( self, define ):
        self.writeLine( "#ifdef {0}".format( define ) )
        if define in defines:
            suppressUntilLevel = level

    def handleIf( self, trailing ):
        global level
        global suppressUntilLevel
        self.writeLine( "#if {0}".format( trailing ) )
        level = level + 1
#        if level == suppressUntilLevel:
#            suppressUntilLevel = LevelMax


    def writeLine( self, line ):
        o.write( line.rstrip() + "\n" )

print( "from: " + firstHeaderPath )
print( "to: " + outPath )
p = FileParser( firstHeaderPath )
p.parse()

o.close()

print "-------------"
print level
#for h in systemHeaders:
#    print "#include <" + h + ">"
#print
#
#for h in projectHeaders:
#    print '#include "' + h + '"'
#print
#for d in defines:
#    print d
#print
