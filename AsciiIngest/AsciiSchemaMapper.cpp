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

#include "AsciiSchemaMapper.h"
#include "asciiingest_error.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <stdexcept>
#include <assert.h>
#include <vector>
#include <string>
#include <boost/xpressive/xpressive.hpp>
#include <boost/algorithm/string.hpp>
#include <DType.h>
#include <SchemaItem.h>
#include "AsciiDataObjDesc.h"

using namespace std;
using namespace DBDataSchema;

namespace AsciiIngest {
    
    AsciiSchemaMapper::AsciiSchemaMapper() {
        numCols = 0;
    }
    
    AsciiSchemaMapper::AsciiSchemaMapper(string formatFile, DBAsserter::AsserterFactory * assertFac, DBConverter::ConverterFactory * convFac) {
        assert(assertFac != NULL);
        
    	AsciiSchemaMapper();

        setFormatFileName(formatFile);
        setAssertionFactory(assertFac);
        setConverterFactory(convFac);
    }
    
    AsciiSchemaMapper::~AsciiSchemaMapper() {
        if (fileStream.is_open())
            fileStream.close();

    }
    
    DBDataSchema::Schema * AsciiSchemaMapper::generateSchema(string dbName, string tblName) {
        DBDataSchema::Schema * returnSchema = new Schema();
        
    	assert(fileStream.is_open());
        
        //rewind file
        fileStream.seekg(0, ios::beg);
        
        returnSchema->setDbName(dbName);
        returnSchema->setTableName(tblName);

        string currLine;
        
        //parse the format file and generate the approperiate schema

    	//the number of columns to read stands at the top of the format file
    	//everything that is commented out with "#" is beeing ignored

        currLine = getNextLineInFile();

        //check if this is realy the line we want. it should not have any spaces (after
        //timming in getNextLine
        
        if(currLine.find(' ') != std::string::npos) {
            AsciiIngest_error("AsciiSchemaMapper: This file does not seem to be a format file. Failed at numCols.\n");
        }
        
        sscanf(currLine.c_str(), "%i\n", &numCols);

        //the number of columns to read stands at the top of the format file
    	//everything that is commented out with "#" is beeing ignored
        
        currLine = getNextLineInFile();
        
        //check if this is realy the line we want. it should not have any spaces (after
        //timming in getNextLine
        
        if(currLine.find(' ') != std::string::npos) {
            AsciiIngest_error("AsciiSchemaMapper: This file does not seem to be a format file. Failed at numHeaderRows.\n");
        }
        
        sscanf(currLine.c_str(), "%i\n", &numHeaderRows);
        
        //read the file into partially parsed substrings, which are held in the rowStringParts structure. Create a vector
        //for further processing. This is to prevent a reread of the file, and any post/re-parsing of the file can be done
        //through this vector
        vector<rowStringParts> fileContent;
        for(int i=0; i<numCols; i++) {
            fileContent.push_back(getNextRowStringParts());
        }
        
        //as a next step, setup the schema items, without yet dealing with asserters and converters that have variables.
        //They need to be treated, after the whole schema has already been initialised...
        vector<SchemaItem*> schemaItemVec;
        for(int i=0; i<fileContent.size(); i++) {
            returnSchema->addItemToSchema(initSchemaItem(fileContent.at(i)));
        }
        
        //now parse the assertions and convertions again to parse any variables that are given for functions that need them
        //link them with other data fields if necessary
        for(int i=0; i<returnSchema->getArrSchemaItems().size(); i++) {
            reparseForVariables(returnSchema, returnSchema->getArrSchemaItems().at(i), fileContent.at(i));
        }


        //sort by offsetId and check if sort was ok, resp. if there are no same offsetIds more than once
        returnSchema->sortSchema();
        int currOff = returnSchema->getArrSchemaItems().at(0)->getDataDesc()->getOffsetId();
        for(int i=1; i<returnSchema->getArrSchemaItems().size(); i++) {
            if(returnSchema->getArrSchemaItems().at(i)->getDataDesc()->getOffsetId() <= currOff) {
                AsciiIngest_error("AsciiSchemaMapper: Error in format file. Same column is read more than once\n");
            }
            currOff = returnSchema->getArrSchemaItems().at(i)->getDataDesc()->getOffsetId();
        }

        return returnSchema;
    }
    
    
    void AsciiSchemaMapper::setFormatFileName(string newFormatFile) {
        if (fileStream.is_open())
            fileStream.close();

        fileStream.open(newFormatFile.c_str());
        
        if (!(fileStream.is_open())) {
        	AsciiIngest_error("AsciiSchemaMapper: Error in opening file.\n");
        }
        
        formatFileName = newFormatFile;
    }
    
