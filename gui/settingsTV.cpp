//---------------------------------------------------------------------------------------------------------
// Copyright(c) <2016> Venu P.Gopal
//
// Original Author : Venu P.Gopal (www.linkedin.com/in/venupgopal/ )
// Original Publication Date : December 23, 2016
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files(the "Software"), to deal in the Software 
// without restriction, including without limitation the rights to use, copy, modify, merge, 
// publish, distribute, sublicense, and / or sell copies of the Software, and to permit 
// persons to whom the Software is furnished to do so, subject to the following conditions :
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// The above copyright notice and this permission notice must be included in all copies or 
// substantial portions of the Software.Further credits may be added following this entire notice.
//---------------------------------------------------------------------------------------------------------


#include <iostream>
#include <fstream>							// for file I/O
#include <sstream>							// for stringstream
#include <cassert>							// for assert()
#include <vector>							// for STL vector

#include <string>							// for STL string class
#include <map>								// for STL map class

#include "settingsTV.h"			// Class definition

using namespace std;

namespace SCL {

//----------------------------------------------------------------------------------------------------------------
// Internally we store the settings data in a map (STL class) consisting of a key-data pair.
// The key is a string that is a concatenation of the section name and the parameter name
// The key is used to a) order the data, b) to retrieve the data
// The data consists of 
//		section name, parameter name, type, a string representation of the data

//----------------------------------------------------------------------------------------------------------------
// Data Store for the settings.  Uses the STL map class.
// We chose to not expose this to use via the header file.

class settingDataType
{
public:
	string		settingSectionName;
	string		settingParamName;
	string		settingType;
	string		settingValueString;
	string		settingComment;
};
typedef std::map<string, settingDataType> settingsDataStoreType;	// type of object we will store settings in

//----------------------------------------------------------------------------------------------------------------
// Private functions used only within the scope of this .cpp file
// We chose to make them static (private) in order to avoid having them exposed to user via the class header

static bool valueStringMatchesType(const string& type, const string& valueString);
static bool sanitizeValueString(const string& type, string& valueString);
static settingDataType* getData(settingsDataStoreType& settingsDataStore, const string& sectionName, const string& paramName);
static uint32_t STRLEN(string& str);																	//length of string
static void DEL_LEADING_TRAILING_SPACES_TABS(string& str);												// delete leading and trailing spaces

//----------------------------------------------------------------------------------------------------------------
//Constructor for the class

settingsTV::settingsTV()
{
	settingsData_p = new settingsDataStoreType;							// create data store (map object) for settings
}

//----------------------------------------------------------------------------------------------------------------
//Initialize settings - Application initializes the default starting values by calling this

int settingsTV::init(uint32_t count, setting_initializer initializers[])
{
	settingsDataStoreType& settingsDataStore = *((settingsDataStoreType*)settingsData_p);		// get a reference to our settings data store

	for (uint32_t i = 0; i < count; i++)
	{
		string sectionNameS = initializers[i].sectionName;
		string paramNameS = initializers[i].paramName;
		string typeS = initializers[i].type;
		string valueS = initializers[i].valueString;
		string commentS = ""; if (initializers[i].comment != NULL) commentS = initializers[i].comment;		// comment, if any

		// Delete leading and trailing spaces and tabs
		DEL_LEADING_TRAILING_SPACES_TABS(sectionNameS);
		DEL_LEADING_TRAILING_SPACES_TABS(paramNameS);
		DEL_LEADING_TRAILING_SPACES_TABS(typeS);
		// cout << "init " << sectionNameS << " " << paramNameS << " " << typeS << " " << valueS << " " << commentS << endl;

		// Make sure parameter name is not an empty string (section name may be an empty string)
		if (paramNameS.length() == 0)
		{
			cout << "settingsTV class: Invalid Initializer |" << sectionNameS << " " << paramNameS << " " << typeS << " " << valueS << "| : " <<
				"Parameter name cannot be an empty string. " << endl;
			return -1;
		}

		// Make sure user has not entered invalid values
		if (!valueStringMatchesType(typeS, valueS))
		{
			cout << "settingsTV class: Invalid Initializer |" << sectionNameS << " " << paramNameS << " " << typeS << " " << valueS << "| : " <<
				"Invalid value for type " << initializers[i].type << endl;
			return -2;
		}

		// Sanitize the value string (remove illegal characters etc. example "section1" "Param1" "ST_UI32" "12345abc"
		sanitizeValueString(typeS, valueS);
		// Create a temp settingDataType object and populate from initializers element
		settingDataType sd;
		sd.settingSectionName = sectionNameS;	sd.settingParamName = paramNameS;
		sd.settingType = typeS;			sd.settingValueString = valueS;
		sd.settingComment = commentS;

		settingsDataStore[sd.settingSectionName + "." + sd.settingParamName] = sd;						// add the setting to the settings data store
	}

	//cout << "Number of items in data store = " << settingsDataStore.size() << endl;
	//settingsDataStoreType::iterator dsIt;
	//for (dsIt = settingsDataStore.begin(); dsIt != settingsDataStore.end(); dsIt++)
	//{
	//	cout << "Setting Datastore: " << dsIt->first << "\t" << (dsIt->second).settingSectionName << "\t" << (dsIt->second).settingParamName 
	//		 << "\t" << (dsIt->second).settingType << "\t" << (dsIt->second).settingValueString << endl;
	//}

	return 0;
}


//----------------------------------------------------------------------------------------------------------------
// Destructor

settingsTV::~settingsTV()
{
	delete settingsData_p;
}

// get member functions

uint32_t settingsTV::getBoolSetting(const string& sectionName, const string& paramName, bool& result)		// Get the value of a BOOL     setting 
{
	result = false;																							// init result
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_BOOL) return 10;							// Verify setting exists; verify that it is the right type
	const string& str = data_p->settingValueString;
	if ((str == "FALSE") || (str == "false")) { result = false; return 0; }
	else if ((str == "TRUE") || (str == "true")) { result = true; return 0; }
	else return 20;
	return 0;
}

