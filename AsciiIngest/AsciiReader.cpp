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

#include "AsciiReader.h"
#include "AsciiDataObjDesc.h"
#include "AsciiHeaderReader.h"
#include "asciiingest_error.h"
#include <stdlib.h>
#include <iostream>
#include <stdexcept>
#include <string.h>
#include <assert.h>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "custGetLine.h"
#include <DType.h>
#include <AsserterFactory.h>

using namespace std;

namespace AsciiIngest {
    
    AsciiReader::AsciiReader() {
        // nothing to be done here for the moment
        
        currRow = -1;
        setCommentFlag("#");
        lineRexString = "";
        endLineFlag = "\n";
        lineReadType = 0;
        numFieldPerRow = -1;
        fastSepAccessArray = NULL;
        fastPrefixArray = NULL;
        fastLenArray = NULL;
        fieldPositionArray = NULL;
        
        askUserToValidateEndline = true;
        
        //allocate header
        AsciiHeaderReader * newHead = new AsciiHeaderReader();
        setHeader(newHead);
    }
    
    AsciiReader::AsciiReader(string newFileName, int numHeaderRows) {
        //allocate header
        askUserToValidateEndline = true;

        AsciiHeaderReader * newHead = new AsciiHeaderReader();
        newHead->setNumHeaderRows(numHeaderRows);
        setHeader(newHead);

        openFile(newFileName);
        
        readHeader();
        
        fastSepAccessArray = NULL;
        fastPrefixArray = NULL;
        fastLenArray = NULL;
        fieldPositionArray = NULL;
    }
    
    AsciiReader::~AsciiReader() {
        closeFile();
        
        if(fastSepAccessArray != NULL)
            free(fastSepAccessArray);
        if(fastPrefixArray != NULL)
            free(fastPrefixArray);
        if(fastLenArray != NULL)
            free(fastLenArray);
        if(fieldPositionArray != NULL)
            free(fieldPositionArray);
    }

    
    void AsciiReader::openFile(string newFileName) {
        if (fileStream.is_open())
            fileStream.close();
        
        fileStream.open(newFileName.c_str());
        
        if (!(fileStream.is_open())) {
            AsciiIngest_error("AsciiReader: Error in opening file.\n");
        }
        
        currRow = 0;
        fileName = newFileName;
    }
    
    void AsciiReader::closeFile() {
        if (fileStream.is_open())
            fileStream.close();
    }
    
    void AsciiReader::rewind() {
        if(fileStream.is_open()) {
            fileStream.clear();
            fileStream.seekg(0, ios::beg);
        }
    }
    
    void AsciiReader::skipHeader() {
        int numHeaderRows = ((AsciiHeaderReader*)getHeader())->getNumHeaderRows();

        string tmp;
        
        for(int i=0; i<numHeaderRows; i++) {
            getline(fileStream, tmp);
        }
    }
    
    void AsciiReader::readHeader() {
        assert(fileStream.is_open());
        
        int numHeaderRows = ((AsciiHeaderReader*)getHeader())->getNumHeaderRows();
        
        DBReader::HeaderReader * currHead = getHeader();
        
        string headerString = "";
        string currLine;
        
        rewind();
        for(int i=0; i<numHeaderRows; i++) {
            getline(fileStream, currLine);
            headerString.append(currLine + '\n');
        }
        
        currHead->setHeaderContent(headerString);
    }

