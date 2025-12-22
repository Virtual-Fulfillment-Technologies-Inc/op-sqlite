// Windows-specific glue file to ensure C++ opsqlite code is compiled into the DLL
// This file forces the linker to include all the C++ implementation code

// Include all C++ implementation files directly
// This ensures all symbols are available within the DLL without needing dllexport

#pragma warning(push)
#pragma warning(disable: 4005) // macro redefinition

// Undefine Windows macros that conflict with C++ code
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

#include "..\..\cpp\OPSqlite.cpp"
#include "..\..\cpp\bridge.cpp"
#include "..\..\cpp\DBHostObject.cpp"
#include "..\..\cpp\DumbHostObject.cpp"
#include "..\..\cpp\SmartHostObject.cpp"
#include "..\..\cpp\PreparedStatementHostObject.cpp"
#include "..\..\cpp\OPThreadPool.cpp"
#include "..\..\cpp\utils.cpp"

#pragma warning(pop)