uint32_t settingsTV::getUI8Setting(const string& sectionName, const string& paramName, uint8_t& result)		// Get the value of a uint8_t setting
{
	result = 0;																								// init result
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_UI8) return 10;								// Verify setting exists; verify that it is the right type
	result = (uint8_t)atoll((data_p->settingValueString).c_str());
	return 0;
}

uint32_t settingsTV::getUI16Setting(const string& sectionName, const string& paramName, uint16_t& result)	// Get the value of a uint16_t setting
{
	result = 0;																								// init result
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_UI16) return 10;							// Verify setting exists; verify that it is the right type
	result = (uint16_t)atoll((data_p->settingValueString).c_str());
	return 0;
}

uint32_t settingsTV::getUI32Setting(const string& sectionName, const string& paramName, uint32_t& result)	// Get the value of a uint32_t setting
{
	result = 0;																								// init result
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_UI32) return 10;							// Verify setting exists; verify that it is the right type
	result = (uint32_t)atoll((data_p->settingValueString).c_str());
	return 0;
}

uint32_t settingsTV::getUI64Setting(const string& sectionName, const string& paramName, uint64_t& result)	// Get the value of a uint64_t setting
{
	result = 0;																								// init result
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_UI64) return 10;							// Verify setting exists; verify that it is the right type
	istringstream ss; ss.str(data_p->settingValueString); ss >> result;
	return 0;
}

uint32_t settingsTV::getI8Setting(const string& sectionName, const string& paramName, int8_t& result)		// Get the value of a int8_t setting
{
	result = 0;																								// init result
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_I8) return 10;								// Verify setting exists; verify that it is the right type
	result = (int8_t)atoll((data_p->settingValueString).c_str());
	return 0;
}

uint32_t settingsTV::getI16Setting(const string& sectionName, const string& paramName, int16_t& result)		// Get the value of a int16_t setting 
{
	result = 0;																								// init result
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_I16) return 10;								// Verify setting exists; verify that it is the right type
	result = (int16_t)atoll((data_p->settingValueString).c_str());
	return 0;
}

uint32_t settingsTV::getI32Setting(const string& sectionName, const string& paramName, int32_t& result)		// Get the value of a int32_t setting
{
	result = 0;																								// init result
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_I32) return 10;								// Verify setting exists; verify that it is the right type
	result = (int32_t)atoll((data_p->settingValueString).c_str());
	return 0;
}

