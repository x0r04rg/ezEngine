#include <GameComponentsPlugin/GameComponentsPCH.h>


EZ_STATICLINK_LIBRARY(GameComponentsPlugin)
{
  if (bReturn)
    return;

  EZ_STATICLINK_REFERENCE(GameComponentsPlugin_Animation_Implementation_CreatureCrawlComponent);
  EZ_STATICLINK_REFERENCE(GameComponentsPlugin_Animation_Implementation_MoveToComponent);
  EZ_STATICLINK_REFERENCE(GameComponentsPlugin_Debugging_Implementation_LineToComponent);
  EZ_STATICLINK_REFERENCE(GameComponentsPlugin_Effects_Shake_Implementation_CameraShakeComponent);
  EZ_STATICLINK_REFERENCE(GameComponentsPlugin_Effects_Shake_Implementation_CameraShakeVolumeComponent);
  EZ_STATICLINK_REFERENCE(GameComponentsPlugin_Gameplay_Implementation_AreaDamageComponent);
  EZ_STATICLINK_REFERENCE(GameComponentsPlugin_Gameplay_Implementation_HeadBoneComponent);
  EZ_STATICLINK_REFERENCE(GameComponentsPlugin_Gameplay_Implementation_PowerConnectorComponent);
  EZ_STATICLINK_REFERENCE(GameComponentsPlugin_Gameplay_Implementation_ProjectileComponent);
  EZ_STATICLINK_REFERENCE(GameComponentsPlugin_Gameplay_Implementation_RaycastComponent);
  EZ_STATICLINK_REFERENCE(GameComponentsPlugin_Physics_Implementation_ClothSheetComponent);
  EZ_STATICLINK_REFERENCE(GameComponentsPlugin_Physics_Implementation_FakeRopeComponent);
  EZ_STATICLINK_REFERENCE(GameComponentsPlugin_Terrain_Implementation_HeightfieldComponent);
}
