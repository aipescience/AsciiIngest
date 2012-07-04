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

#include "AsciiHeaderReader.h"
#include "AsciiDataObjDesc.h"
#include "asciiingest_error.h"
#include <string>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

namespace AsciiIngest {
    
    AsciiHeaderReader::AsciiHeaderReader() {
        numHeaderRows = 0;
    }
    
    
    AsciiHeaderReader::~AsciiHeaderReader() {
        
    }
    
    void* AsciiHeaderReader::getItem(DBDataSchema::DataObjDesc * thisItem) {
        //parse the header item if needed
        if(thisItem->getConstData() == NULL) {
            void * tmpResult = malloc(DBDataSchema::getByteLenOfDType(thisItem->getDataObjDType()));
            
            AsciiDataObjDesc * thisAsciiItem = (AsciiDataObjDesc*) thisItem;
            string headerCont = getHeaderContent();
            stringstream headerStream(headerCont, stringstream::in);
            
            string currLine;
            //find the row
            for(int i=0; i<thisAsciiItem->getLineNum(); i++) {
                getline(headerStream, currLine);
            }
            
            string value = currLine.substr(thisAsciiItem->getPrefixSkip(), thisAsciiItem->getLenData());
            
            DBDataSchema::castStringToDType(value, thisItem->getDataObjDType(), tmpResult);

            //ask user for validation
            if(getAskUserToValidateRead() == 1) {
                string answer;
                printf("\nPlease validate this header read:\n");
                printf("Field name: %s\n", thisItem->getDataObjName().c_str());
                printf("You wanted to extract line %i, starting from character %i to %i.\n", thisAsciiItem->getLineNum(), thisAsciiItem->getPrefixSkip(), thisAsciiItem->getPrefixSkip() + thisAsciiItem->getLenData());
                printf("I have found: %s\n", value.c_str());
                printf("Please enter (Y/n): ");
                getline(cin, answer);
                
                if(answer.length() != 0 && answer.at(0) != 'Y' && answer.at(0) != 'y') {
                    AsciiIngest_error("AsciiHeaderReader: You did not approve my suggestion. Please adjust the structure file and try again.\n");
                }
            }

            thisItem->setConstData(tmpResult);
        }
        
        return thisItem->getConstData();
    }
    
    void AsciiHeaderReader::setNumHeaderRows(int newNumHeaderRows) {
        assert(newNumHeaderRows >= 0);
        
        numHeaderRows = newNumHeaderRows;
    }
    
    int AsciiHeaderReader::getNumHeaderRows() {
        return numHeaderRows;
    }
}
