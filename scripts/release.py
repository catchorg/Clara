#!/usr/bin/env python

from __future__ import print_function
import os
import sys
import re

rootPath = os.path.dirname(os.path.realpath(os.path.dirname(sys.argv[0])))
claraPath = os.path.join( rootPath, "include/clara.hpp" )
readmePath = os.path.join( rootPath, "README.md" )

versionParser = re.compile( r'\s*\/\/\s*Clara\s+v([0-9]+)\.([0-9]+)(\.([0-9]+)(\-(.*)\.([0-9]*))?)?' )
readmeParser = re.compile( r'\s*#\s*Clara\s+v(.*)' )

warnings = 0

def precheck():
    global warnings
    f = open( claraPath, 'r' )
    lineNo = 0
    for line in f:
        lineNo = lineNo+1
        if "dynamic" in line:
            warnings = warnings + 1
            print( "** Warning: use of dynamic_cast on line {0}!".format(lineNo) )
    f.close()

class Version:
    def __init__(self):
        f = open( claraPath, 'r' )
        for line in f:
            m = versionParser.match( line )
            if m:
                self.majorVersion = int(m.group(1))
                self.minorVersion = int(m.group(2))
                self.patchNumber = int(m.group(4))
                if m.group(6) == None:
                    self.branchName = ""
                else:
                    self.branchName = m.group(6)
                if m.group(7) == None:
                    self.buildNumber = 0
                else:
                    self.buildNumber = int(m.group(7))
        f.close()

    def nonDevelopRelease(self):
        if self.branchName != "":
            self.branchName = ""
            self.buildNumber = 0
    def developBuild(self):
        if self.branchName == "":
            self.branchName = "develop"
            self.buildNumber = 0
        else:
            self.buildNumber = self.buildNumber+1

    def incrementBuildNumber(self):
        self.developBuild()
        self.buildNumber = self.buildNumber+1

    def incrementPatchNumber(self):
        self.nonDevelopRelease()
        self.patchNumber = self.patchNumber+1

    def incrementMinorVersion(self):
        self.nonDevelopRelease()
        self.patchNumber = 0
        self.minorVersion = self.minorVersion+1

    def incrementMajorVersion(self):
        self.nonDevelopRelease()
        self.patchNumber = 0
        self.minorVersion = 0
        self.majorVersion = self.majorVersion+1

    def getVersionString(self):
        versionString = '{0}.{1}.{2}'.format( self.majorVersion, self.minorVersion, self.patchNumber )
        if self.branchName != "":
            versionString = versionString + '-{0}.{1}'.format( self.branchName, self.buildNumber )
        return versionString

    def updateHeader(self):
        f = open( claraPath, 'r' )
        lines = []
        for line in f:
            m = versionParser.match( line )
            if m:
                lines.append( '// Clara v{0}'.format( self.getVersionString() ) )
            else:
                lines.append( line.rstrip() )
        f.close()
        f = open( claraPath, 'w' )
        for line in lines:
            f.write( line + "\n" )

    def updateReadme(self):
        f = open( readmePath, 'r' )
        lines = []
        changed = False
        for line in f:
            m = readmeParser.match( line )
            if m:
                lines.append( '# Clara v{0}'.format( self.getVersionString() ) )
                changed = True
            else:
                lines.append( line.rstrip() )
        f.close()
        if changed:
            f = open( readmePath, 'w' )
            for line in lines:
                f.write( line + "\n" )
        else:
            print( "*** Did not update README" )
def usage():
    print( "\n**** Run with patch|minor|major|dev|verify\n" )
    return 1

if len( sys.argv) == 1:
    exit( usage() )

precheck()

v = Version()
oldV = v.getVersionString()

cmd = sys.argv[1].lower()

if warnings > 0:
    print( "Found {0} issue(s)".format(warnings))
    exit(1)

if cmd == "verify":
    print( "No issues found")
    exit(0)
elif cmd == "patch":
    v.incrementPatchNumber()
elif cmd == "minor":
    v.incrementMinorVersion()
elif cmd == "major":
    v.incrementMajorVersion()
elif cmd == "dev":
    v.developBuild()
else:
    exit( usage() )

v.updateHeader()
v.updateReadme()

print( "Updated clara.hpp and README from {0} to v{1}".format( oldV, v.getVersionString() ) )
