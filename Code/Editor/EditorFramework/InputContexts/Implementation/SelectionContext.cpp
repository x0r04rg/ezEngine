#include <EditorFramework/EditorFrameworkPCH.h>

#include <EditorFramework/Assets/AssetCurator.h>
#include <EditorFramework/Assets/AssetDocument.h>
#include <EditorFramework/DocumentWindow/EngineDocumentWindow.moc.h>
#include <EditorFramework/DocumentWindow/EngineViewWidget.moc.h>
#include <EditorFramework/Gizmos/GizmoBase.h>
#include <EditorFramework/InputContexts/SelectionContext.h>
#include <Foundation/Utilities/GraphicsUtils.h>

ezSelectionContext::ezSelectionContext(ezQtEngineDocumentWindow* pOwnerWindow, ezQtEngineViewWidget* pOwnerView, const ezCamera* pCamera)
{
  m_pCamera = pCamera;

  SetOwner(pOwnerWindow, pOwnerView);

  m_hMarqueeGizmo.ConfigureHandle(nullptr, ezEngineGizmoHandleType::LineBox, ezColor::CadetBlue, ezGizmoFlags::ShowInOrtho | ezGizmoFlags::OnTop);
  pOwnerWindow->GetDocument()->AddSyncObject(&m_hMarqueeGizmo);
}

ezSelectionContext::~ezSelectionContext()
{
  // if anyone is registered for object picking, tell them that nothing was picked,
  // so that they reset their state
  if (m_PickObjectOverride.IsValid())
  {
    m_PickObjectOverride(nullptr);
    ResetPickObjectOverride();
  }
}

void ezSelectionContext::SetPickObjectOverride(ezDelegate<void(const ezDocumentObject*)> pickOverride)
{
  m_PickObjectOverride = pickOverride;
  GetOwnerView()->setCursor(Qt::CrossCursor);
}

void ezSelectionContext::ResetPickObjectOverride()
{
  if (m_PickObjectOverride.IsValid())
  {
    m_PickObjectOverride.Invalidate();
    GetOwnerView()->unsetCursor();
  }
}

ezEditorInput ezSelectionContext::DoMousePressEvent(QMouseEvent* e)
{
  if (e->button() == Qt::MouseButton::LeftButton)
  {
    const ezObjectPickingResult& res = GetOwnerView()->PickObject(e->pos().x(), e->pos().y());

    if (res.m_PickedOther.IsValid())
    {
      auto pSO = GetOwnerWindow()->GetDocument()->FindSyncObject(res.m_PickedOther);

      if (pSO != nullptr)
      {
        if (pSO->GetDynamicRTTI()->IsDerivedFrom<ezGizmoHandle>())
        {
          ezGizmoHandle* pGizmoHandle = static_cast<ezGizmoHandle*>(pSO);
          ezGizmo* pGizmo = pGizmoHandle->GetOwnerGizmo();

          if (pGizmo)
          {
            pGizmo->ConfigureInteraction(pGizmoHandle, m_pCamera, res.m_vPickedPosition, m_vViewport);
            return pGizmo->MousePressEvent(e);
          }
        }
      }
    }

    m_Mode = Mode::Single;

    if (m_bPressedSpace && !m_PickObjectOverride.IsValid())
    {
      m_uiMarqueeID += 23;
      m_vMarqueeStartPos.Set(e->pos().x(), e->pos().y(), 0.01f);

      // no modifier -> add, CTRL -> remove
      m_Mode = e->modifiers().testFlag(Qt::ControlModifier) ? Mode::MarqueeRemove : Mode::MarqueeAdd;
      MakeActiveInputContext();

      if (m_Mode == Mode::MarqueeAdd)
        m_hMarqueeGizmo.SetColor(ezColor::LightSkyBlue);
      else
        m_hMarqueeGizmo.SetColor(ezColor::PaleVioletRed);

      return ezEditorInput::WasExclusivelyHandled;
    }
  }

  return ezEditorInput::MayBeHandledByOthers;
}

