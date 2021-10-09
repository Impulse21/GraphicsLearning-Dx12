#include "Dx12Core/ResourceStore.h"

#include "DirectXTex.h"

#include <filesystem>

namespace fs = std::filesystem;

using namespace Dx12Core;
using namespace DirectX;

Dx12Core::TextureResourceStore::TextureResourceStore(IGraphicsDevice* graphicsDevice)
	: m_graphicsDevice(graphicsDevice)
{
}

TextureHandle Dx12Core::TextureResourceStore::Load(std::string path, ICommandContext& uploadContext, BindFlags bindFlags, bool isSRGB)
{
	{
		std::scoped_lock _(this->m_storeMutex);
		auto itr = this->m_textureStore.find(path);
		if (itr != this->m_textureStore.end())
		{
			// validate resources bindings
			return itr->second;
		}
	}


	fs::path filePath(path);
	if (!fs::exists(filePath))
	{
		LOG_CORE_ERROR("File not found %s", path);
		throw std::exception();
	}

	// Load Texture
	TexMetadata metadata;
	ScratchImage scratchImage;
	std::wstring fileToLoad(path.begin(), path.end());
	if (filePath.extension() == ".dds")
	{
		ThrowIfFailed(
			LoadFromDDSFile(
				fileToLoad.c_str(),
				DDS_FLAGS_FORCE_RGB,
				&metadata,
				scratchImage));
	}
	else if (filePath.extension() == "hdr")
	{
		ThrowIfFailed(
			LoadFromHDRFile(
				fileToLoad.c_str(),
				&metadata,
				scratchImage));
	}
	else if (filePath.extension() == ".tga")
	{
		ThrowIfFailed(
			LoadFromTGAFile(
				fileToLoad.c_str(),
				&metadata,
				scratchImage));
	}
	else
	{
		ThrowIfFailed(
			LoadFromWICFile(
				fileToLoad.c_str(),
				WIC_FLAGS_FORCE_RGB,
				&metadata,
				scratchImage));
	}

	if (isSRGB)
	{
		metadata.format = MakeSRGB(metadata.format);
	}

	TextureDesc desc = {};
	desc.Width = metadata.width;
	desc.Height = metadata.height;
	desc.Format = metadata.format;
	desc.Bindings = bindFlags;
	desc.InitialState = D3D12_RESOURCE_STATE_COPY_DEST;
	desc.MipLevels = metadata.mipLevels;

	switch (metadata.dimension)
	{
	case TEX_DIMENSION_TEXTURE1D:
		desc.Dimension = TextureDimension::Texture1D;
		desc.ArraySize = metadata.arraySize;
		break;

	case TEX_DIMENSION_TEXTURE2D:
		desc.Dimension = TextureDimension::Texture2D;
		desc.ArraySize = metadata.arraySize;
		break;

	case TEX_DIMENSION_TEXTURE3D:
		desc.Dimension = TextureDimension::Texture3D;
		desc.Depth = metadata.depth;
		break;
	default:
		throw std::exception("Invalid texture dimension.");
		break;
	}

	TextureHandle texture = this->m_graphicsDevice->CreateTexture(desc);

	std::vector<D3D12_SUBRESOURCE_DATA> subresources(scratchImage.GetImageCount());
	const Image* pImages = scratchImage.GetImages();
	for (int i = 0; i < scratchImage.GetImageCount(); ++i)
	{
		auto& subresource = subresources[i];
		subresource.RowPitch = pImages[i].rowPitch;
		subresource.SlicePitch = pImages[i].slicePitch;
		subresource.pData = pImages[i].pixels;
	}

	uploadContext.WriteTexture(texture, 0, subresources.size(), subresources.data());
	uploadContext.TransitionBarrier(texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	{
		std::scoped_lock _(this->m_storeMutex);
		this->m_textureStore[path] = texture;
	}

	return texture;
}

void Dx12Core::TextureResourceStore::Clear()
{
	std::scoped_lock _(this->m_storeMutex);
	this->m_textureStore.clear();
}