uint32_t settingsTV::getI64Setting(const string& sectionName, const string& paramName, int64_t& result)		// Get the value of a int64_t setting
{
	result = 0;																								// init result
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_I64) return 10;								// Verify setting exists; verify that it is the right type
	result = atoll((data_p->settingValueString).c_str());
	return 0;
}

uint32_t settingsTV::getDblSetting(const string& sectionName, const string& paramName, double& result)		// Get the value of a double setting 
{
	result = 0.0;																							// init result
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_DBL) return 10;								// Verify setting exists; verify that it is the right type
	result = atof((data_p->settingValueString).c_str());
	return 0;
}

uint32_t settingsTV::getStrSetting(const string& sectionName, const string& paramName, string& result)		// Get the value of a string setting
{
	result = "";																							// init result
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_STR) return 10;								// Verify setting exists; verify that it is the right type
	result = data_p->settingValueString;
	return 0;
}

// set member functions

uint32_t settingsTV::setBoolSetting(const string& sectionName, const string& paramName, bool     value)		// set the value of a BOOL     setting (returns errorcode if error)
{
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_BOOL) return 10;							// Verify setting exists; verify that it is the right type
	if (value) data_p->settingValueString = string("true"); else data_p->settingValueString = string("false");
	return 0;
}

uint32_t settingsTV::setUI8Setting(const string& sectionName, const string& paramName, uint8_t  value)		// set the value of a uint8_t  setting (returns errorcode if error)
{
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_UI8) return 10;								// Verify setting exists; verify that it is the right type
	data_p->settingValueString = to_string(value);															// std::to_string
	return 0;
}

uint32_t settingsTV::setUI16Setting(const string& sectionName, const string& paramName, uint16_t value)		// set the value of a uint16_t setting (returns errorcode if error)
{
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_UI16) return 10;							// Verify setting exists; verify that it is the right type
	data_p->settingValueString = to_string(value);															// std::to_string
	return 0;
}

uint32_t settingsTV::setUI32Setting(const string& sectionName, const string& paramName, uint32_t value)		// set the value of a uint32_t setting (returns errorcode if error)
{
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_UI32) return 10;							// Verify setting exists; verify that it is the right type
	data_p->settingValueString = to_string(value);															// std::to_string
	return 0;
}

uint32_t settingsTV::setUI64Setting(const string& sectionName, const string& paramName, uint64_t value)		// set the value of a uint64_t setting (returns errorcode if error)
{
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_UI64) return 10;							// Verify setting exists; verify that it is the right type
	data_p->settingValueString = to_string(value);															// std::to_string
	return 0;
}

uint32_t settingsTV::setI8Setting(const string& sectionName, const string& paramName, int8_t   value)		// set the value of a int8_t   setting (returns errorcode if error)
{
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_I8) return 10;								// Verify setting exists; verify that it is the right type
	data_p->settingValueString = to_string(value);															// std::to_string
	return 0;
}

uint32_t settingsTV::setI16Setting(const string& sectionName, const string& paramName, int16_t  value)		// set the value of a int16_t  setting (returns errorcode if error)
{
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_I16) return 10;								// Verify setting exists; verify that it is the right type
	data_p->settingValueString = to_string(value);															// std::to_string
	return 0;
}

uint32_t settingsTV::setI32Setting(const string& sectionName, const string& paramName, int32_t  value)		// set the value of a int32_t  setting (returns errorcode if error)
{
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_I32) return 10;								// Verify setting exists; verify that it is the right type
	data_p->settingValueString = to_string(value);															// std::to_string
	return 0;
}

uint32_t settingsTV::setI64Setting(const string& sectionName, const string& paramName, int64_t  value)		// set the value of a int64_t  setting (returns errorcode if error)
{
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_I64) return 10;								// Verify setting exists; verify that it is the right type
	data_p->settingValueString = to_string(value);															// std::to_string
	return 0;
}

uint32_t settingsTV::setDblSetting(const string& sectionName, const string& paramName, double   value)		// set the value of a double   setting (returns errorcode if error)
{
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_DBL) return 10;								// Verify setting exists; verify that it is the right type
	std::ostringstream ss; ss.precision(17); ss << value;
	data_p->settingValueString = ss.str();																	// cannot se t precision if we use std::to_string
	return 0;
}

