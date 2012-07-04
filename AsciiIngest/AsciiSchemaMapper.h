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

#include <SchemaDataMapGenerator.h>
#include <DBType.h>
#include <AsserterFactory.h>
#include <ConverterFactory.h>
#include <string>
#include <fstream>
#include <stdio.h>

#ifndef ASCIISCHEMAMAPPER_H_
#define ASCIISCHEMAMAPPER_H_

namespace AsciiIngest {
    
    typedef struct {
        std::string colNumString;
        std::string DType;
        std::string prefixLen;
        std::string dataLen;
        std::string sep;
        std::string schemaColName;
        std::string assertConvList;
    } rowStringParts;
    
    class AsciiSchemaMapper : public DBDataSchema::SchemaDataMapGenerator  {
        
    private:
        std::ifstream fileStream;
        
        std::string formatFileName;
        
        int numCols;
        int numHeaderRows;
        
        std::string getNextLineInFile();
        DBDataSchema::DBType getDBType(std::string fromDType);

        /*! \brief reads the format file and saves the strings in a rowStringParts structure
         \return a rowStringParts structure for the current row
         
         This method parses the current row from the format file, starts to divide the strings
         into the various components and saves the strings in a rowStringParts structure. These
         can be reparsed again and again if needed, without the need to go through the format file
         again.*/
        rowStringParts getNextRowStringParts();

        /*! \brief creats a not fully instantiated schema from rowStringParts data
         \param rowStringParts rawData: data for this row
         return a loosely initialised schema item for this row (see text below)
         
         This method will parse a rowStringParts object and create an initialised SchemaItem
         object out of it. If this row does not need any asserter/converter that takes variables,
         the SchemaItem produced by this method can be used without further processing. However if
         any asserter/converter takes arguments, reprocession of the Schema is needed, to properly
         link up all the SchemaItems. Therefore initialisation of SchemaItems is a two step process.
         First use initSchemaItem, then apply reparseForVariables.*/
        DBDataSchema::SchemaItem * initSchemaItem(rowStringParts rawData);
        
        /*! \brief sets up all the asserter/converters with arguments, linking them properly with the data
         \param DBDataSchema::Schema * thisSchema: the complete schema needed for linking the data
         \param SchemaItem * thisItem: current schema item where converter arguments need to be set up
         \param rowStringParts rawData: original data from the structure file for this row
         
         This method sets up all the links to the data neede for any converter with arguments. For this, a reparsing
         of the original input string from the format file is needed. The data is searched for in the Schema and the
         links are properly set. If a constant value is provided, the corresponding constant SchemaItems are allocated
         and linked to the converter.*/
        int reparseForVariables(DBDataSchema::Schema * thisSchema, DBDataSchema::SchemaItem * thisItem, rowStringParts rawData);
        

        DBAsserter::AsserterFactory * assertFac;
        DBConverter::ConverterFactory * convFac;
        
    public:
        AsciiSchemaMapper();
        AsciiSchemaMapper(std::string formatFile, DBAsserter::AsserterFactory * assertFac, DBConverter::ConverterFactory * convFac);
        ~AsciiSchemaMapper();
        
        DBDataSchema::Schema * generateSchema(std::string dbName, std::string tblName);
        
        void setFormatFileName(std::string newFormatFile);
        std::string getFormatFileName();
        
        void setAssertionFactory(DBAsserter::AsserterFactory * newFactory);
        DBAsserter::AsserterFactory * getAssertionFactory();

        void setConverterFactory(DBConverter::ConverterFactory * newFactory);
        DBConverter::ConverterFactory * getConverterFactory();

        int getNumCols();
        
        int getNumHeaderRows();
        
        /*! \brief sets all the object descriptors greedy mode
         \param int greedy: 1 = greedy, 0 = non-greedy
         \param DBDataSchema::Schema * inThisSchema: a schema to change the greedy state in
         
         This method will set all the greedy states in the schema. WARNING! Only works for
         AsciiDataObjDesc objects!*/
        void setGreedyTo(int greedy, DBDataSchema::Schema * inThisSchema);
    };
    
}

#endif /* ASCIISCHEMAMAPPER_H_ */
