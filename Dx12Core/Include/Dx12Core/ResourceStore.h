#pragma once

#include <string>
#include <unordered_map>
#include <mutex>

#include "Dx12Core/Dx12Core.h"

namespace Dx12Core
{
	class TextureResourceStore
	{
	public:
		TextureResourceStore(IGraphicsDevice* graphicsDevice);

		TextureHandle Load(std::string path, ICommandContext& uploadContext, BindFlags flags, bool isSRGB = false);

		void Clear();

	private:
		std::mutex m_storeMutex;
		IGraphicsDevice* m_graphicsDevice;

		std::unordered_map<std::string, TextureHandle> m_textureStore;
	};
}