uint32_t settingsTV::setStrSetting(const string& sectionName, const string& paramName, const string& value)	// set the value of a string   setting (returns errorcode if error)
{
	settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sectionName, paramName);	// get data for this setting
	if (data_p == NULL) return 1; if (data_p->settingType != ST_STR) return 10;								// Verify setting exists; verify that it is the right type
	data_p->settingValueString = value;																		// std::to_string
	return 0;
}

// File I/O functions

// Save the settings to a file.  Returns 0 if success, errorcode if failure.

SettingsExportErrType settingsTV::saveSettingsToFile(const string& pathname, bool overWriteIfExists)
{
	//cout << "Saving settings to file " << pathname << "(overWriteIfExists=" << overWriteIfExists << ")" << endl;

	if (!overWriteIfExists) {
		if (ifstream(pathname)) { return SERC_FILE_EXISTS; }
	}

	ofstream settingsFile;
	settingsFile.open(pathname, ios::out | ios::trunc);
	if (!settingsFile.is_open()) { cout << "Error opening file \"" << pathname << "\". " << endl; settingsFile.close(); return SERC_FILE_CANNOT_OPEN; }

	settingsDataStoreType& settingsDataStore = *((settingsDataStoreType*)settingsData_p);		// get a reference to our settings data store
	settingsDataStoreType::iterator dsIt;

	// find max length of parameterName so we can right pad to the max length 
	uint32_t paramMaxLen = 0, typeMaxLen = 0, valueMaxLen = 0;
	for (dsIt = settingsDataStore.begin(); dsIt != settingsDataStore.end(); dsIt++)
	{
		string str = (dsIt->second).settingParamName; if (STRLEN(str) > paramMaxLen) paramMaxLen = STRLEN(str);		// STRLEN supports UTF-8 encoded strings
		str = (dsIt->second).settingType; if (STRLEN(str) > typeMaxLen) typeMaxLen = STRLEN(str);		//
		str = (dsIt->second).settingValueString; if (STRLEN(str) > valueMaxLen) valueMaxLen = STRLEN(str);		// 
	}

	// write to file
	string indent = "    ";
	string currentSection = "";
	for (dsIt = settingsDataStore.begin(); dsIt != settingsDataStore.end(); dsIt++)
	{
		// Create right padded versions of items { paramName, paramType, ParamValue } so that file looks neat
		string paddedParmName = (dsIt->second).settingParamName; while (STRLEN(paddedParmName) < paramMaxLen) paddedParmName += " "; 		// STRLEN supports UTF-8 encoded strings
		string paddedType = (dsIt->second).settingType; while (STRLEN(paddedType) < typeMaxLen) paddedType += " ";
		string paddedValuestr;
		if ((dsIt->second).settingType != ST_STR) { paddedValuestr = (dsIt->second).settingValueString; }
		else																				// string type need additional handling - adding double quotes etc.
		{
			for (size_t i = 0; i < ((dsIt->second).settingValueString).length(); i++)
			{
				char c = (dsIt->second).settingValueString[i];
				if ((c == '\"') || (c == '\\')) paddedValuestr += '\\';						// for strings add backslash for " and \ characters
				paddedValuestr += c;
			}
			paddedValuestr = "\"" + paddedValuestr + "\"";									// for strings, add lead/trail double quotes
		}
		while (STRLEN(paddedValuestr) < (valueMaxLen + 2)) paddedValuestr += " ";			// The additional 2 is to allow for the addition of double quotes for string types
		// Write section name out if new section
		if ((dsIt->second).settingSectionName != currentSection)
		{
			currentSection = (dsIt->second).settingSectionName;
			settingsFile << "[" << currentSection << "]" << endl;
		}
		if (currentSection == "") paddedParmName += indent;									// empty section params need additional padding since non-empty section params are indented
		else settingsFile << indent;
		settingsFile << paddedParmName << " " << paddedType << " ";
		settingsFile << paddedValuestr;														// we attempt to pad tp fixed field width - but many international chars are non-fixed width
		settingsFile << " // " << (dsIt->second).settingComment;
		settingsFile << endl;
	}

	settingsFile.close();
	return SERC_SUCCESS;
}

	// Import settings from a file 
