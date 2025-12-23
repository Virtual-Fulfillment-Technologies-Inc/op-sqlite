#include "pch.h"

#include "OpSqlite.h"

namespace winrt::OpSqlite
{

// See https://microsoft.github.io/react-native-windows/docs/native-platform for help writing native modules

void OPSQLite::Initialize(React::ReactContext const &reactContext) noexcept {
  m_context = reactContext;
}

OpSqliteCodegen::OPSQLiteSpec_Constants OPSQLite::GetConstants() noexcept {
  using namespace winrt::Windows::Storage;
  using namespace winrt::Windows::ApplicationModel;

  OpSqliteCodegen::OPSQLiteSpec_Constants constants;

  // iOS constants - return empty strings on Windows
  constants.IOS_DOCUMENT_PATH = "";
  constants.IOS_LIBRARY_PATH = "";

  // Android constants - return empty strings on Windows
  constants.ANDROID_DATABASE_PATH = "";
  constants.ANDROID_FILES_PATH = "";
  constants.ANDROID_EXTERNAL_FILES_PATH = "";

  // Windows-specific paths
  try {
    auto localFolder = ApplicationData::Current().LocalFolder();
    constants.WINDOWS_LOCAL_FOLDER = winrt::to_string(localFolder.Path());

    auto tempFolder = ApplicationData::Current().TemporaryFolder();
    constants.WINDOWS_TEMP_FOLDER = winrt::to_string(tempFolder.Path());

    // Database path - use LocalFolder as the base path
    constants.WINDOWS_DATABASE_PATH = winrt::to_string(localFolder.Path());
  } catch (...) {
    // Fallback to empty strings on error
    constants.WINDOWS_LOCAL_FOLDER = "";
    constants.WINDOWS_TEMP_FOLDER = "";
    constants.WINDOWS_DATABASE_PATH = "";
  }

  return constants;
}

bool OPSQLite::install() noexcept {
  using namespace winrt::Microsoft::ReactNative;
  using namespace winrt::Windows::Storage;

  try {
    // Get JSI Runtime from React Context
    facebook::jsi::Runtime* runtime = TryGetOrCreateContextRuntime(m_context);
    if (!runtime) {
      return false;
    }

    // Get CallInvoker for async JS callbacks
    auto callInvoker = m_context.CallInvoker();
    if (!callInvoker) {
      return false;
    }

    // Get base path for databases (LocalFolder)
    auto localFolder = ApplicationData::Current().LocalFolder();
    std::string basePath = winrt::to_string(localFolder.Path());

    // No extensions support for now (pass empty strings)
    std::string crsqlitePath = "";
    std::string sqliteVecPath = "";

    // Call the C++ opsqlite::install function
    opsqlite::install(*runtime, callInvoker,
                     basePath.c_str(),
                     crsqlitePath.c_str(),
                     sqliteVecPath.c_str());

    return true;
  } catch (...) {
    return false;
  }
}

bool OPSQLite::moveAssetsDatabase(std::string name, std::string extension) noexcept {
  using namespace winrt::Windows::Storage;
  using namespace winrt::Windows::ApplicationModel;

  try {
    // Construct full filename
    std::string filename = name;
    if (!extension.empty()) {
      filename += "." + extension;
    }

    // Get source path from app package installation folder
    auto installedLocation = Package::Current().InstalledLocation();
    auto sourceFileTask = installedLocation.GetFileAsync(winrt::to_hstring(filename));
    auto sourceFile = sourceFileTask.get();

    // Get destination folder (LocalFolder for databases)
    auto destFolder = ApplicationData::Current().LocalFolder();

    // Check if file already exists in destination
    try {
      auto existingFileTask = destFolder.GetFileAsync(winrt::to_hstring(filename));
      auto existingFile = existingFileTask.get();
      // File exists - don't overwrite, return success
      return true;
    } catch (...) {
      // File doesn't exist, proceed with copy
    }

    // Copy file from package to LocalFolder
    auto copyTask = sourceFile.CopyAsync(destFolder, winrt::to_hstring(filename));
    copyTask.get();

    return true;
  } catch (...) {
    return false;
  }
}

} // namespace winrt::OPSQlite