    int AsciiReader::getNextRow() {
        assert(fileStream.is_open());
        assert(commentFlag.length() != 0);
        //assert(lineRexString.length() != 0);
        assert(numFieldPerRow != -1);
        //assert(endLineFlag.length() != 0);
        
       if(lineReadType == 0) {
            if(!getline(fileStream, buffer, endLineFlag[0])) {
                return 0;
            }
        } else {
            if(!mdelimGetline(fileStream, buffer, endLineFlag)) {
                return 0;
            }
        }
        
        //check if there is just a newline at the end
        if(strlen(buffer.c_str()) < 1) {
            return 0;
        }
        
        //check if there are characters which mark a comment
        boost::trim_left(buffer);
        if( strncmp(buffer.c_str(), commentFlag.c_str(), strlen(commentFlag.c_str())) == 0 ) {
            int err = getNextRow();
            return err;
        }
        
        //reset the fast access array
        memset(fieldPositionArray, -1, numFieldPerRow * sizeof(int64_t));
        
        currRow++;
        boost::trim_right(buffer);
        
        oneLine = buffer;
        
        return 1;
    }
    
    void AsciiReader::setLineParser(DBDataSchema::Schema * dbSchema) {
        //This method assumes that dbSchema is SORTED! It is not explicitely checked.
        lineRexString = "";
        
        numFieldPerRow = dbSchema->getArrSchemaItems().size();
        
        if(fastPrefixArray != NULL)
            free(fastPrefixArray);
        if(fastSepAccessArray != NULL)
            free(fastSepAccessArray);
        if(fastLenArray != NULL)
            free(fastLenArray);
        if(fieldPositionArray != NULL)
            free(fieldPositionArray);
        
        fastPrefixArray = (uint32_t*)malloc(numFieldPerRow * sizeof(uint32_t));
        if(fastPrefixArray == NULL) {
            AsciiIngest_error("AsciiReader: Error in allocating prefix array\n");
        }
        fastSepAccessArray = (uint32_t*)malloc(numFieldPerRow * sizeof(uint32_t));
        if(fastSepAccessArray == NULL) {
            AsciiIngest_error("AsciiReader: Error in allocating access array\n");
        }
        fastLenArray = (uint32_t*)malloc(numFieldPerRow * sizeof(uint32_t));
        if(fastLenArray == NULL) {
            AsciiIngest_error("AsciiReader: Error in allocating len array\n");
        }
        fieldPositionArray = (int64_t*)malloc(numFieldPerRow * sizeof(int64_t));
        if(fieldPositionArray == NULL) {
            AsciiIngest_error("AsciiReader: Error in allocating field position array\n");
        }
        
        memset(fastPrefixArray, 0, numFieldPerRow * sizeof(uint32_t));
        memset(fastLenArray, 0, numFieldPerRow * sizeof(uint32_t));
        memset(fastSepAccessArray, 0, numFieldPerRow * sizeof(uint32_t));
        memset(fieldPositionArray, -1, numFieldPerRow * sizeof(int64_t));
        
        //loop through the schema and find the last parsable entry (ommiting header and constant stuff)
        int lastItem = 0;
        for(int i=0; i<numFieldPerRow; i++) {
            AsciiDataObjDesc * currAsciiDatObj = (AsciiDataObjDesc*)dbSchema->getArrSchemaItems().at(i)->getDataDesc();
            
            if(currAsciiDatObj->getIsConstItem() == 0 && currAsciiDatObj->getIsHeaderItem() == 0) {
                lastItem = i;
            }
        }
        
        //loop through the schema and construct the "regex string"
        for(int i=0; i<numFieldPerRow; i++) {
            AsciiDataObjDesc * currAsciiDatObj = (AsciiDataObjDesc*)dbSchema->getArrSchemaItems().at(i)->getDataDesc();
            
            //only do something if this is not a header or constant item
            if(currAsciiDatObj->getIsConstItem() != 0 || currAsciiDatObj->getIsHeaderItem() != 0) {
                continue;
            }
            
            //if the separator string is empty, then characters of size DataLen are read
            //otherwise the part between the former and current separator is read.
            char * currSep = currAsciiDatObj->getSep();
            
            if(currAsciiDatObj->getPrefixSkip() != 0) {
                fastPrefixArray[i] = currAsciiDatObj->getPrefixSkip();
            }
            
            if(currSep[0] == '\0') {
                //donot capture prefix, capture lenData size string afterwards
                fastLenArray[i] = currAsciiDatObj->getLenData();
                
                if(i == lastItem) {
                    setEndLineFlag(currAsciiDatObj->getSep());
                }
            } else {
                //donot capture prefix, capture string up to the separator
                if(i == lastItem) {
                    setEndLineFlag(currAsciiDatObj->getSep());
                } else {
                    lineRexString.append(currAsciiDatObj->getSep());
                    if( i != 0)
                        fastSepAccessArray[i] = fastSepAccessArray[i-1] + (uint32_t)strlen(currAsciiDatObj->getSep());
                    else
                        fastSepAccessArray[i] = (uint32_t)strlen(currAsciiDatObj->getSep());
                }
            }
        }
    }

