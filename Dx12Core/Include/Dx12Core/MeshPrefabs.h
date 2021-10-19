#pragma once

#include <vector>
#include <DirectXMath.h>

namespace Dx12Core
{
	struct MeshData
	{
		std::vector<uint16_t> Indices;

		std::vector<DirectX::XMFLOAT3> Positions;
		std::vector<DirectX::XMFLOAT3> Normal;
		std::vector<DirectX::XMFLOAT3> Colour;
		std::vector<DirectX::XMFLOAT2> TexCoords;
		std::vector<DirectX::XMFLOAT4> Tangents;
		std::vector<DirectX::XMFLOAT4> BiTangents;
	};

	class MeshPrefabs
	{
	public:
		static MeshData CreateCube(
			float size = 1,
			bool rhsCoord = false);

		static MeshData CreateSphere(
			float diameter = 1,
			size_t tessellation = 16,
			bool rhcoords = false);
	};
}