ezEditorInput ezSelectionContext::DoMouseReleaseEvent(QMouseEvent* e)
{
  if (e->button() == Qt::MouseButton::MiddleButton)
  {
    if (e->modifiers() & Qt::KeyboardModifier::ControlModifier)
    {
      const ezObjectPickingResult& res = GetOwnerView()->PickObject(e->pos().x(), e->pos().y());

      OpenDocumentForPickedObject(res);
    }
  }

  if (e->button() == Qt::MouseButton::LeftButton)
  {
    if (m_Mode == Mode::Single)
    {
      const ezObjectPickingResult& res = GetOwnerView()->PickObject(e->pos().x(), e->pos().y());

      const bool bToggle = (e->modifiers() & Qt::KeyboardModifier::ControlModifier) != 0;
      const bool bDirect = (e->modifiers() & Qt::KeyboardModifier::AltModifier) != 0;
      SelectPickedObject(res, bToggle, bDirect);

      DoFocusLost(false);

      // we handled the mouse click event
      // but this is it, we don't stay active
      return ezEditorInput::WasExclusivelyHandled;
    }

    if (m_Mode == Mode::MarqueeAdd || m_Mode == Mode::MarqueeRemove)
    {
      SendMarqueeMsg(e, (m_Mode == Mode::MarqueeAdd) ? 1 : 2);

      const bool bPressedSpace = m_bPressedSpace;
      DoFocusLost(false);
      m_bPressedSpace = bPressedSpace;
      return ezEditorInput::WasExclusivelyHandled;
    }
  }

  return ezEditorInput::MayBeHandledByOthers;
}


void ezSelectionContext::OpenDocumentForPickedObject(const ezObjectPickingResult& res) const
{
  if (!res.m_PickedComponent.IsValid())
    return;

  auto* pDocument = GetOwnerWindow()->GetDocument();

  const ezDocumentObject* pPickedComponent = pDocument->GetObjectManager()->GetObject(res.m_PickedComponent);

  for (auto pDocMan : ezDocumentManager::GetAllDocumentManagers())
  {
    if (ezAssetDocumentManager* pAssetMan = ezDynamicCast<ezAssetDocumentManager*>(pDocMan))
    {
      if (pAssetMan->OpenPickedDocument(pPickedComponent, res.m_uiPartIndex).Succeeded())
      {
        return;
      }
    }
  }

  GetOwnerWindow()->ShowTemporaryStatusBarMsg("Could not open a document for the picked object");
}

void ezSelectionContext::SelectPickedObject(const ezObjectPickingResult& res, bool bToggle, bool bDirect) const
{
  if (res.m_PickedObject.IsValid())
  {
    auto* pDocument = GetOwnerWindow()->GetDocument();
    const ezDocumentObject* pObject = pDocument->GetObjectManager()->GetObject(res.m_PickedObject);
    if (!pObject)
      return;

    if (m_PickObjectOverride.IsValid())
    {
      m_PickObjectOverride(pObject);
    }
    else
    {
      if (bToggle)
        pDocument->GetSelectionManager()->ToggleObject(determineObjectToSelect(pObject, true, bDirect));
      else
        pDocument->GetSelectionManager()->SetSelection(determineObjectToSelect(pObject, false, bDirect));
    }
  }
}

