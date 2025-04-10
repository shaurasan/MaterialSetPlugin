
#include "MakeNode.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "MaterialGraph/MaterialGraph.h"
#include "MaterialEditingLibrary.h"

#include "Components/MeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"


void FMakeNode::CreateTextureSampleNode(UMaterial* Material, UTexture2D* Albedo, UTexture2D* Normal, UTexture2D* Roughness)
{

    if (!Material) return;

    int X = 100;
    int Y = 100;

    if (Albedo)
    {
        X = 600;
        AddConnectNode(Material, Albedo, EMaterialProperty::MP_BaseColor, X, Y);
        Y += 250;
    }

    if (Normal)
    {
        X -= 200;
        AddConnectNode(Material, Normal, EMaterialProperty::MP_Normal, X, Y);
        Y += 250;
    }

    if (Roughness)
    {
        AddConnectNode(Material, Roughness, EMaterialProperty::MP_Roughness, X, Y);
    }

    UMaterialEditingLibrary::LayoutMaterialExpressions(Material);
    Material -> MarkPackageDirty();
    Material -> ForceRecompileForRendering();

    UE_LOG(LogTemp, Warning, TEXT("applied"));



}


void FMakeNode::AddConnectNode(UMaterial* Material, UTexture2D* Texture, EMaterialProperty Property, int32 X, int32 Y)
{
    if (!Material || !Texture) return;

    auto* TextureSample = Cast<UMaterialExpressionTextureSample>(
        UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionTextureSample::StaticClass()) 
    );

    if (!TextureSample) return;


    TextureSample -> Texture = Texture;
    TextureSample -> MaterialExpressionEditorX = X;
    TextureSample -> MaterialExpressionEditorY = Y;

    //ノーマル用設定
    if (Property == MP_Normal)
    {
        TextureSample -> SamplerType = SAMPLERTYPE_Normal;
    }

    //ラフネス用設定
    if (Property == MP_Roughness)
    {
        Texture -> SRGB = false;
        TextureSample -> SamplerType = SAMPLERTYPE_LinearColor;

        UMaterialEditingLibrary::ConnectMaterialProperty(TextureSample, TEXT("R"), MP_Metallic);
        UMaterialEditingLibrary::ConnectMaterialProperty(TextureSample, TEXT("G"), MP_Roughness);
        UMaterialEditingLibrary::ConnectMaterialProperty(TextureSample, TEXT("B"), MP_AmbientOcclusion);

        return;
    }


    UMaterialEditingLibrary::ConnectMaterialProperty(TextureSample, TEXT("RGB"), Property);
}
