#pragma once

#include "pch.h"
#include "resource.h"

#if __has_include("codegen/NativeOPSQLiteDataTypes.g.h")
  #include "codegen/NativeOPSQLiteDataTypes.g.h"
#endif
// Note: The following lines use Mustache template syntax which will be processed during
// project generation to produce standard C++ code. If existing codegen spec files are found,
// use the actual filename; otherwise use conditional includes.
#include "codegen/NativeOPSQLiteSpec.g.h"

#include "NativeModules.h"

namespace winrt::OpSqlite
{

// See https://microsoft.github.io/react-native-windows/docs/native-platform for help writing native modules

REACT_MODULE(OPSQLite)
struct OPSQLite
{
  // Note: Mustache template syntax below will be processed during project generation
  // to produce standard C++ code based on detected codegen files.
  using ModuleSpec = OpSqliteCodegen::OPSQLiteSpec;

  REACT_INIT(Initialize)
  void Initialize(React::ReactContext const &reactContext) noexcept;

  REACT_GET_CONSTANTS(GetConstants)
  OpSqliteCodegen::OPSQLiteSpec_Constants GetConstants() noexcept;

  REACT_SYNC_METHOD(install)
  bool install() noexcept;

  REACT_SYNC_METHOD(moveAssetsDatabase)
  bool moveAssetsDatabase(std::string name, std::string extension) noexcept;

private:
  React::ReactContext m_context;
};

} // namespace winrt::OpSqlite