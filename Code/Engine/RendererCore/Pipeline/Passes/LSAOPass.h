#pragma once

#include <RendererCore/../../../Data/Base/Shaders/Pipeline/LSAOConstants.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererFoundation/RendererFoundationDLL.h>

/// \brief Defines the depth compare function to be used to decide sample weights.
struct EZ_RENDERERCORE_DLL ezLSAODepthCompareFunction
{
  using StorageType = ezUInt8;

  enum Enum
  {
    Depth,                   ///< A hard cutoff function between the linear depth values. Samples with an absolute distance greater than
                             ///< ezLSAOPass::SetDepthCutoffDistance are ignored.
    Normal,                  ///< Samples that are on the same plane as constructed by the center position and normal will be weighted higher than those samples that
                             ///< are above or below the plane.
    NormalAndSampleDistance, ///< Same as Normal, but if two samples are tested, their distance to the center position is is inversely multiplied as
                             ///< well, giving closer matches a higher weight.
    Default = NormalAndSampleDistance
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_RENDERERCORE_DLL, ezLSAODepthCompareFunction);

/// Screen space ambient occlusion using "line sweep ambient occlusion" by Ville Timonen
///
/// Resources:
/// Use in Quantum Break: http://wili.cc/research/quantum_break/SIGGRAPH_2015_Remedy_Notes.pdf
/// Presentation slides EGSR: http://wili.cc/research/lsao/EGSR13_LSAO.pdf
/// Paper: http://wili.cc/research/lsao/lsao.pdf
///
/// There are a few adjustments and own ideas worked into this implementation.
/// The biggest change probably is that pixels in the gather pass compute their target linesample arithmetically instead of relying on lookups.
class EZ_RENDERERCORE_DLL ezLSAOPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezLSAOPass, ezRenderPipelinePass);

public:
  ezLSAOPass();
  ~ezLSAOPass();

  virtual bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;
  virtual void InitRenderPipelinePass(const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

  virtual void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  virtual void ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  virtual ezResult Serialize(ezStreamWriter& inout_stream) const override;
  virtual ezResult Deserialize(ezStreamReader& inout_stream) override;

  ezUInt32 GetLineToLinePixelOffset() const { return m_iLineToLinePixelOffset; }
  void SetLineToLinePixelOffset(ezUInt32 uiPixelOffset);
  ezUInt32 GetLineSamplePixelOffset() const { return m_iLineSamplePixelOffsetFactor; }
  void SetLineSamplePixelOffset(ezUInt32 uiPixelOffset);

  // Factor used for depth cutoffs (determines when a depth difference is too large to be considered)
  float GetDepthCutoffDistance() const;
  void SetDepthCutoffDistance(float fDepthCutoffDistance);

  // Determines how quickly the occlusion falls of.
  float GetOcclusionFalloff() const;
  void SetOcclusionFalloff(float fFalloff);


protected:
  /// Destroys all GPU data that might have been created in in SetupLineSweepData
  void DestroyLineSweepData();
  void SetupLineSweepData(const ezVec3I32& imageResolution);


  void AddLinesForDirection(const ezVec3I32& imageResolution, const ezVec2I32& sampleDir, ezUInt32 lineIndex, ezDynamicArray<LineInstruction>& outinLineInstructions, ezUInt32& outinTotalNumberOfSamples);

  ezRenderPipelineNodeInputPin m_PinDepthInput;
  ezRenderPipelineNodeOutputPin m_PinOutput;

  ezConstantBufferStorageHandle m_hLineSweepCB;

  bool m_bSweepDataDirty = true;
  bool m_bConstantsDirty = true;

  /// Output of the line sweep pass.
  ezGALBufferHandle m_hLineSweepOutputBuffer;
  ezGALBufferUnorderedAccessViewHandle m_hLineSweepOutputUAV;
  ezGALBufferResourceViewHandle m_hLineSweepOutputSRV;

  /// Structured buffer containing instructions for every single line to trace.
  ezGALBufferHandle m_hLineInfoBuffer;
  ezGALBufferResourceViewHandle m_hLineSweepInfoSRV;

  /// Total number of lines to be traced.
  ezUInt32 m_uiNumSweepLines = 0;

  ezInt32 m_iLineToLinePixelOffset = 2;
  ezInt32 m_iLineSamplePixelOffsetFactor = 1;
  float m_fOcclusionFalloff = 0.2f;
  float m_fDepthCutoffDistance = 4.0f;

  ezEnum<ezLSAODepthCompareFunction> m_DepthCompareFunction;
  bool m_bDistributedGathering = true;

  ezShaderResourceHandle m_hShaderLineSweep;
  ezShaderResourceHandle m_hShaderGather;
  ezShaderResourceHandle m_hShaderAverage;
};
