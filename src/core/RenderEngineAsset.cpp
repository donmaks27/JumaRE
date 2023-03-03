// Copyright © 2023 Leonov Maksim. All Rights Reserved.

#include "JumaRE/RenderEngineAsset.h"

namespace JumaRenderEngine
{
	void RenderEngineAsset::clearAsset()
	{
		onDestroying.call(this);
		onClearAsset();
	}
}
