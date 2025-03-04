#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>
#include <RendererCore/RenderContext/Implementation/RenderContextStructs.h>
#include <RendererCore/RendererCoreDLL.h>
#include <RendererFoundation/RendererFoundationDLL.h>
#include <Texture/Image/Image.h>
#include <Texture/ezTexFormat/ezTexFormat.h>

class EZ_RENDERERCORE_DLL ezTextureResourceLoader : public ezResourceTypeLoader
{
public:
  struct LoadedData
  {
    LoadedData()
      : m_Reader(&m_Storage)
    {
    }

    ezContiguousMemoryStreamStorage m_Storage;
    ezMemoryStreamReader m_Reader;
    ezImage m_Image;

    bool m_bIsFallback = false;
    ezTexFormat m_TexFormat;
  };

  virtual ezResourceLoadData OpenDataStream(const ezResource* pResource) override;
  virtual void CloseDataStream(const ezResource* pResource, const ezResourceLoadData& loaderData) override;
  virtual bool IsResourceOutdated(const ezResource* pResource) const override;

  static ezResult LoadTexFile(ezStreamReader& inout_stream, LoadedData& ref_data);
  static void WriteTextureLoadStream(ezStreamWriter& inout_stream, const LoadedData& data);
};