    string AsciiSchemaMapper::getFormatFileName() {
        return formatFileName;
    }

    void AsciiSchemaMapper::setAssertionFactory(DBAsserter::AsserterFactory * newFactory) {
        assert(newFactory != NULL);
        assertFac = newFactory;
    }
    
    DBAsserter::AsserterFactory * AsciiSchemaMapper::getAssertionFactory() {
        return assertFac;
    }

    void AsciiSchemaMapper::setConverterFactory(DBConverter::ConverterFactory * newFactory) {
        assert(newFactory != NULL);
        convFac = newFactory;
    }
    
    DBConverter::ConverterFactory * AsciiSchemaMapper::getConverterFactory() {
        return convFac;
    }

    int AsciiSchemaMapper::getNumCols() {
        return numCols;
    }

    int AsciiSchemaMapper::getNumHeaderRows() {
        return numHeaderRows;
    }
    
    void AsciiSchemaMapper::setGreedyTo(int greedy, DBDataSchema::Schema * inThisSchema) {
        assert(inThisSchema != NULL);
        assert(greedy == 0 || greedy == 1);
        
        //loop through the schema and set all greedy to whatever specified
        for(int i=1; i<inThisSchema->getArrSchemaItems().size(); i++) {
            AsciiDataObjDesc * currObj = (AsciiDataObjDesc*)(inThisSchema->getArrSchemaItems().at(i)->getDataDesc());
            currObj->setGreedy(greedy);
        }
    }

/////////// PRIVATE METHODS

    string AsciiSchemaMapper::getNextLineInFile() {
        string buffer;
        if(!std::getline(fileStream, buffer)) {
            AsciiIngest_error("AsciiSchemaMapper: Error in reading file. Did you specify the right number of columns?\n");
        }
        
        //check if there is a "#" which marks a comment
        boost::trim_left(buffer);
        while (buffer[0] == '#') {
            //get new line
            std::getline(fileStream, buffer);
            boost::trim_left(buffer);
        }
        
        boost::trim_right(buffer);
        return buffer;
    }
    
    DBType AsciiSchemaMapper::getDBType(string fromDType) {
        boost::to_upper(fromDType);
        
        if(fromDType == "CHAR") {
            return DBT_CHAR;
        }
        
        if(fromDType == "INT2") {
            return DBT_SMALLINT;
        }
        
        if(fromDType == "INT4") {
            return DBT_INTEGER;
        }
        
        if(fromDType == "INT8") {
            return DBT_BIGINT;
        }

        if(fromDType == "UINT2") {
            return DBT_USMALLINT;
        }
        
        if(fromDType == "UINT4") {
            return DBT_UINTEGER;
        }
        
        if(fromDType == "UINT8") {
            return DBT_UBIGINT;
        }

        if(fromDType == "REAL4") {
            return DBT_FLOAT;
        }
        
        if(fromDType == "REAL8") {
            return DBT_REAL;
        }
        
        AsciiIngest_error("AsciiSchemaMapper getDBType: Type not known...");
        return (DBType)0;    
    }
    
