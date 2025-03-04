#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <GuiFoundation/GuiFoundationDLL.h>

class ezEventTrack;

class EZ_GUIFOUNDATION_DLL ezEventTrackControlPointData : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezEventTrackControlPointData, ezReflectedClass);

public:
  ezTime GetTickAsTime() const { return ezTime::MakeFromSeconds(m_iTick / 4800.0); }
  void SetTickFromTime(ezTime time, ezInt64 iFps);
  const char* GetEventName() const { return m_sEvent.GetData(); }
  void SetEventName(const char* szSz) { m_sEvent.Assign(szSz); }

  ezInt64 m_iTick; // 4800 ticks per second
  ezHashedString m_sEvent;
};

class EZ_GUIFOUNDATION_DLL ezEventTrackData : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezEventTrackData, ezReflectedClass);

public:
  ezInt64 TickFromTime(ezTime time) const;
  void ConvertToRuntimeData(ezEventTrack& out_result) const;

  ezUInt16 m_uiFramesPerSecond = 60;
  ezDynamicArray<ezEventTrackControlPointData> m_ControlPoints;
};

class EZ_GUIFOUNDATION_DLL ezEventSet
{
public:
  bool IsModified() const { return m_bModified; }

  const ezSet<ezString>& GetAvailableEvents() const { return m_AvailableEvents; }

  void AddAvailableEvent(ezStringView sEvent);

  ezResult WriteToDDL(const char* szFile);
  ezResult ReadFromDDL(const char* szFile);

private:
  bool m_bModified = false;
  ezSet<ezString> m_AvailableEvents;
};