SettingsImportErrType settingsTV::importSettingsFromFile(const string& pathname, SettingsImportType importType, bool& settingsVersionDifferent, string& errorString, import_callback callback_func)
{
#define CloseFileAndReturn(rc) { settingsFile.close(); return rc; }

	settingsVersionDifferent = false;													// initializer
	ifstream settingsFile;
	settingsFile.open(pathname, ios::in);
	if (!settingsFile.is_open()) { errorString = "Cannot open file";  CloseFileAndReturn(SIRC_CANNOT_OPEN); }

	// We need to parse each line into tokens and then process the tokens into a setting
	string currentSection = "";																							// Initial secion
	uint32_t lineNum = 0; string line; size_t lineLen; size_t nextCharPos = 0; /*vector<string> tokens;*/
	while (getline(settingsFile, line))								// getline works fine with UTF-8 encoded strings since UTF-8 does not use ASCII code bytes
	{
		lineNum++;																										// line number
		// In case a BOM (Byte Order Marker) was added to the front of the file by a text editor, we need to eat it
		if (lineNum == 1)
		{
			if (line.length() >= 3) if ((line[0] == '\xef') && (line[1] == '\xbb') && (line[2] == '\xbf')) line = line.substr(3);	// UTF-8 BOM found, just eat it & continue
			if (line.length() >= 2) if ((line[0] == '\xfe') && (line[1] == '\xff')) 												// UTF-16 Big Endian BOM found
				{ stringstream ss; ss << "Unsupported BOM at beginning of file on line " << lineNum; errorString = ss.str(); CloseFileAndReturn(SIRC_UNSUPPORTED_BOM); }
			if (line.length() >= 2) if ((line[0] == '\xff') && (line[1] == '\xfe')) 												// UTF-16 Little Endian BOM found
				{ stringstream ss; ss << "Unsupported BOM at beginning of file on line " << lineNum; errorString = ss.str(); CloseFileAndReturn(SIRC_UNSUPPORTED_BOM); }
			if (line.length() >= 4) if ((line[0] == '\x00') && (line[1] == '\x00') && (line[2] == '\xfe') && (line[3] == '\xff')) 	// UTF-32 Big Endian BOM found
				{ stringstream ss; ss << "Unsupported BOM at beginning of file on line " << lineNum; errorString = ss.str(); CloseFileAndReturn(SIRC_UNSUPPORTED_BOM); }
			if (line.length() >= 4) if ((line[0] == '\xff') && (line[1] == '\xfe') && (line[2] == '\x00') && (line[3] == '\x00')) 	// UTF-32 Little Endian BOM found
				{ stringstream ss; ss << "Unsupported BOM at beginning of file on line " << lineNum; errorString = ss.str(); CloseFileAndReturn(SIRC_UNSUPPORTED_BOM); }
		}
		string firstToken = ""; string secondToken = ""; string thirdToken = ""; string commentToken = "";
		lineLen = line.length(); nextCharPos = 0;
		while (nextCharPos < lineLen)																					// Parse this line
		{
			while ((line[nextCharPos] == ' ') || (line[nextCharPos] == '\t') && (nextCharPos < lineLen)) nextCharPos++;	// eat any spaces, tabs
			if ((line[nextCharPos] == '/') && (line[nextCharPos + 1] == '/'))
			{
				nextCharPos += 2; while ((line[nextCharPos] == ' ') || (line[nextCharPos] == '\t')) nextCharPos++;		// eat any spaces, tabs before comment
				commentToken = line.substr(nextCharPos);																		// rest is comment
				break;
			}
			string token = "";
			if (line[nextCharPos] != '\"')																				// does not start with double quotes
			{
				while ((line[nextCharPos] != ' ') && (line[nextCharPos] != '\t') && (nextCharPos < lineLen))
				{
					token += line[nextCharPos]; nextCharPos++;
				}
			}
			else																										// starts with double quotes
			{
				nextCharPos++;																							// skip starting double quote
				while ((line[nextCharPos] != '\"') && (nextCharPos < lineLen))
				{
					if (line[nextCharPos] == '\\') { nextCharPos++; if (nextCharPos >= lineLen) break; }				// allow backslash escaped chars like "a\"b", "a\\b"
					token += line[nextCharPos]; nextCharPos++;
				}
				if (line[nextCharPos] == '\"') nextCharPos++;															// skip ending double quote
			}
			if (token == "") break;
			if (firstToken == "") firstToken = token;
			else if (secondToken == "") secondToken = token;
			else if (thirdToken == "") thirdToken = token;																// dump any tokens after third
		}

		// now we have from one to three tokens plus any comment
		//cout << line << endl;
		//cout << "T1=" << firstToken << " T2=" << secondToken << " T3=" << thirdToken << " Comment=" << commentToken << endl;
		if (firstToken == "") { continue; }																				// blank line or just comment line
		if (firstToken[0] == '[') {
			if (firstToken.length() < 3) { stringstream ss; ss << "Invalid Section identifier on line " << lineNum; errorString = ss.str(); CloseFileAndReturn(SIRC_SECTION_INVALID_FORMAT); }
			if ((firstToken[firstToken.length() - 1]) != ']') { stringstream ss; ss << "Missing ']' on line " << lineNum; errorString = ss.str(); CloseFileAndReturn(SIRC_SECTION_INVALID_FORMAT); }
			currentSection = firstToken.substr(1, firstToken.length() - 2);
			//cout << "NEW SECTION " << currentSection << endl;
			continue;
		}

		// determine sectionName, paramName, Type, Value and Comment
		if ((firstToken == "") || (secondToken == "") || (thirdToken == ""))
		{
			stringstream ss; ss << "Incorrect number of items on line " << lineNum; errorString = ss.str(); CloseFileAndReturn(SIRC_PARAMETER_INVALID_FORMAT);
		}
		bool unrecongnizedType = false;																					// if we encounter unrecognized types 
		// store into temporary settingDataType
		string sSectionName = currentSection;	string sParamName = firstToken;
		string sType = secondToken;	string sVal = thirdToken;
		string sComment = commentToken;

		// Check for invalid types
		if ((sType != ST_BOOL) &&
			(sType != ST_UI8) && (sType != ST_UI16) && (sType != ST_UI32) && (sType != ST_UI64) &&
			(sType != ST_I8) && (sType != ST_I16) && (sType != ST_I32) && (sType != ST_I64) &&
			(sType != ST_DBL) &&
			(sType != ST_STR))
		{
			stringstream ss; ss << "Invalid type on line " << lineNum; errorString = ss.str(); CloseFileAndReturn(SIRC_PARAMETER_INVALID_FORMAT);
		}
		if ((sType == ST_BOOL) && (sVal != "false") && (sVal != "FALSE") && (sVal != "true") && (sVal != "TRUE"))
		{
			stringstream ss; ss << "Invalid value for BOOL type on line " << lineNum; errorString = ss.str(); CloseFileAndReturn(SIRC_PARAMETER_INVALID_FORMAT);
		}

		// We could add a check for numeric types to be digits only - not done

		// Sanitize the value strings in case they were edited manually
		//cout << "S=" << sSectionName << " P=" << sParamName << " T=" << sType << " V=" << sVal << " C=" << sComment << endl;	// print out tokens pre-sanitized setting
		if (!valueStringMatchesType(sType, sVal))
		{
			stringstream ss; ss << "Invalid value for " << sType << " on line " << lineNum; errorString = ss.str(); CloseFileAndReturn(SIRC_PARAMETER_INVALID_FORMAT);
		}
		sanitizeValueString(sType, sVal);			// remove any extraneous chars etc. from value string
		//cout << "S=" << sSectionName << " P=" << sParamName << " T=" << sType << " V=" << sVal << " C=" << sComment << endl;	// print out sanitized setting

		// Now we do the import
		if ((importType == IMPORT_ALL) || (importType == IMPORT_ALL_EXCEPT_VERSION))
		{
			settingDataType* data_p = getData(*((settingsDataStoreType*)settingsData_p), sSectionName, sParamName);	// get data for this setting in application
			if (data_p == NULL) continue;																			// this setting not in the current program

			if ((importType == IMPORT_ALL_EXCEPT_VERSION) && (sSectionName == "") && (sParamName == ".SettingsVersion"))
			{ 
				/* cout << ".SettingsVersion not imported" << endl;*/ 
				if ((data_p->settingType != sType) || (data_p->settingValueString != sVal)) settingsVersionDifferent = true;
				continue;
			}
			if (data_p->settingType != sType) continue;																// this setting has a new type in the current program
			data_p->settingValueString = sVal;																		// store modified value
		}
		if (importType == IMPORT_USES_CALLBACK)
		{
			if (callback_func == NULL) { stringstream ss; ss << "Application programming error: No callback function provided to process setting from file."; errorString = ss.str(); CloseFileAndReturn(SIRC_CALLBACK_NOT_VALID); }
			uint32_t result = (*callback_func)(sSectionName, sParamName, sType, sVal, sComment);					// call user's callback funtion to process the setting from file
			if (result !=0) { stringstream ss; ss << "Settings import callback failed"; errorString = ss.str(); CloseFileAndReturn(SIRC_CALLBACK_RETURNED_ERROR); }
		}
	}
	CloseFileAndReturn(SIRC_SUCCESS);
}