    rowStringParts AsciiSchemaMapper::getNextRowStringParts() {
        string currLine;
        rowStringParts result;
        
        currLine = getNextLineInFile();
        
        //before starting with anything, parse the separator. could be that this is includes spaces which would not be nice
        //to split
        
        //parse the split character. this is between two quotes ''
        boost::xpressive::sregex rex = boost::xpressive::sregex::compile( ".*'(.*)'.*" );
        boost::xpressive::smatch what;
        if( regex_match(currLine, what, rex) ) {
            result.sep.assign(what[1].str());
        } else { 
            string errStr = "AsciiSchemaMapper: Format file error: Error in separator declaration.\n";
            errStr += "Current line in struct file: " + currLine + "\n";
            AsciiIngest_error(errStr.c_str());
        }
        
        //now replace any whitespaces in that separator string...
        rex = boost::xpressive::sregex::compile( "'(.*)'" );
        string repl("'sep'");
        currLine = regex_replace(currLine, rex, repl);
        
        vector<string> splitVec;
        boost::split(splitVec, currLine, boost::is_any_of("\t "), boost::token_compress_on);
        
        if(splitVec.size() < 6) {
            string errStr = "AsciiSchemaMapper: This file does not seem to be a format file. Failed at split.\n";
            errStr += "Current line in struct file: " + currLine + "\n";
            AsciiIngest_error(errStr.c_str());
        }
        
        result.colNumString.assign(splitVec[0]);
        boost::trim_left(result.colNumString);
        boost::trim_right(result.colNumString);
        
        result.DType = splitVec[1];
        
        //check if DType is known and ok
        if(!DBDataSchema::testDType(result.DType)) {
            string errStr = "AsciiSchemaMapper: Format file error: You did not provide a proper DType.\n";
            errStr += "Current line in struct file: " + currLine + "\n";
            AsciiIngest_error(errStr.c_str());
        }
        
        result.prefixLen.assign(splitVec[2]);
        
        result.dataLen.assign(splitVec[3]);
        
        result.schemaColName.assign(splitVec[5]);
        
        //for the next two field there might be the possibility, that the user put a space after the , to seperate
        //the assert and convert lists. fix this...
        int j = 6;
        for(j=j; j < splitVec.size(); j++) {
            boost::trim_right(splitVec[j]);
            if(splitVec[j].at(splitVec[j].size()-1) == ',') {
                boost::trim_left(splitVec[j]);
            }
            
            result.assertConvList.append(splitVec[j]);
        }
        
        return result;
    }
    
