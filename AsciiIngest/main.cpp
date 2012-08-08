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


#include <iostream>
#include "AsciiSchemaMapper.h"
#include "AsciiReader.h"
#include <Schema.h>
#include <DType.h>
#include <DBIngestor.h>
#include <DBAdaptorsFactory.h>
#include <AsserterFactory.h>
#include <ConverterFactory.h>
#include <boost/program_options.hpp>
#ifndef _WIN32
#include <stdint.h>
#else
#include <stdint_win.h>
#endif


using namespace AsciiIngest;
using namespace std;
namespace po = boost::program_options;

int main (int argc, char * argv[])
{

    string structFile;
    string dbase;
    string table;
    string data;
    string comment;
    string system;
    string socket;
    string user;
    string pwd;
    string port;
    string host;
    string path;
    uint32_t bufferSize;
    uint32_t outputFreq;
    uint32_t disableKeys;
    uint32_t enableKeys;
    bool usrVal;
    bool greedyDelim;
    bool isDryRun;

    DBServer::DBAbstractor * dbServer;
    DBIngest::DBIngestor * asciiIngestor;
    DBServer::DBAdaptorsFactory adaptorFac;
    
    //build database string
    string dbSystemDesc = "database system to use (";
    
#ifdef DB_SQLITE3
    dbSystemDesc.append("sqlite3, ");
#endif
    
#ifdef DB_MYSQL
    dbSystemDesc.append("mysql, ");
#endif
    
#ifdef DB_ODBC
    dbSystemDesc.append("unix_sqlsrv_odbc, ");
    dbSystemDesc.append("sqlsrv_odbc, ");
    dbSystemDesc.append("sqlsrv_odbc_bulk, ");
    dbSystemDesc.append("cust_odbc, ");
    dbSystemDesc.append("cust_odbc_bulk, ");
#endif
    
    dbSystemDesc.append(") - [default: mysql]");
    
    po::options_description progDesc("AsciiIngest - THE ingestor for ascii data into databases\n\nAsciiIngest [OPTIONS] [structFile] [dataFile]\n\nCommand line options:");
    
    progDesc.add_options()
                ("help,?", "output help")
                ("structFile", po::value<string>(&structFile), "path to the structure file")
                ("data,d", po::value<string>(&data), "datafile to ingest")
                ("system,s", po::value<string>(&system)->default_value("mysql"), dbSystemDesc.c_str())
                ("bufferSize,B", po::value<uint32_t>(&bufferSize)->default_value(128), "ingest buffer size (will be reduced to sytem maximum if needed) [default: 128]")
                ("outputFreq,F", po::value<uint32_t>(&outputFreq)->default_value(100000), "number of rows after which a performance measurement is output [default: 100000]")
                ("dbase,D", po::value<string>(&dbase)->default_value(""), "name of the database where the data is added to (where applicable)")
                ("table,T", po::value<string>(&table)->default_value(""), "name of the table where the data is added to")
                ("comment,C", po::value<string>(&comment)->default_value("#"), "comment identifier - [default: #]")
                ("socket,S", po::value<string>(&socket)->default_value(""), "socket to use for database access (where applicable)")
                ("user,U", po::value<string>(&user)->default_value(""), "user name (where applicable")
                ("pwd,P", po::value<string>(&pwd)->default_value(""), "password (where applicable")
                ("port,O", po::value<string>(&port)->default_value("3306"), "port to use for database access (where applicable) [default: 3306 (mysql)]")
                ("host,H", po::value<string>(&host)->default_value("localhost"), "host to use for database access (where applicable) [default: localhost]")
                ("path,p", po::value<string>(&path)->default_value(""), "path to a database file (mainly for sqlite3, where applicable)")
                ("disableKeys", po::value<uint32_t>(&disableKeys)->default_value(0), "disables all keys/indexes on the table where you ingest to [default: 0]")
                ("enableKeys", po::value<uint32_t>(&enableKeys)->default_value(0), "reenables the keys/indexes on the table. WARNING: This can take a long time, so only do this if you know what you are doing! [default: 0]")
                ("usrVal, V", po::value<bool>(&usrVal)->default_value(1), "double check whether any assumption I take, or things that might go wrong, are as intended by the user or not [default: 1]")
                ("greedyDelim, G", po::value<bool>(&greedyDelim)->default_value(1), "should the delimiters be treated as greedy or non-greedy? in other words: should multiple equal delimiters be merged into one or not? [default: 1]")
                ("isDryRun", po::value<bool>(&isDryRun)->default_value(0), "should this run be carried out as a dry run (no data added to database)? [default: 0]")
                ;
    
    
    po::positional_options_description posDesc;
    posDesc.add("structFile", 1);
    posDesc.add("data", 1);
    
    //read out the options
    po::variables_map varMap;
    po::store(po::command_line_parser(argc, argv).options(progDesc).positional(posDesc).run(), varMap);
    po::notify(varMap);
    
    if(varMap.count("help") || varMap.count("?") || data.length() == 0 || structFile.length() == 0) {
        cout << progDesc;
        return EXIT_SUCCESS;
    }
    
    /*data = "/Users/adrian/Documents/eScience/tmp/z_0.000_dishalum";
    structFile = "/Users/adrian/Documents/eScience/tmp/dump_z_0.000_dishalum.frm";
    dbase = "tmp1";
    table = "catalog";
    user = "root";*/
    
    cout << "You have entered the following parameters:" << endl;
    cout << "Structure file: " << structFile << endl;
    cout << "Data file: " << data << endl;
    cout << "DB system: " << system << endl;
    cout << "Buffer size: " << bufferSize << endl;
    cout << "Performance output frequency: " << outputFreq << endl;
    cout << "Database name: " << dbase << endl;
    cout << "Table name: " << table << endl;
    cout << "Comment marker: " << comment << endl;
    cout << "Socket: " << socket << endl;
    cout << "User: " << user << endl;
    if(pwd.compare("") == 0) {
        cout << "Password not given" << endl;
    } else {
        cout << "Password given" << endl;
    }
    cout << "Port: " << port << endl;
    cout << "Host: " << host << endl;
    cout << "Path: " << path << endl << endl;
    if(disableKeys != 0) {
        cout << "You DISABLE all the keys on this table" << endl;
    } 
    if(enableKeys != 0) {
        cout << "You REENABLE all the keys on this table" << endl;
    } 
    if(usrVal == 1) {
        cout << "You are asking the user to validate header information" << endl;
    }
    if(isDryRun == 1) {
        cout << "THIS IS A DRY RUN! No data will be added to the database!" << endl;
    }
    
    DBAsserter::AsserterFactory * assertFac = new DBAsserter::AsserterFactory;
    DBConverter::ConverterFactory * convFac = new DBConverter::ConverterFactory;
    AsciiSchemaMapper * thisSchemaMapper = new AsciiSchemaMapper(structFile, assertFac, convFac);
    
    DBDataSchema::Schema * thisSchema;
    thisSchema = thisSchemaMapper->generateSchema(dbase, table);
    thisSchemaMapper->setGreedyTo(greedyDelim, thisSchema);
    
    //now setup the file reader
    AsciiReader * thisReader = new AsciiReader(data, thisSchemaMapper->getNumHeaderRows());
    thisReader->getHeader()->setAskUserToValidateRead(usrVal);
    thisReader->setCommentFlag(comment);
    thisReader->setLineParser(thisSchema);
    thisReader->setAskUserToValidateEndline(usrVal);
    
    dbServer = adaptorFac.getDBAdaptors(system);
    asciiIngestor = new DBIngest::DBIngestor(thisSchema, thisReader, dbServer);
    asciiIngestor->setAskUserToValidateRead(usrVal);
    asciiIngestor->setUsrName(user);
    asciiIngestor->setPasswd(pwd);
    
    if(system.compare("mysql") == 0) {
        asciiIngestor->setSocket(socket);
        asciiIngestor->setPort(port);
        asciiIngestor->setHost(host);
    } else if (system.compare("sqlite3") == 0) {
        asciiIngestor->setHost(path);
    } else if (system.compare("unix_sqlsrv_odbc") == 0) {
        asciiIngestor->setSocket("DRIVER=FreeTDS;TDS_Version=7.0;");
        //asciiIngestor->setSocket("DRIVER=SQL Server Native Client 10.0;");
        asciiIngestor->setPort(port);
        asciiIngestor->setHost(host);
    } else if (system.compare("sqlsrv_odbc") == 0) {
        asciiIngestor->setSocket("DRIVER=SQL Server Native Client 10.0;");
        asciiIngestor->setPort(port);
        asciiIngestor->setHost(host);
    } else if (system.compare("sqlsrv_odbc_bulk") == 0) {
        //TESTS ON SQL SERVER SHOWED THIS IS VERY SLOW. BUT NO CLUE WHY, DID NOT BOTHER TO LOOK AT PROFILER YET
        asciiIngestor->setSocket("DRIVER=SQL Server Native Client 10.0;");
        asciiIngestor->setPort(port);
        asciiIngestor->setHost(host);
    }  else if (system.compare("cust_odbc") == 0) {
        asciiIngestor->setSocket(socket);
        asciiIngestor->setPort(port);
        asciiIngestor->setHost(host);
    } else if (system.compare("cust_odbc_bulk") == 0) {
        //TESTS ON SQL SERVER SHOWED THIS IS VERY SLOW. BUT NO CLUE WHY, DID NOT BOTHER TO LOOK AT PROFILER YET
        asciiIngestor->setSocket(socket);
        asciiIngestor->setPort(port);
        asciiIngestor->setHost(host);
    }  
    
    //now ingest data after setup
    asciiIngestor->setPerformanceMeter(outputFreq);
    asciiIngestor->setIsDryRun(isDryRun);
    asciiIngestor->ingestData(bufferSize);    
    
    delete thisSchemaMapper;
    delete thisSchema;
    delete assertFac;
    delete convFac;
    
    return 0;
}

