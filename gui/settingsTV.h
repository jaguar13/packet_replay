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





// Standard C++ headers
#include <string>					// STL string class


#ifdef _VG_INTERNATIONAL_
// We do not need to do anything special to support Unicode; the strings may be ANSI or UTF-8 encoded Unicode - this class does not care.
#endif

// From the perspective of the user of the SCL (settingsTV) library, a setting has
//		a section name,
//		a parameter name,
//		a type (uint32_t, string etc.) - the type is specified during initialization only
//		a value (initially populated via a C string, subsequently (possibly) modified by reading a settings file
//			subsequently (possibly) modified by the user changing the setting
// For internal representation, see the .cpp file.

using namespace std;

namespace SCL {			// SCL = Settings Class Library

// Initializer structrure used during initialization
// UTF-8 encoded unicode may be used in section names, param names and value strings but they may have to be declared
// using hex representation of chars - example "Sec\xe0\xb4\x95tion4"

// For program upgrades that involve the import of old settings, it may be useful to define a Settings Version, with "." as the section name.
// FOr example Section = ".", ParamName "SettingVersion" ST_STR "1.00.00.00"

struct setting_initializer {
	const char* sectionName;
	const char* paramName;
	const char* type;
	const char* valueString;
	const char* comment;		// description or comment about setting
};

// Settings types
#define ST_BOOL "BOOL"
#define ST_UI8  "UI8"
#define ST_UI16 "UI16"
#define ST_UI32 "UI32"
#define ST_UI64 "UI64"
#define ST_I8   "I8"
#define ST_I16  "I16"
#define ST_I32  "I32"
#define ST_I64  "I64"
#define ST_DBL  "DBL"		// should be 64-bit on most platforms
#define ST_STR  "STR"

typedef enum SettingsImportTypes				// enum used in importSettingsFile() function
{ 
		// IMPORT_ALL just updates the current settings with settings found in the file that match
		// the Section name, Parameter name an Parameter type of a current setting.
		// Settings in the file that do not match the Section name, Parameter name an Parameter type of a current setting
		// are ignored.
		// There is no notion of a settings version - the return value of settingsVersionDifferent arg is always false.
	IMPORT_ALL,	
		// IMPORT_ALL_EXCEPT_VERSION works the same as IMPORT_ALL with the following exception.
		// It is aware of a 'settings version' setting { Section name = "", parameter name = ".SettingsVersion" }
		// This setting is used by the application to store the version of the settings (settings schema version)
		// (usually in string form).
		// It will not update that setting in the application to prevent the current application 'settings version'
		// from being over-written with the (possibly older) version from the file. It does not check that the 
		// file settings version is newer than the application settings version.
		// If the settings version in the file is different than the current application version,
		// then the return value of settingsVersionDifferent arg is set to true. 
		// This can be used, for example, to upgrade the settings file to the current version.
		IMPORT_ALL_EXCEPT_VERSION,
		// IMPORT_USES_CALLBACK is used in more sophisticated scenarios where the application needs to migrate
		// settings from a different version to the current application version.  For every setting in the file,
		// an application callback function is called to process the setting.  
		// Application should store a 'settings version' setting to facilitate upgrades, for example using a 
		// setting such as { Section name = "", parameter name = ".SettingsVersion", type = ST_STR }
		// It is left up to the application on how each setting in the file is handled (including the 'settings version' setting).
		// The return value of settingsVersionDifferent arg is always false
								
	IMPORT_USES_CALLBACK,

} SettingsImportType;

typedef enum SettingsImportErrTypes				// Error codes returned by the importSettingsFromFile() function
{
	SIRC_SUCCESS = 0,							// successfully imported settings
	SIRC_CANNOT_OPEN,							// Could not open file for read
	SIRC_UNSUPPORTED_BOM,						// File has a BOM at beginning that is not UTF-8
	SIRC_SECTION_INVALID_FORMAT,				// Section identifier in file has an invalid format
	SIRC_PARAMETER_INVALID_FORMAT,				// Parameter setting in file has an invalid format
	SIRC_CALLBACK_NOT_VALID,					// Callback function is not provided
	SIRC_CALLBACK_RETURNED_ERROR,				// Callback failed to import setting and returned error (rest of file skipped)
} SettingsImportErrType;

typedef enum SettingsExportErrTypes				// Error codes returned by the importSettingsFromFile() function
{
	SERC_SUCCESS = 0,							// successfully imported settings
	SERC_FILE_EXISTS,							// Over-write flag is set to false and file already exists
	SERC_FILE_CANNOT_OPEN,						// File cannot be opened for writing
} SettingsExportErrType;


class settingsTV
{
public:

	// constants

	// Functions

	settingsTV();																			// Constructor
	int init(uint32_t count, setting_initializer intializers[]);							// Initialize settings - should be called once during program initialization
	~settingsTV();																			// Destructor

	// get functions

	uint32_t getBoolSetting(const string& sectionName, const string& paramName, bool    & result);		// Get the value of a BOOL     setting (return errorcode if error)
	uint32_t getUI8Setting (const string& sectionName, const string& paramName, uint8_t & result);		// Get the value of a uint8_t  setting (return errorcode if error)
	uint32_t getUI16Setting(const string& sectionName, const string& paramName, uint16_t& result);		// Get the value of a uint16_t setting (return errorcode if error)
	uint32_t getUI32Setting(const string& sectionName, const string& paramName, uint32_t& result);		// Get the value of a uint32_t setting (return errorcode if error)
	uint32_t getUI64Setting(const string& sectionName, const string& paramName, uint64_t& result);		// Get the value of a uint64_t setting (return errorcode if error)
	uint32_t getI8Setting  (const string& sectionName, const string& paramName, int8_t  & result);		// Get the value of a int8_t   setting (return errorcode if error)
	uint32_t getI16Setting (const string& sectionName, const string& paramName, int16_t & result);		// Get the value of a int16_t  setting (return errorcode if error)
	uint32_t getI32Setting (const string& sectionName, const string& paramName, int32_t & result);		// Get the value of a int32_t  setting (return errorcode if error)
	uint32_t getI64Setting (const string& sectionName, const string& paramName, int64_t & result);		// Get the value of a int64_t  setting (return errorcode if error)
	uint32_t getDblSetting (const string& sectionName, const string& paramName, double  & result);		// Get the value of a double   setting (return errorcode if error)
	uint32_t getStrSetting (const string& sectionName, const string& paramName, string  & result);		// Get the value of a string   setting (return errorcode if error)
																							
	// set functions

	uint32_t setBoolSetting(const string& sectionName, const string& paramName, bool     value);		// set the value of a BOOL     setting (returns errorcode if error)
	uint32_t setUI8Setting (const string& sectionName, const string& paramName, uint8_t  value);		// set the value of a uint8_t  setting (returns errorcode if error)
	uint32_t setUI16Setting(const string& sectionName, const string& paramName, uint16_t value);		// set the value of a uint16_t setting (returns errorcode if error)
	uint32_t setUI32Setting(const string& sectionName, const string& paramName, uint32_t value);		// set the value of a uint32_t setting (returns errorcode if error)
	uint32_t setUI64Setting(const string& sectionName, const string& paramName, uint64_t value);		// set the value of a uint64_t setting (returns errorcode if error)
	uint32_t setI8Setting  (const string& sectionName, const string& paramName, int8_t   value);		// set the value of a int8_t   setting (returns errorcode if error)
	uint32_t setI16Setting (const string& sectionName, const string& paramName, int16_t  value);		// set the value of a int16_t  setting (returns errorcode if error)
	uint32_t setI32Setting (const string& sectionName, const string& paramName, int32_t  value);		// set the value of a int32_t  setting (returns errorcode if error)
	uint32_t setI64Setting (const string& sectionName, const string& paramName, int64_t  value);		// set the value of a int64_t  setting (returns errorcode if error)
	uint32_t setDblSetting (const string& sectionName, const string& paramName, double   value);		// set the value of a double   setting (returns errorcode if error)
	uint32_t setStrSetting (const string& sectionName, const string& paramName, const string& value);	// set the value of a string   setting (returns errorcode if error)

	// File Output

	SettingsExportErrType saveSettingsToFile(const string& pathname, bool overWriteIfExists);	// Save settings to given path/file

	// File Input

		// signature of user provided callback function to import a setting from a file into user's program
	typedef uint32_t (*import_callback) (const string& section_name,
		const string& param_name, const string& param_type, const string& param_value_str, const string& param_comment);
		// Import settings from given path/file.  Set import_callback to NULL if not using callback.
	SettingsImportErrType importSettingsFromFile(const string& pathname, SettingsImportType importType, bool& settingsVersionDifferent, string& errorString, import_callback);

	// Misc functions

	void print();	
													// prints out settings to stdout
private:

	void* settingsData_p;							// pointer to internal data store

};


}	// namespace end