    SchemaItem * AsciiSchemaMapper::initSchemaItem(rowStringParts rawData) {
        int currCol;
        int lineNum;
        int prefixLen;
        int dataLen;
        bool isHeaderItem = 0;
        bool isConstantItem = 0;
        bool isStorageItem = 0;
        
        //check if this is a header item or not
        //normal data columns should start with 'D' and then their column number - however to remain backward compatible, old way is still assumed
        //header items start with 'H' in their column number
        //constant items start with 'C' in their column number
        if(rawData.colNumString.at(0) == 'D' || rawData.colNumString.at(0) == 'd') {
            const char* currStr = rawData.colNumString.c_str();
            currCol = atoi(currStr+1);
        } else if(rawData.colNumString.at(0) == 'H' || rawData.colNumString.at(0) == 'h') {
            isHeaderItem = 1;
            const char* currStr = rawData.colNumString.c_str();
            currCol = atoi(currStr+1);
        } else if (rawData.colNumString.at(0) == 'C' || rawData.colNumString.at(0) == 'c') {
            isConstantItem = 1;
            isStorageItem = 0;
            const char* currStr = rawData.colNumString.c_str();
            currCol = atoi(currStr+1);
        } else if (rawData.colNumString.at(0) == 'S' || rawData.colNumString.at(0) == 's') {
            isConstantItem = 1;
            isStorageItem = 1;
            const char* currStr = rawData.colNumString.c_str();
            currCol = atoi(currStr+1);
        } else {
            currCol = atoi(rawData.colNumString.c_str());
        }
        
        if(currCol < 1) {
            AsciiIngest_error("AsciiSchemaMapper: Format file error: The column number cannot be smaller than 1. Please start numbering with 1.\n");
        }
        
        if(isHeaderItem == 1) {
            vector<string> tmpStringVec;
            boost::split(tmpStringVec, rawData.prefixLen, boost::is_any_of(":"));
            
            //check if this is valid
            if(tmpStringVec.size() != 2 && rawData.sep.compare("''") == 0) {
                AsciiIngest_error("AsciiSchmeaMapper: Format file error: Header info: you need to pass line number and offset together in the format 'lineNum:offset'\n");
            } else if (tmpStringVec.size() == 2) {
                lineNum = atoi(tmpStringVec[0].c_str());
                prefixLen = atoi(tmpStringVec[1].c_str());
            } else {
                lineNum = 0;
                prefixLen = 0;
            }
        } else {
            lineNum = 0;
            prefixLen = atoi(rawData.prefixLen.c_str());
        }
        dataLen = atoi(rawData.dataLen.c_str());
        
        //generate SchemaItem and add to return schema
        //validity of the schema in terms of what is stored in the database
        //will be evaluated at a later stage
        
        AsciiDataObjDesc * currObjDesc = new AsciiDataObjDesc();            
        
        currObjDesc->setOffsetId(currCol);
        currObjDesc->setDataObjDType(convDType(rawData.DType));
        currObjDesc->setLineNum(lineNum);
        currObjDesc->setPrefixSkip(prefixLen);
        currObjDesc->setLenData(dataLen);
        currObjDesc->setSep(rawData.sep);
        currObjDesc->setDataObjName(rawData.schemaColName);
        
        currObjDesc->setIsHeaderItem(isHeaderItem);
        currObjDesc->setIsConstItem(isConstantItem, isStorageItem);
        
        //split the assertConvLists string and add asserter and converters to this data object
        if(rawData.assertConvList.size() > 0) {
            vector<string> splitAssertConvVec;
            boost::split(splitAssertConvVec, rawData.assertConvList, boost::is_any_of(","), boost::token_compress_on);
            
            for(int i=0; i<splitAssertConvVec.size(); i++) {
                boost::to_upper(splitAssertConvVec.at(i));
                
                if(splitAssertConvVec.at(i).substr(0, 4).compare("ASRT") == 0) {
                    currObjDesc->addAssertion(assertFac->getAsserter(splitAssertConvVec.at(i)));
                } else if (splitAssertConvVec.at(i).substr(0, 4).compare("CONV") == 0) {
                    //parse the name of the asserter, extracting the variable list if available
                    boost::xpressive::sregex rex = boost::xpressive::sregex::compile( "(.+)\\(+.*\\)+" ); //should be greedy!!!
                    boost::xpressive::smatch what;
                    if( !regex_match(splitAssertConvVec.at(i), what, rex) ) {
                        //could be that this is not a function, query again
                        rex = boost::xpressive::sregex::compile( "(.+)" );
                        
                        if( !regex_match(splitAssertConvVec.at(i), what, rex) ) {
                            AsciiIngest_error("AsciiSchemaMapper: Format file error: Error in extracting name of conversion.\n");
                        }
                    }
                    
                    currObjDesc->addConverter(convFac->getConverter(what[1].str()));
                } else {
                    AsciiIngest_error("At least one of your assertions or converters you specified in the structure file donot start with ASRT or CONV.\n");
                }
            }
        }
        
        SchemaItem * currItem = new SchemaItem();
        
        //check if this is an empty column or not
        if(rawData.schemaColName.compare("SKIP_THIS_COL") == 0) {
            currItem->setColumnName(EMPTY_SCHEMAITEM_NAME);
        } else {        
            currItem->setColumnName(rawData.schemaColName);
        }
        
        currItem->setColumnDBType(getDBType(rawData.DType));
        currItem->setDataDesc(currObjDesc);
        
        /*cout << currCol << endl;
         cout << DType << endl;
         cout << lineNum << endl;
         cout << prefixLen << endl;
         cout << dataLen << endl;
         cout << sep << endl;
         cout << schemaColName << endl;
         cout << assertConvList << endl;
         cout << convList << endl;*/
        
        return currItem;
    }