// Print settings to stdout

void settingsTV::print()
{
	settingsDataStoreType& settingsDataStore = *((settingsDataStoreType*)settingsData_p);		// get a reference to our settings data store
	settingsDataStoreType::iterator dsIt;
	cout << endl << "Settings data: BEGIN --------------------------------" << endl;
	for (dsIt = settingsDataStore.begin(); dsIt != settingsDataStore.end(); dsIt++)
	{
		cout << (dsIt->second).settingSectionName << " " << (dsIt->second).settingParamName << " "
			 << (dsIt->second).settingType << " " ;
		if ((dsIt->second).settingType == ST_STR) cout << "\"";
		cout << (dsIt->second).settingValueString;
		if ((dsIt->second).settingType == ST_STR) cout << "\"";
		cout << " // " << (dsIt->second).settingComment << endl;
	}
	cout << "Settings data: END  ---------------------------------" << endl << endl;
}


//----------------------------------------------------------------------------------------------------------------
// Private functions
//----------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------
// private function to get the data associated with a setting, given its section name and parameter name
// (keyword static - to keep this function private to this file), (keyword const - so caller cannot modify the data)

static settingDataType* getData(settingsDataStoreType& settingsDataStore, const string& sectionName, const string& paramName)
{
	if (paramName.length() == 0) return NULL;											// parameter name cannot be empty string (section name can be)
	if (settingsDataStore.empty()) return NULL;											// there are no settings in data store
	settingsDataStoreType::iterator dsIt;												// iterator for data store map object
	dsIt = settingsDataStore.find(sectionName + "." + paramName);
	if (dsIt == settingsDataStore.end()) return NULL;									// couldn't find this setting
	return &(dsIt->second);																// return pointer to setting's data
	return NULL;
}


