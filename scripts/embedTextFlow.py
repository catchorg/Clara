# Execute this script any time you import a new copy of textflow into the third_party area
import os
import sys
import embed

rootPath = os.path.dirname(os.path.realpath( os.path.dirname(sys.argv[0])))

filename = os.path.join( rootPath, "third_party", "TextFlow.hpp" )
outfilename = os.path.join( rootPath, "include", "clara_textflow.hpp" )


# Mapping of pre-processor identifiers
idMap = {
   "TEXTFLOW_HPP_INCLUDED": "CLARA_TEXTFLOW_HPP_INCLUDED",
    "TEXTFLOW_CONFIG_CONSOLE_WIDTH": "CLARA_TEXTFLOW_CONFIG_CONSOLE_WIDTH"
    }

# outer namespace to add
outerNamespace = "clara"

mapper = embed.LineMapper( idMap, outerNamespace )
mapper.mapFile( filename, outfilename )