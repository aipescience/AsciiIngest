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


#include <fstream>
#include <string>
#ifndef _WIN32
#include <stdint.h>
#else
#include <stdint_win.h>
#endif

#ifndef AsciiIngest_custGetLine_h
#define AsciiIngest_custGetLine_h

#ifndef AING_SECURE_GETLINE
    #define AING_SECURE_GETLINE 1
#endif

/*! \brief mdelimGetline(ifstream& inStream, string& outString, string& delim), crude reimplementation of std::getline allowing for delimiters of variable size.
 \param ifstream& inStream: an ifstream to read from
 \param string& outString: an string to write the "line" to
 \param string& delim: an string holding the delimiter
 \return int: error code
 
 This function is from features identical to std::getline. The only difference is, that mdelimGetline 
 can handle delimiters, that contain more than one character. The implementation is crude, uses std::getline
 and ifstream::read. Optimally this would reimplement the getline from the libstdc++. However this is postboned
 to the future. At the moment, the performance of this routine might be slow.*/
int mdelimGetline(std::ifstream& inStream, std::string& outString, std::string& delim);

/*! \brief mdelimGetfield(char * charArray, string& outString, const char * delim, uint32_t lenDelim), reads the next field up to a given separator
 \param char * charArray: the string from which to read the next field
 \param string& outString: an string to write the field to
 \param const char * delim: current field delimiter
 \param uint32_t lenDelim: length of the delimiter string
 \return length of the string that was read
 
 This function reads from the starting pointer defined in charArray to the next delimiter string. The output is then written to
 outString. This function can be used to parse a string with different delimiters for each field. Also delimiters with size
 greater 1 (i.e. multi character delimiters) are supported. If you want to itereate through a string, pass the start pointer to
 the function and in the next call to this function pass the same pointer shifter by outString.size() + strlen(delim)!*/
unsigned long mdelimGetfield(char * charArray, std::string& outString, const char * delim, uint32_t lenDelim);

#endif
