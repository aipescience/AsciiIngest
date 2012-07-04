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

#include <Reader.h>
#include <Schema.h>
#include <string>
#include <fstream>
#include <stdio.h>
#include <boost/xpressive/xpressive.hpp>

#ifndef ASCIIREADER_H_
#define ASCIIREADER_H_

namespace AsciiIngest {
    
    class AsciiReader : public DBReader::Reader {
    private:
        std::ifstream fileStream;
        
        std::string fileName;
        
        std::string commentFlag;
        
        std::string endLineFlag;
        int lineReadType;
        bool askUserToValidateEndline;
        
        int currRow;
        unsigned long numFieldPerRow;
        
        std::string buffer;
        boost::xpressive::sregex commentRex;
        
        std::string lineRexString;
        uint32_t * fastSepAccessArray;
        uint32_t * fastPrefixArray;
        uint32_t * fastLenArray;
        int64_t * fieldPositionArray;
        std::string oneLine;
        boost::xpressive::sregex lineRex;
        boost::xpressive::smatch lineData;
        
        
        //this is here for performance reasons. used to be in getItemInRow, but is allocated statically here to get rid
        //of many frees
        std::string tmpStr;
        
        bool readFieldFromLine(DBDataSchema::DataObjDesc * thisItem, void* result);
        
    public:
        AsciiReader();
        AsciiReader(std::string newFileName, int numHeaderRows);
        ~AsciiReader();
        
        void openFile(std::string newFileName);

        void closeFile();
        
        void rewind();

		void skipHeader();

        void readHeader();
        
        int getNextRow();
        
        bool getItemInRow(DBDataSchema::DataObjDesc * thisItem, bool applyAsserters, bool applyConverters, void* result);
        
        /*! \brief retrieves a constant item
         \param DBDataSchema::DataObjDesc thisItem: the data object describing what needs to be
         read
         \param void* result: writes the data item in the row to this address space (buffer that is previously allocated!)
         
         Reads/parses the current constant item. If the constant item has not yet been parsed and saved in the constant
         data field of the DataObjDesc object, this function does that. If it finds an already converted item, this returns
         the pointer to the data*/
        void getConstItem(DBDataSchema::DataObjDesc * thisItem, void* result);
      
        void setLineParser(DBDataSchema::Schema * dbSchema);
        
        std::string getFileName();
        
        void setCommentFlag(std::string newCommentFlag);
        
        std::string getCommentFlag();
        
        void setEndLineFlag(std::string newEndLineFlag);
        
        std::string getEndLineFlag();
        
        int getCurrRow();
        
        void setAskUserToValidateEndline(bool newVal);
    };
    
}

#endif /* ASCIIREADER_H_ */
