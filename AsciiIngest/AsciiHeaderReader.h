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

#include <Schema.h>
#include <HeaderReader.h>

#ifndef ASCIIHEADERREADER_H_
#define ASCIIHEADERREADER_H_

namespace AsciiIngest {
    
    class AsciiHeaderReader : public DBReader::HeaderReader  {
        
        int numHeaderRows;
        
    public:
        AsciiHeaderReader();
        ~AsciiHeaderReader();

        /*! \brief reads the specified header
         item from the header.
         \param DBDataSchema::DataObjDesc * thisItem: a header data object
         \return void*: a pointer to the data
         
         This developer implemented method looks for the specified data object item in the header
         and returns its value.*/
		virtual void* getItem(DBDataSchema::DataObjDesc * thisItem);
        
        void setNumHeaderRows(int newNumHeaderRows);

        int getNumHeaderRows();
    };
    
}

#endif /* ASCIIHEADERREADER_H_ */
