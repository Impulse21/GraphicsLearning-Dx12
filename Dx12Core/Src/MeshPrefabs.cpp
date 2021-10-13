#include "Dx12Core/MeshPrefabs.h"

#include "Dx12Core/Log.h"

using namespace DirectX;
using namespace Dx12Core;

// Helper for flipping winding of geometric primitives for LH vs. RH coords
static void ReverseWinding(MeshData& meshData)
{
	assert((meshData.Indices.size() % 3) == 0);
	for (auto it = meshData.Indices.begin(); it != meshData.Indices.end(); it += 3)
	{
		std::swap(*it, *(it + 2));
	}
}

MeshData Dx12Core::MeshPrefabs::CreateCube(float size, bool rhsCoord)
{

	// A cube has six faces, each one pointing in a different direction.
	const int FaceCount = 6;

	static const XMVECTORF32 faceNormals[FaceCount] =
	{
		{ 0,  0,  1 },
		{ 0,  0, -1 },
		{ 1,  0,  0 },
		{ -1,  0,  0 },
		{ 0,  1,  0 },
		{ 0, -1,  0 },
	};

	static const XMFLOAT3 faceColour[] =
	{
		{ 1.0f,  0.0f,  0.0f },
		{ 0.0f,  1.0f,  0.0f },
		{ 0.0f,  0.0f,  1.0f },
	};

	static const XMFLOAT2 textureCoordinates[4] =
	{
		{ 1, 0 },
		{ 1, 1 },
		{ 0, 1 },
		{ 0, 0 },
	};

	MeshData outMeshData = {};
	size /= 2;

	for (int i = 0; i < FaceCount; i++)
	{
		XMVECTOR normal = faceNormals[i];

		// Get two vectors perpendicular both to the face normal and to each other.
		XMVECTOR basis = (i >= 4) ? g_XMIdentityR2 : g_XMIdentityR1;

		XMVECTOR side1 = XMVector3Cross(normal, basis);
		XMVECTOR side2 = XMVector3Cross(normal, side1);

		// Six indices (two triangles) per face.
		size_t vbase = outMeshData.Positions.size();
		outMeshData.Indices.push_back(static_cast<uint16_t>(vbase + 0));
		outMeshData.Indices.push_back(static_cast<uint16_t>(vbase + 1));
		outMeshData.Indices.push_back(static_cast<uint16_t>(vbase + 2));

		outMeshData.Indices.push_back(static_cast<uint16_t>(vbase + 0));
		outMeshData.Indices.push_back(static_cast<uint16_t>(vbase + 2));
		outMeshData.Indices.push_back(static_cast<uint16_t>(vbase + 3));

		XMFLOAT3 positon;
		XMStoreFloat3(&positon, (normal - side1 - side2) * size);
		outMeshData.Positions.push_back(positon);
		XMStoreFloat3(&positon, (normal - side1 + side2) * size);
		outMeshData.Positions.push_back(positon);
		XMStoreFloat3(&positon, (normal + side1 + side2) * size);
		outMeshData.Positions.push_back(positon);
		XMStoreFloat3(&positon, (normal + side1 - side2) * size);
		outMeshData.Positions.push_back(positon);

		outMeshData.TexCoords.push_back(textureCoordinates[0]);
		outMeshData.TexCoords.push_back(textureCoordinates[1]);
		outMeshData.TexCoords.push_back(textureCoordinates[2]);
		outMeshData.TexCoords.push_back(textureCoordinates[3]);


		outMeshData.Colour.push_back(faceColour[0]);
		outMeshData.Colour.push_back(faceColour[1]);
		outMeshData.Colour.push_back(faceColour[2]);
		outMeshData.Colour.push_back(faceColour[3]);

	}

	if (rhsCoord)
	{
		ReverseWinding(outMeshData);
	}

	return outMeshData;
}

MeshData Dx12Core::MeshPrefabs::CreateSphere(float diameter, size_t tessellation, bool rhcoords)
{
	if (tessellation < 3)
	{
		LOG_CORE_ERROR("tessellation parameter out of range");
		throw std::out_of_range("tessellation parameter out of range");
	}

	MeshData outMeshData = {};

	float radius = diameter / 2.0f;
	size_t verticalSegments = tessellation;
	size_t horizontalSegments = tessellation * 2;


	// Create rings of vertices at progressively higher latitudes.
	for (size_t i = 0; i <= verticalSegments; i++)
	{
		float v = 1 - (float)i / verticalSegments;

		float latitude = (i * XM_PI / verticalSegments) - XM_PIDIV2;
		float dy, dxz;

		XMScalarSinCos(&dy, &dxz, latitude);

		// Create a single ring of vertices at this latitude.
		for (size_t j = 0; j <= horizontalSegments; j++)
		{
			float u = (float)j / horizontalSegments;

			float longitude = j * XM_2PI / horizontalSegments;
			float dx, dz;

			XMScalarSinCos(&dx, &dz, longitude);

			dx *= dxz;
			dz *= dxz;

			XMVECTOR normal = XMVectorSet(dx, dy, dz, 0);

			XMFLOAT3 positon;
			XMStoreFloat3(&positon, normal * radius);
			outMeshData.Positions.push_back(positon);
			outMeshData.TexCoords.push_back({ u, v });
			outMeshData.Normal.push_back({ dx, dy, dz });
			outMeshData.Colour.push_back({ 0.1f, 0.1f, 0.1f });
		}
	}

	// Fill the index buffer with triangles joining each pair of latitude rings.
	size_t stride = horizontalSegments + 1;
	for (size_t i = 0; i < verticalSegments; i++)
	{
		for (size_t j = 0; j <= horizontalSegments; j++)
		{
			size_t nextI = i + 1;
			size_t nextJ = (j + 1) % stride;

			outMeshData.Indices.push_back(static_cast<uint16_t>(i * stride + j));
			outMeshData.Indices.push_back(static_cast<uint16_t>(nextI * stride + j));
			outMeshData.Indices.push_back(static_cast<uint16_t>(i * stride + nextJ));

			outMeshData.Indices.push_back(static_cast<uint16_t>(i * stride + nextJ));
			outMeshData.Indices.push_back(static_cast<uint16_t>(nextI * stride + j));
			outMeshData.Indices.push_back(static_cast<uint16_t>(nextI * stride + nextJ));
		}
	}

	if (rhcoords)
	{
		ReverseWinding(outMeshData);
	}

	return outMeshData;
}