    int AsciiSchemaMapper::reparseForVariables(DBDataSchema::Schema * thisSchema, SchemaItem * thisItem, rowStringParts rawData) {
        //uint32_t currAssert = 0;
        uint32_t currConvIdx = 0;
        
        if(rawData.assertConvList.size()  == 0) {
            return 0;
        }
        
        //split the assertConvLists string and add asserter and converters to this data object
        vector<string> splitAssertConvVec;
        boost::split(splitAssertConvVec, rawData.assertConvList, boost::is_any_of(","), boost::token_compress_on);
        
        for(int i=0; i<splitAssertConvVec.size(); i++) {
            string upperString(splitAssertConvVec.at(i));
            boost::to_upper(upperString);
            
            if (upperString.substr(0, 4).compare("CONV") == 0) {
                DBConverter::Converter * currConv = thisItem->getDataDesc()->getConversion(currConvIdx);
                
                if(currConv->getNumParameters() != 0) {
                    boost::xpressive::sregex rex2 = boost::xpressive::sregex::compile( ".*\\((.*)\\)" );
                    boost::xpressive::smatch what2;
                    if( !regex_match(splitAssertConvVec.at(i), what2, rex2) ) {
                        AsciiIngest_error("AsciiSchemaMapper: Format file error: Error in extracting variable string in conversion. Did you realy define variables where there should be some?\n");
                    }
                    
                    string varString(what2[1].str());
                    
                    //parse variable strings for variables
                    vector<string> splitVariablesVec;
                    boost::split(splitVariablesVec, varString, boost::is_any_of(";"), boost::token_compress_on);
                    
                    //check if the right number of variables have been provided
                    if(splitVariablesVec.size() != currConv->getNumParameters()) {
                        printf("Error AsciiSchemaMapper:\n");
                        printf("Error in variables for converter: %s\n", splitAssertConvVec.at(i).c_str());
                        AsciiIngest_error("AsciiSchemaMapper: Error in converter variables. Not the right number of variables provided for this converter.\n");
                    }
                    
                    //start processing the variables, adding them to the converter
                    for(int j=0; j<splitVariablesVec.size(); j++) {
                        string currString(splitVariablesVec.at(j));
                        
                        boost::trim_left(currString);
                        boost::trim_right(currString);
                        
                        //if first character is D,H, or C, this is a row as defined in the format file
                        //otherwise this is a constant value
                        if(currString.at(0) == 'D' || currString.at(0) == 'd' ||
                           currString.at(0) == 'H' || currString.at(0) == 'h' ||
                           currString.at(0) == 'C' || currString.at(0) == 'c' ||
                           currString.at(0) == 'S' || currString.at(0) == 's') {
                            
                            const char* currStr = currString.c_str();
                            int currCol = atoi(currStr+1);
                            
                            //look for this column
                            DataObjDesc * currDatObj = NULL;
                            for(int k=0; k<thisSchema->getArrSchemaItems().size(); k++) {
                                if(thisSchema->getArrSchemaItems().at(k)->getDataDesc()->getOffsetId() == currCol) {
                                    currDatObj = thisSchema->getArrSchemaItems().at(k)->getDataDesc();
                                    break;
                                }
                            }
                            
                            //check for recursion!!
                            if(currDatObj == thisItem->getDataDesc()) {
                                AsciiIngest_error("AsciiSchemaMapper: Recursion is NOT allowed in a converter function!\n");
                            }
                            
                            //check if we have found something
                            if(currDatObj == NULL) {
                                AsciiIngest_error("AsciiSchemaMapper: Could not find the row that has been specified in a converter function.\n");
                            }
                            
                            currConv->registerParameter(j, currDatObj);                            
                        } else {
                            //this is a constant value that has been passed to the function. create a dataobject for it
                            
                            AsciiDataObjDesc * currDatObj = new AsciiDataObjDesc();
                            
                            currDatObj->setOffsetId(j);
                            currDatObj->setDataObjName("CONST_CONV_ITEM");
                            currDatObj->setIsHeaderItem(0);
                            currDatObj->setIsConstItem(1, 0);

                            //crude check if we need a double or an integer:
                            if(currString.find('.') != currString.npos) {
                                //this is a double
                                currDatObj->setDataObjDType(DT_REAL8);
                            } else {
                                //this is an int
                                currDatObj->setDataObjDType(DT_INT8);
                            }

                            //check if this is a string
                            if(currString.find('"') != currString.npos) {
                                //this is a string
                                currDatObj->setDataObjDType(DT_STRING);
                                currString.erase(currString.find('"'), 1);
                                currString.erase(currString.rfind('"'), 1);
                            }

                            currDatObj->setSep(currString);

                            currConv->registerParameter(j, currDatObj);  
                        }
                    }
                }
                
                currConvIdx++;
            } 
        }
        
                
        return 0;
    }
}