//----------------------------------------------------------------------------------------------------------------
// private function to validate value string against its type (used to check initializer entries)

static bool valueStringMatchesType(const string& type, const string& valueString)
{
	int64_t li;

	if (type == ST_BOOL) {
		if ((valueString != "true") && (valueString != "false") &&
			(valueString != "TRUE") && (valueString != "FALSE")) return false;
		return true; 
	};
	// We should probably add a check here to ensure that numeric types have only digits - not done yet
	if (type == ST_UI8 ) { li = stoll(valueString, NULL, 10); if ((li < 0) || (li > 255))        return false; return true; }
	if (type == ST_UI16) { li = stoll(valueString, NULL, 10); if ((li < 0) || (li > 65535))      return false; return true; }
	if (type == ST_UI32) { li = stoll(valueString, NULL, 10); if ((li < 0) || (li > 0xFFFFFFFF)) return false; return true; }
	if (type == ST_UI64) { return true; }			// no checking done in this case
	if (type == ST_I8  ) { li = stoll(valueString, NULL, 10); if ((li < -128) || (li > 127))     return false; return true; }
	if (type == ST_I16 ) { li = stoll(valueString, NULL, 10); if ((li < -32768) || (li > 32767)) return false; return true; }
	if (type == ST_I32 ) { li = stoll(valueString, NULL, 10); if ((li < INT32_MIN) || (li > INT32_MAX)) return false; return true; }
	if (type == ST_I64 ) { return true; }			// no checking done in this case
	if (type == ST_DBL ) { return true; }			// no checking done in this case
	if (type == ST_STR ) { return true; }			// no checking done in this case

	return false;										// unknown type
}

