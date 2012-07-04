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

#include "AsciiDataObjDesc.h"
#include <math.h>
#include <string.h>
#include <iostream>
#include <assert.h>
#include <boost/algorithm/string/replace.hpp>

using namespace std;
using namespace DBDataSchema;

namespace AsciiIngest {
    
    AsciiDataObjDesc::AsciiDataObjDesc() {
        greedySep = 0;
    }
    
    AsciiDataObjDesc::~AsciiDataObjDesc() {

    }

    int AsciiDataObjDesc::getPrefixSkip() {
        return prefixSkip;
    }
    
    void AsciiDataObjDesc::setPrefixSkip(int newPrefixSkip) {
        assert(newPrefixSkip >= 0);
        
        prefixSkip = newPrefixSkip;
    }
    

    int AsciiDataObjDesc::getLenData() {
        return lenData;
    }
    
    void AsciiDataObjDesc::setLenData(int newLenData) {
        assert(newLenData >= 0);
        
        lenData = newLenData;
    }

    int AsciiDataObjDesc::getLineNum() {
        return lineNum;
    }
    
    void AsciiDataObjDesc::setLineNum(int newLineNum) {
        assert(newLineNum >= 0);
        
        lineNum = newLineNum;
    }

    char * AsciiDataObjDesc::getSep() {
        return sep;
    }
    
    void AsciiDataObjDesc::setSep(string newSep) {
        boost::replace_all(newSep, "\\n", "\n");
        boost::replace_all(newSep, "\\t", "\t");
        boost::replace_all(newSep, "\\r", "\r");
        boost::replace_all(newSep, "\\\\", "\\");
        boost::replace_all(newSep, "\\0", "\0");
        
        strncpy(sep, newSep.c_str(), 10);        
    }
    
    void AsciiDataObjDesc::setGreedy(int newGreedy) {
        assert(newGreedy == 0 || newGreedy == 1);
        greedySep = newGreedy;
    }
    
    int AsciiDataObjDesc::getGreedy() {
        return greedySep;
    }

}