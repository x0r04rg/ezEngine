#include <FileservePlugin/FileservePluginPCH.h>

#include <FileservePlugin/Client/FileserveDataDir.h>

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(FileservePlugin, FileservePluginMain)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    ezFileSystem::RegisterDataDirectoryFactory(ezDataDirectory::FileserveType::Factory, 100.0f);

    if (ezStartup::HasApplicationTag("tool") ||
        ezStartup::HasApplicationTag("testframework")) // the testframework configures a fileserve client itself
      return;

    ezFileserveClient* fs = ezFileserveClient::GetSingleton();

    if (fs == nullptr)
    {
      fs = EZ_DEFAULT_NEW(ezFileserveClient);
      EZ_IGNORE_UNUSED(fs);

      // on sandboxed platforms we must go through fileserve, so we enforce a fileserve connection
      // on unrestricted platforms, we use fileserve, if a connection can be established,
      // but if the connection times out, we fall back to regular file accesses
#if EZ_DISABLED(EZ_SUPPORTS_UNRESTRICTED_FILE_ACCESS)
      if (fs->SearchForServerAddress().Failed())
      {
        fs->WaitForServerInfo().IgnoreResult();
      }
#endif
    }
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    if (ezStartup::HasApplicationTag("tool") ||
        ezStartup::HasApplicationTag("testframework"))
      return;

    if (ezFileserveClient::GetSingleton() != nullptr)
    {
      ezFileserveClient* pSingleton = ezFileserveClient::GetSingleton();
      EZ_DEFAULT_DELETE(pSingleton);
    }
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on


EZ_STATICLINK_FILE(FileServePlugin, FileServePlugin_FileservePlugin);