static bool sanitizeValueString(const string& type, string& valueString)
{
	if (type == ST_BOOL)
	{
		if ((valueString != "false") && (valueString != "FALSE") && (valueString != "true") && (valueString != "TRUE"))
			valueString = "false";
		if (valueString != "FALSE") valueString = "false";
		if (valueString != "TRUE") valueString = "true";
	}

	if (type == ST_UI8 ) { uint8_t  val = (uint8_t ) atoll(valueString.c_str()); stringstream ss; ss << (int)val; valueString = ss.str(); }
	if (type == ST_UI16) { uint16_t val = (uint16_t) atoll(valueString.c_str()); stringstream ss; ss << val; valueString = ss.str(); }
	if (type == ST_UI32) { uint32_t val = (uint32_t) atoll(valueString.c_str()); stringstream ss; ss << val; valueString = ss.str(); }
	if (type == ST_UI64) { uint64_t val; istringstream ssi; ssi.str(valueString); ssi >> val; stringstream ss; ss << val; valueString = ss.str(); }

	if (type == ST_I8  ) { int8_t   val = (int8_t  ) atoll(valueString.c_str()); stringstream ss; ss << (int)val; valueString = ss.str(); }
	if (type == ST_I16 ) { int16_t  val = (int16_t ) atoll(valueString.c_str()); stringstream ss; ss << val; valueString = ss.str(); }
	if (type == ST_I32 ) { int32_t  val = (int32_t ) atoll(valueString.c_str()); stringstream ss; ss << val; valueString = ss.str(); }
	if (type == ST_I64 ) { int64_t  val = atoll(valueString.c_str()); stringstream ss; ss << val; valueString = ss.str(); }

	if (type == ST_DBL ) { double   val = atof(valueString.c_str()); stringstream ss; ss.precision(17); ss << val; valueString = ss.str(); }

	if (type == ST_STR ) {}	// Nothing needs to be done

	return true;
}



// gets length of strings, including UTF-8 encoded strings
static uint32_t STRLEN(string& str)
{
#ifdef  _VG_UTF_8_ENCODED_STRING_SUPPORTED_
	string::iterator strBegin = str.begin();
	string::iterator strEnd   = str.end();
	return utf8::distance(strBegin, strEnd);							// calc length in UTF-8 characters
#else
	return str.length();
#endif
}

static void DEL_LEADING_TRAILING_SPACES_TABS(string& str)
{
#ifdef  _VG_UTF_8_ENCODED_STRING_SUPPORTED_
	// NOTE: Following code is not very performance efficient, but works
	if (str == "") return;
	while (((str.at(0) == ' ') || (str.at(0) == '\t')) && (str.length() > 0))	// spaces and tabs are one char UTF-8s
		str = str.substr(1, str.length() - 1);
	if (str == "") return;
	while (true)
	{
		string::iterator itBegin = str.begin();
		string::iterator itEnd   = str.end();
		string::iterator itPrev  = str.end();
		if (itPrev == itBegin) break;
		uint32_t codePoint = utf8::prior(itPrev, itBegin);						// moves pointer to previous utf8 char and gets unicode number for that char
		int8_t utf8char[5] = { 0,0,0,0,0 }; utf8::append(codePoint, utf8char);	// encode the codepoint into a UTF-8 char
		if ((utf8char[0] == ' ') || (utf8char[0] == '\t')) str = str.substr(0,str.length()-1); 	// delete it (we can do this since space and tab are one char UTF-8s)
		else break;
	}
#else
	size_t firstNST = str.find_first_not_of(" \t");						// first char that is not space or tab
	if (firstNST == string::npos) { str = ""; return; } 					// no char other than spaces and tabs
	// some char other than space or tab is there
	size_t lastNST = str.find_last_not_of(" \t");						// last char that is not space ot tab
	size_t range = lastNST - firstNST + 1;
	str = str.substr(firstNST, range);							
#endif
}

//----------------------------------------------------------------------------------------------------------------
}	// namespace
//----------------------------------------------------------------------------------------------------------------



