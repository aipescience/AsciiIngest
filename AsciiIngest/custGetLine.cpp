/*  
 *  Copyright (c) 2012, Adrian M. Partl <apartl@aip.de>, 
 *                      eScience team AIP Potsdam
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  See the NOTICE file distributed with this work for additional
 *  information regarding copyright ownership. You may obtain a copy
 *  of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


#include "custGetLine.h"
#include "asciiingest_error.h"
#include <iostream>
#include <string>
#include <assert.h>
#include <string.h>
#include <boost/format.hpp>

using namespace std;

int mdelimGetline(ifstream& inStream, string& outString, string& delim) {
    assert(delim.size() > 0);
    
    const char * delStr = delim.c_str();
    
    unsigned long lenDelim = delim.size()-1;
    
    int found = 0;
    
    if(AING_SECURE_GETLINE == 1) {
        if(delim.compare("/n") == 0) {
            AsciiIngest_error("mdelimGetline: You have defined '/n' as a delimiter. \nTo save you from freezing your machine due to this possible mistake, I will not continue. \nPlease assess if you really wanted to use '/n'. \nIf yes, recompile me by defining AING_SECURE_GETLINE = 0!\nSorry, but I have to be strict here...\n");
        } else if(delim.compare("/r") == 0) {
            AsciiIngest_error("mdelimGetline: You have defined '/r' as a delimiter. \nTo save you from freezing your machine due to this possible mistake, I will not continue. \nPlease assess if you really wanted to use '/r'. \nIf yes, recompile me by defining AING_SECURE_GETLINE = 0!\nSorry, but I have to be strict here...\n");
        } else if(delim.compare("/t") == 0) {
            AsciiIngest_error("mdelimGetline: You have defined '/t' as a delimiter. \nTo save you from freezing your machine due to this possible mistake, I will not continue. \nPlease assess if you really wanted to use '/t'. \nIf yes, recompile me by defining AING_SECURE_GETLINE = 0!\nSorry, but I have to be strict here...\n");
        } else if(delim.compare("/r/n") == 0) {
            AsciiIngest_error("mdelimGetline: You have defined '/r/n' as a delimiter. \nTo save you from freezing your machine due to this possible mistake, I will not continue. \nPlease assess if you really wanted to use '/r/n'. \nIf yes, recompile me by defining AING_SECURE_GETLINE = 0!\nSorry, but I have to be strict here...\n");
        }
    }
    
    outString = "";
    
    string buffer;
    char delimBuff[100];
    memset(delimBuff, '\0', sizeof(delimBuff));
    
    while(found == 0) {
        if(!getline(inStream, buffer, delim[0])) {
            return 0;
        }
        
        outString.append(buffer);
        
        inStream.read(delimBuff, lenDelim);
        if(!strcmp(delimBuff, &(delStr[1])) || inStream.eof() || lenDelim == 0) {
            found = 1;
            break;
        } 
        
        outString.append( boost::str(boost::format("%c%s") % delim[0] % delimBuff) );
    }
    
    return 1;    
}

unsigned long mdelimGetfield(char * charArray, string& outString, const char * delim, uint32_t lenDelim) {
    int found = 0;
    char * fieldEnd = charArray;
    
    while(found == 0) {
        fieldEnd = strchr(fieldEnd, delim[0]);
        
        //if this is a space, also parse for \ts
        if(delim[0] == 32) {
            char * tmp =charArray;
            tmp = strchr(tmp, 9);   //check for tabulator
            if(tmp < fieldEnd && tmp != NULL)
                fieldEnd = tmp;
        }
        
        if(lenDelim > 1 && !strcmp(fieldEnd, &(delim[1]))) {
            found = 1;
            break;
        } else if (lenDelim == 1) {
            found = 1;
        } else if (strlen(fieldEnd) == 0) {
            found = 1;
        }
    }
   
   if(fieldEnd - charArray == 0) {
        outString.assign("\0");
   } else {
        outString.assign(charArray, fieldEnd - charArray);
   }
    
    return outString.length();
}