void ezSelectionContext::SendMarqueeMsg(QMouseEvent* e, ezUInt8 uiWhatToDo)
{
  ezVec2I32 curPos;
  curPos.Set(e->pos().x(), e->pos().y());

  ezMat4 mView = m_pCamera->GetViewMatrix();
  ezMat4 mProj;
  m_pCamera->GetProjectionMatrix((float)m_vViewport.x / (float)m_vViewport.y, mProj);

  ezMat4 mViewProj = mProj * mView;
  ezMat4 mInvViewProj = mViewProj;
  if (mInvViewProj.Invert(0.0f).Failed())
  {
    // if this fails, the marquee will not be rendered correctly
    EZ_ASSERT_DEBUG(false, "Failed to invert view projection matrix.");
  }

  const ezVec3 vMousePos(e->pos().x(), e->pos().y(), 0.01f);

  const ezVec3 vScreenSpacePos0(vMousePos.x, vMousePos.y, vMousePos.z);
  const ezVec3 vScreenSpacePos1(m_vMarqueeStartPos.x, m_vMarqueeStartPos.y, m_vMarqueeStartPos.z);

  ezVec3 vPosOnNearPlane0, vRayDir0;
  ezVec3 vPosOnNearPlane1, vRayDir1;
  ezGraphicsUtils::ConvertScreenPosToWorldPos(mInvViewProj, 0, 0, m_vViewport.x, m_vViewport.y, vScreenSpacePos0, vPosOnNearPlane0, &vRayDir0).IgnoreResult();
  ezGraphicsUtils::ConvertScreenPosToWorldPos(mInvViewProj, 0, 0, m_vViewport.x, m_vViewport.y, vScreenSpacePos1, vPosOnNearPlane1, &vRayDir1).IgnoreResult();

  ezTransform t;
  t.SetIdentity();
  t.m_vPosition = ezMath::Lerp(vPosOnNearPlane0, vPosOnNearPlane1, 0.5f);
  t.m_qRotation = ezQuat::MakeFromMat3(m_pCamera->GetViewMatrix().GetRotationalPart());

  // box coordinates in screen space
  ezVec3 vBoxPosSS0 = t.m_qRotation * vPosOnNearPlane0;
  ezVec3 vBoxPosSS1 = t.m_qRotation * vPosOnNearPlane1;

  t.m_qRotation = t.m_qRotation.GetInverse();

  t.m_vScale.x = ezMath::Abs(vBoxPosSS0.x - vBoxPosSS1.x);
  t.m_vScale.y = ezMath::Abs(vBoxPosSS0.y - vBoxPosSS1.y);
  t.m_vScale.z = 0.0f;

  m_hMarqueeGizmo.SetTransformation(t);
  m_hMarqueeGizmo.SetVisible(true);

  {
    ezViewMarqueePickingMsgToEngine msg;
    msg.m_uiViewID = GetOwnerView()->GetViewID();
    msg.m_uiPickPosX0 = (ezUInt16)m_vMarqueeStartPos.x;
    msg.m_uiPickPosY0 = (ezUInt16)m_vMarqueeStartPos.y;
    msg.m_uiPickPosX1 = (ezUInt16)(e->pos().x());
    msg.m_uiPickPosY1 = (ezUInt16)(e->pos().y());
    msg.m_uiWhatToDo = uiWhatToDo;
    msg.m_uiActionIdentifier = m_uiMarqueeID;

    GetOwnerView()->GetDocumentWindow()->GetDocument()->SendMessageToEngine(&msg);
  }
}

ezEditorInput ezSelectionContext::DoMouseMoveEvent(QMouseEvent* e)
{
  if (IsActiveInputContext() && (m_Mode == Mode::MarqueeAdd || m_Mode == Mode::MarqueeRemove))
  {
    SendMarqueeMsg(e, 0xFF);

    return ezEditorInput::WasExclusivelyHandled;
  }
  else
  {
    ezViewHighlightMsgToEngine msg;

    {
      const ezObjectPickingResult& res = GetOwnerView()->PickObject(e->pos().x(), e->pos().y());

      if (res.m_PickedComponent.IsValid())
        msg.m_HighlightObject = res.m_PickedComponent;
      else if (res.m_PickedOther.IsValid())
        msg.m_HighlightObject = res.m_PickedOther;
      else
        msg.m_HighlightObject = res.m_PickedObject;
    }

    GetOwnerWindow()->GetEditorEngineConnection()->SendHighlightObjectMessage(&msg);

    // we only updated the highlight, so others may do additional stuff, if they like
    return ezEditorInput::MayBeHandledByOthers;
  }
}

