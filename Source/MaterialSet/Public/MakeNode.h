
#pragma once

#include "CoreMinimal.h"

class UMaterial;
class UTexture2D;

class FMakeNode
{

public:
    static void CreateTextureSampleNode(UMaterial* Material, UTexture2D* Albedo, UTexture2D* Normal, UTexture2D* Roughness);

    static void AddConnectNode(UMaterial* Material, UTexture2D* Texture, EMaterialProperty Property, int X, int Y);
        
};