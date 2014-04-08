/*  
 *  Copyright (c) 2012 - 2014, Adrian M. Partl <apartl@aip.de>, 
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

#include <DataObjDesc.h>

#ifndef AsciiIngest_AsciiDataObjDesc_h
#define AsciiIngest_AsciiDataObjDesc_h

namespace AsciiIngest {
    
    class AsciiDataObjDesc : public DBDataSchema::DataObjDesc  {
        
    private:
        /*! \var int lineNum
         only used for reading headers: the line number where to read the header information from
         */
		int lineNum;

        /*! \var int prefixSkip
         number of bytes/characters (depends on implementation) that need to be 
         skiped before reading the real data field from the file. This is usefull 
         when getting rid of the fortran field separator.
         */
		int prefixSkip;
        
        /*! \var int lenData
         length in bytes of the field to read. If there are delimiters and this field
         is 0, the whole data set is read. Otherwise only the number of given bytes will
         be read
         */
		int lenData;
        
        /*! \var char sep[128]
         stores the separator between this and the next data field
         */
        char sep[128];

        /*! \var int greedySep
         should the delimiter/separator be treated as greedy or not? Should multiple separators
         be merged into one or not? if this is set to 1 = greedy, 0 = non-greedy
         */
		int greedySep;
        
    public:
        AsciiDataObjDesc();
        ~AsciiDataObjDesc();
        
        int getPrefixSkip();
        
		void setPrefixSkip(int newPrefixSkip);

        int getLenData();
        
		void setLenData(int newLenData);

        int getLineNum();
        
		void setLineNum(int newLineNum);

        char * getSep();
        
		void setSep(std::string newSep);
        
        void setGreedy(int newGreedy);
        
        int getGreedy();
       
    };

}

#endif