    bool AsciiReader::readFieldFromLine(DBDataSchema::DataObjDesc * thisItem, void* result) {
        uint64_t currStrPos;
        AsciiDataObjDesc * thisAsciiItem = (AsciiDataObjDesc*) thisItem;
        int currId = thisItem->getOffsetId()-1;
        
        if(currId < 0) {
            currId = 0;
        }
        
        //determine position in line (if this is a constant or header, the last position in the row will be stored)
        
        //check if this item has already been read. if not, determine the position of this item in the file
        //through recursion
        
        if(fieldPositionArray[currId] == -1) {
            //is this the first time we are reading something in this row?
            if(currId == 0) {
                fieldPositionArray[0] = 0;
            } else {
                //now read the field before this one to obtain the position
                readFieldFromLine(getSchema()->getArrSchemaItems().at(currId-1)->getDataDesc(), result);
            }
        }
        
        //check if this is a constant, header or normal DataObjDesc
        if(thisItem->getIsConstItem() == 1) {
            getConstItem(thisItem, result);
            
            if(thisItem->getOffsetId() != 0 && thisItem->getOffsetId() < numFieldPerRow-1) {
                fieldPositionArray[currId+1] = fieldPositionArray[currId];
            }

            return 0;
        } else if (thisItem->getIsHeaderItem() == 1) {
            AsciiDataObjDesc* currAsciiItem = (AsciiDataObjDesc*)thisItem;
            AsciiHeaderReader * thisHeader = (AsciiHeaderReader *)getHeader();
            
            if (currAsciiItem->getPrefixSkip() == 0 && currAsciiItem->getLenData() == 0) {
                //use the search in header functionality
                void * tmpRes = thisHeader->getItemBySearch(thisItem, currAsciiItem->getSep());
                
                //check assertions
                checkAssertions(thisItem, tmpRes);
                
                memcpy(result, tmpRes, DBDataSchema::getByteLenOfDType(thisItem->getDataObjDType()));
            }
            
            fieldPositionArray[currId+1] = currStrPos;

            return 0;
        }
        
        currStrPos = fieldPositionArray[currId];
        

        if(oneLine.size() <= currStrPos) {
            AsciiIngest_error("AsciiReader: Error in getItemInRow. You want to read more than there is available.\n");
        }
        
        //do we read to the delimiter or just a field of N chars?
        if(fastLenArray[currId] != 0) {
            tmpStr.assign(oneLine, currStrPos + fastPrefixArray[currId], fastLenArray[currId]);
            currStrPos += fastPrefixArray[currId] + fastLenArray[currId];
        } else {
            //read up to the next delimiter
            if(currId == 0 ) {
                //read non-greedy if needed...
                do {
                    mdelimGetfield((char*)oneLine.c_str() + (currStrPos + fastPrefixArray[currId]), 
                                   tmpStr, lineRexString.c_str(), fastSepAccessArray[currId]); 
                    currStrPos += tmpStr.size() + fastSepAccessArray[currId];
                } while (tmpStr.size() == 0 && thisAsciiItem->getGreedy() == 1);
            } else {
                do {
                    mdelimGetfield((char*)oneLine.c_str() + (currStrPos + fastPrefixArray[currId]), 
                                   tmpStr, lineRexString.c_str()+fastSepAccessArray[currId-1], 
                                   fastSepAccessArray[currId]-fastSepAccessArray[currId-1]); 
                    currStrPos += tmpStr.size() + fastSepAccessArray[currId]-fastSepAccessArray[currId-1];
                } while (tmpStr.size() == 0 && thisAsciiItem->getGreedy() == 1);
            }
        }
        
        //save the currStrPos to the fast access array. This marks the beginning of the i+1th element in the line
        if(currId+1 != numFieldPerRow) {
            fieldPositionArray[currId+1] = currStrPos;
        }

        //cast string accordingly and return
        return DBDataSchema::castStringToDType(tmpStr, thisItem->getDataObjDType(), result);        
    }