ezEditorInput ezSelectionContext::DoKeyPressEvent(QKeyEvent* e)
{
  /// \todo Handle the current cursor (icon) across all active input contexts

  if (e->key() == Qt::Key_Space)
  {
    m_bPressedSpace = true;
    return ezEditorInput::MayBeHandledByOthers;
  }

  if (e->key() == Qt::Key_Delete)
  {
    GetOwnerWindow()->GetDocument()->DeleteSelectedObjects();
    return ezEditorInput::WasExclusivelyHandled;
  }

  if (e->key() == Qt::Key_Escape)
  {
    if (m_PickObjectOverride.IsValid())
    {
      m_PickObjectOverride(nullptr);
      ResetPickObjectOverride();
    }
    else
    {
      if (m_Mode == Mode::MarqueeAdd || m_Mode == Mode::MarqueeRemove)
      {
        const bool bPressedSpace = m_bPressedSpace;
        FocusLost(true);
        m_bPressedSpace = bPressedSpace;
      }
      else
      {
        GetOwnerWindow()->GetDocument()->GetSelectionManager()->Clear();
      }
    }

    return ezEditorInput::WasExclusivelyHandled;
  }

  return ezEditorInput::MayBeHandledByOthers;
}

ezEditorInput ezSelectionContext::DoKeyReleaseEvent(QKeyEvent* e)
{
  if (e->key() == Qt::Key_Space)
  {
    m_bPressedSpace = false;
  }

  return ezEditorInput::MayBeHandledByOthers;
}

static const bool IsInSelection(const ezDeque<const ezDocumentObject*>& selection, const ezDocumentObject* pObject, const ezDocumentObject*& out_pParentInSelection, const ezDocumentObject*& out_pParentChild, const ezDocumentObject* pRootObject)
{
  if (pObject == pRootObject)
    return false;

  if (selection.IndexOf(pObject) != ezInvalidIndex)
  {
    out_pParentInSelection = pObject;
    return true;
  }

  const ezDocumentObject* pParent = pObject->GetParent();

  if (IsInSelection(selection, pParent, out_pParentInSelection, out_pParentChild, pRootObject))
  {
    if (out_pParentChild == nullptr)
      out_pParentChild = pObject;

    return true;
  }

  return false;
}

static const ezDocumentObject* GetPrefabParentOrSelf(const ezDocumentObject* pObject)
{
  const ezDocumentObject* pParent = pObject;
  const ezDocument* pDocument = pObject->GetDocumentObjectManager()->GetDocument();
  const auto& metaData = *pDocument->m_DocumentObjectMetaData;

  while (pParent != nullptr)
  {
    {
      const ezDocumentObjectMetaData* pMeta = metaData.BeginReadMetaData(pParent->GetGuid());
      bool bIsPrefab = pMeta->m_CreateFromPrefab.IsValid();
      metaData.EndReadMetaData();

      if (bIsPrefab)
        return pParent;
    }
    pParent = pParent->GetParent();
  }

  return pObject;
}

const ezDocumentObject* ezSelectionContext::determineObjectToSelect(const ezDocumentObject* pickedObject, bool bToggle, bool bDirect) const
{
  auto* pDocument = GetOwnerWindow()->GetDocument();
  const ezDeque<const ezDocumentObject*> sel = pDocument->GetSelectionManager()->GetSelection();

  const ezDocumentObject* pRootObject = pDocument->GetObjectManager()->GetRootObject();

  const ezDocumentObject* pParentInSelection = nullptr;
  const ezDocumentObject* pParentChild = nullptr;

  if (!IsInSelection(sel, pickedObject, pParentInSelection, pParentChild, pRootObject))
  {
    if (bDirect)
      return pickedObject;

    return GetPrefabParentOrSelf(pickedObject);
  }
  else
  {
    if (bToggle)
    {
      // always toggle the object that is already in the selection
      return pParentInSelection;
    }

    if (bDirect)
      return pickedObject;

    if (sel.GetCount() > 1)
    {
      // multi-selection, but no toggle, so we are about to set the selection
      // -> always use the top-level parent in this case
      return GetPrefabParentOrSelf(pickedObject);
    }

    if (pParentInSelection == pickedObject)
    {
      // object itself is in the selection
      return pickedObject;
    }

    if (pParentChild == nullptr)
    {
      return pParentInSelection;
    }

    return pParentChild;
  }
}

void ezSelectionContext::DoFocusLost(bool bCancel)
{
  ezEditorInputContext::DoFocusLost(bCancel);

  m_bPressedSpace = false;
  m_Mode = Mode::None;
  m_hMarqueeGizmo.SetVisible(false);

  if (IsActiveInputContext())
    MakeActiveInputContext(false);
}