    bool AsciiReader::getItemInRow(DBDataSchema::DataObjDesc * thisItem, bool applyAsserters, bool applyConverters, void* result) {
        bool isNull = 0;
        
        //read data
        isNull = readFieldFromLine(thisItem, result);
        
        //if this is null, we can already return here
        if(isNull == true) {
            return isNull;
        }
        
        //check assertions
        if(applyAsserters == 1)
            checkAssertions(thisItem, result);
        
        //apply conversion
        if(applyConverters == 1)
            isNull = applyConversions(thisItem, result);
        
        return isNull;
    }
    
    void AsciiReader::getConstItem(DBDataSchema::DataObjDesc * thisItem, void* result) {
        //parse the constant item if needed
        if(thisItem->getConstData() == NULL) {
            void * tmpResult = malloc(DBDataSchema::getByteLenOfDType(thisItem->getDataObjDType()));
            string tmpStr(((AsciiDataObjDesc*)thisItem)->getSep());
            
            DBDataSchema::castStringToDType(tmpStr, thisItem->getDataObjDType(), tmpResult);

            //check assertions
            checkAssertions(thisItem, tmpResult);

            thisItem->setConstData(tmpResult);
        }
        
        memcpy(result, thisItem->getConstData(), DBDataSchema::getByteLenOfDType(thisItem->getDataObjDType()));
    }
        
    string AsciiReader::getFileName() {
        return fileName;
    }
    
    void AsciiReader::setCommentFlag(string newCommentFlag) {
        string rexStr = "(.*)";
        boost::trim(newCommentFlag);
        
        //parse the split character. this is between two quotes ''
        commentRex = boost::xpressive::sregex::compile( rexStr + newCommentFlag + rexStr );
        commentFlag = newCommentFlag;
    }
    
    string AsciiReader::getCommentFlag() {
        return commentFlag;
    }
    
    void AsciiReader::setEndLineFlag(string newEndLineFlag) {
        endLineFlag = newEndLineFlag;
        if(newEndLineFlag.size() <= 1) {
            lineReadType = 0;
        } else {
            lineReadType = 1;
        }
        
        //ask user for validation
        if(askUserToValidateEndline == true && endLineFlag.compare("\n") != 0) {
            string answer;
            printf("\nPlease validate this assumption:\n");
            printf("You specified lines to not end with a NEWLINE!\nEndline character: '%s'\n", endLineFlag.c_str());
            printf("Is this what you have intended??\n");
            printf("Please enter (Y/n): ");
            getline(cin, answer);
            
            if(answer.length() != 0 && answer.at(0) != 'Y' && answer.at(0) != 'y') {
                AsciiIngest_error("AsciiReader: You did not approve my suggestion. Please adjust the structure file and try again.\n");
            }
        }
        
    }
    
    string AsciiReader::getEndLineFlag() {
        return endLineFlag;
    }
    
    int AsciiReader::getCurrRow() {
        return currRow;
    }
    
    void AsciiReader::setAskUserToValidateEndline(bool newVal) {
        askUserToValidateEndline = newVal;
    }

}
