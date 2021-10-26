
#include "Dx12Core/Dx12Core.h"
#include "Dx12Core/Application.h"

#include "Shaders/PbrDemoVS_compiled.h"
#include "Shaders/PbrDemoPS_compiled.h""

#include "Shaders/SkyboxVS_compiled.h"
#include "Shaders/SkyboxPS_compiled.h"

#include <imgui.h>
#include "Dx12Core/ImGui/ImGuiRenderer.h"

#include <DirectXMath.h>

#include "Dx12Core/MeshPrefabs.h"

using namespace Dx12Core;
using namespace DirectX;


class Camera
{
public:
	Camera() = default;

	void Update()
	{
		this->CalculateViewProjMatrix();
	}

	void SetLookAt(XMVECTOR const& eye, XMVECTOR const& target, XMVECTOR const& up)
	{
		this->m_position = eye;
		assert(XMVectorGetX(XMVector3Dot(target, up)) == 0.0f);

		this->CalculateViewMatrixLH(
			this->m_position,
			XMVectorSubtract(target, eye),
			up);
	}

	void InitialzeProjectMatrix(float fovDegrees, float aspectRatio, float nearZ = 0.1f, float farZ = 100.f)
	{
		XMStoreFloat4x4(
			&this->m_projMatrix,
			XMMatrixPerspectiveFovLH(XMConvertToRadians(fovDegrees), aspectRatio, nearZ, farZ));
	}

	void Translate(XMVECTOR const& translation)
	{
		this->m_position = XMVectorAdd(this->m_position, translation);

		this->m_viewMatrix.r[4] = XMVectorSelect(g_XMIdentityR3.v, this->m_position, g_XMSelect1110);
	}

	void Translate(float pitchRad, float yawRad, float rollRad = 0.0f)
	{
		// I assume the values are already converted to radians.
		if (0)
		{
			float cosPitch = cos(pitchRad);
			float sinPitch = sin(pitchRad);
			float cosYaw = cos(yawRad);
			float sinYaw = sin(yawRad);

			XMVECTOR xAxis = { cosYaw, 0, -sinYaw };
			XMVECTOR yAxis = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
			XMVECTOR zAxis = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };

			this->m_viewMatrix = this->ConstructViewMatrixLH(xAxis, yAxis, zAxis, this->m_position);
		}
		else
		{
			XMMATRIX rot = XMMatrixRotationRollPitchYaw(pitchRad, yawRad, rollRad);
			// Let's take the inverse of the rotation matrix and apply it to the view
			// Since this is purely a rotation matrix we can just take the transpose of the matrix
			rot = XMMatrixTranspose(rot);
			this->m_viewMatrix = this->ConstructViewMatrixLH(rot.r[0], rot.r[1], rot.r[2], this->m_position);
		}
	}

	void Translate(XMMATRIX const& worldRotation)
	{
		// Since the view matrix is the inverse the camera's world position.
		// Let's take the inverse of the rotation matrix and apply it to the view
		// Since this is purely a rotation matrix we can just take the transpose of the matrix
		XMMATRIX viewRot = XMMatrixTranspose(worldRotation);
		this->m_viewMatrix = this->ConstructViewMatrixLH(viewRot.r[0], viewRot.r[1], viewRot.r[2], this->m_position);
	}

	void SetTransformLH(XMMATRIX const& worldOrientation, XMVECTOR const& translation)
	{
		this->m_position = translation;
		this->CalculateViewMatrixLH(this->m_position, worldOrientation.r[2], worldOrientation.r[1]);
	}

	const XMFLOAT4X4& GetViewMatrix()
	{ 
		XMFLOAT4X4 retVal;
		XMStoreFloat4x4(&retVal, this->m_viewMatrix);
		return retVal;
	}

	const XMFLOAT4X4& GetProjMatrix()
	{
		return this->m_projMatrix;
	}

	const XMFLOAT4X4& GetViewProjMatrix()
	{
		return this->m_viewProjMatrix;
	}

	const XMFLOAT3& GetPosition() const 
	{
		XMFLOAT3 retVal = {};
		XMStoreFloat3(&retVal, this->m_position);

		return retVal;
	}

	const XMVECTOR& GetPositionVector() const
	{
		return this->m_position;
	}

	const XMVECTOR& GetForwardVector() const
	{
		return this->m_viewMatrix.r[2];
	}

	const XMVECTOR& GetUpVector() const
	{
		return this->m_viewMatrix.r[1];
	}

	const XMVECTOR& GetRightVector() const
	{
		return this->m_viewMatrix.r[0];
	}

private:

	void SetLookDirectionLH(XMVECTOR const& ForwardVector, XMVECTOR const& upVector)
	{

	}

	void CalculateViewMatrixLH(XMVECTOR const& eye, XMVECTOR const& forward, XMVECTOR const& up)
	{
		// axisZ == Forward Vector
		const XMVECTOR axisZ = XMVector3Normalize(forward);

		// axisX == right vector
		const XMVECTOR axisX = XMVector3Normalize(XMVector3Cross(up, forward));

		// Axisy == Up vector ( forward cross with right)
		const XMVECTOR axisY = XMVector3Cross(axisZ, axisX);

		this->m_viewMatrix = this->ConstructViewMatrixLH(
			axisX,
			axisY,
			axisZ,
			eye);
	}

	XMMATRIX ConstructViewMatrixLH(XMVECTOR const& axisX, XMVECTOR const& axisY, XMVECTOR const& axisZ, XMVECTOR const& position) const
	{
		const XMVECTOR negEye = XMVectorNegate(position);

		// Not sure I get this bit.
		const XMVECTOR d0 = XMVector3Dot(axisX, negEye);
		const XMVECTOR d1 = XMVector3Dot(axisY, negEye);
		const XMVECTOR d2 = XMVector3Dot(axisZ, negEye);

		// Construct column major view matrix;
		XMMATRIX m;
		m.r[0] = XMVectorSelect(d0, axisX, g_XMSelect1110.v);
		m.r[1] = XMVectorSelect(d1, axisY, g_XMSelect1110.v);
		m.r[2] = XMVectorSelect(d2, axisZ, g_XMSelect1110.v);
		m.r[3] = g_XMIdentityR3.v;

		return XMMatrixTranspose(m);
	}

	void CalculateViewProjMatrix()
	{
		auto proj = XMLoadFloat4x4(&this->m_projMatrix);

		XMStoreFloat4x4(&this->m_viewProjMatrix, XMMatrixTranspose(this->m_viewMatrix * proj));
	}

private:
	const XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	const XMVECTOR DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	XMMATRIX m_viewMatrix;
	XMFLOAT4X4 m_projMatrix;
	XMFLOAT4X4 m_viewProjMatrix;

	XMVECTOR m_position;
};

class ICameraController
{
public:
	~ICameraController() = default;

	virtual void Update(double elapsedTime) = 0;

	virtual void EnableDebugWindow(bool enabled) = 0;
};

class ImguiCameraController : public ICameraController
{
public:
	ImguiCameraController(Camera& camera)
		: m_camera(camera)
	{
	}

	void Update(double elapsedTime) override
	{
		static bool showWindow = true;
		ImGui::Begin("Camera Controls", &showWindow, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::DragFloat("Pitch", &this->m_pitch, 1.0f, -90.0f, 90.0f);
		ImGui::DragFloat("Yaw", &this->m_yaw, 1.0f, 0.0f, 360.0f);

		this->m_currentPitch = XMConvertToRadians(this->m_pitch);
		this->m_currentHeading = XMConvertToRadians(this->m_yaw);

		this->m_camera.Translate(this->m_currentPitch, this->m_currentHeading);

		ImGui::End();

		m_camera.Update();
	}

	// No-op
	void EnableDebugWindow(bool enabled) {};
private:
	XMVECTOR m_worldNorth;
	XMVECTOR m_worldEast;
	XMVECTOR m_worldUp;

	Camera& m_camera;

	float m_pitch = 0.0f;
	float m_yaw = 0.0f;

	float m_currentHeading = 0.0f;
	float m_currentPitch = 0.0f;
};

class DebugCameraController : public ICameraController
{
public:
	DebugCameraController(Camera& camera, XMVECTOR const& worldUp)
		: m_camera(camera)
	{
		bool joystickFound = glfwJoystickPresent(this->m_joystickId);
		assert(joystickFound);
		if (!joystickFound)
		{
			LOG_ERROR("Unable to locate joystick. This is currently required by the application");
		}

		this->m_worldUp = XMVector3Normalize(worldUp);
		this->m_worldNorth = XMVector3Normalize(XMVector3Cross(g_XMIdentityR0, this->m_worldUp));
		this->m_worldEast = XMVector3Cross(this->m_worldUp, this->m_worldNorth);

		// Construct current pitch
		this->m_pitch = std::sin(XMVectorGetX(XMVector3Dot(this->m_worldUp, this->m_camera.GetForwardVector())));

		XMVECTOR forward = XMVector3Normalize(XMVector3Cross(this->m_camera.GetRightVector(), this->m_worldUp));
		this->m_yaw = atan2(-XMVectorGetX(XMVector3Dot(this->m_worldEast, forward)), XMVectorGetX(XMVector3Dot(this->m_worldNorth, forward)));

	}

	void Update(double elapsedTime)
	{
		GLFWgamepadstate state;

		if (!glfwGetGamepadState(this->m_joystickId, &state))
		{
			return;
		}


		this->m_pitch += this->GetAxisInput(state, GLFW_GAMEPAD_AXIS_RIGHT_Y) * this->m_lookSpeed;
		this->m_yaw += this->GetAxisInput(state, GLFW_GAMEPAD_AXIS_RIGHT_X) * this->m_lookSpeed;

		// Max out pitich
		this->m_pitch = XMMin(XM_PIDIV2, this->m_pitch);
		this->m_pitch = XMMax(-XM_PIDIV2, this->m_pitch);

		if (this->m_yaw > XM_PI)
		{
			this->m_yaw -= XM_2PI;
		}
		else if (this->m_yaw <= -XM_PI)
		{
			this->m_yaw += XM_2PI;
		}

		const XMMATRIX worldBase(this->m_worldEast, this->m_worldUp, this->m_worldNorth, g_XMIdentityR3);
		XMMATRIX orientation = worldBase * XMMatrixRotationX(this->m_pitch) * XMMatrixRotationY(this->m_yaw);

		float forward = this->GetAxisInput(state, GLFW_GAMEPAD_AXIS_LEFT_Y) * this->m_movementSpeed;
		float strafe = this->GetAxisInput(state, GLFW_GAMEPAD_AXIS_LEFT_X) * this->m_strafeMovementSpeed;

		XMVECTOR translation = XMVectorSet(strafe, 0.0f, -forward, 1.0f);
		translation =  XMVector3TransformNormal(translation, orientation) + this->m_camera.GetPositionVector();

		this->m_camera.SetTransformLH(orientation, translation);

		this->m_camera.Update();
		this->ShowDebugWindow();

	}

	void EnableDebugWindow(bool enabled) { this->m_enableDebugWindow = enabled; }

private:
	float GetAxisInput(GLFWgamepadstate const& state,  int inputId)
	{
		auto value = state.axes[inputId];
		return value > this->DeadZone || value < -this->DeadZone
			? value
			: 0.0f;
	}

	void ShowDebugWindow()
	{
		ImGui::Begin("Camera Info", &this->m_enableDebugWindow, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Controller Type:  Debug Camera");
		ImGui::Text("Pitch %f (Degrees)", XMConvertToDegrees(this->m_pitch));
		ImGui::Text("Yaw %f (Degrees)", XMConvertToDegrees(this->m_yaw));
		ImGui::NewLine();

		ImGui::DragFloat("Movement Speed", &this->m_movementSpeed, 0.001f, 0.001f, 1.0f);
		ImGui::DragFloat("Strafe Speed", &this->m_strafeMovementSpeed, 0.001f, 0.001f, 1.0f);
		ImGui::DragFloat("Look Speed", &this->m_lookSpeed, 0.001f, 0.001f, 1.0f);

		ImGui::End();
	}

private:
	const int m_joystickId = GLFW_JOYSTICK_1;
	const float DeadZone = 0.2f;

	float m_movementSpeed = 0.3f;
	float m_strafeMovementSpeed = 0.08f;
	float m_lookSpeed = 0.01f;

	float m_pitch = 0.0f;
	float m_yaw = 0.0f;
	Camera& m_camera;

	bool m_enableDebugWindow = false;

	XMVECTOR m_worldUp;
	XMVECTOR m_worldNorth;
	XMVECTOR m_worldEast;
};

namespace CameraControllerFactory
{
	std::unique_ptr<ICameraController> Create(Camera& camera, XMVECTOR const& worldUp)
	{
		// return std::make_unique<ImguiCameraController>(camera);
		return std::make_unique<DebugCameraController>(camera, worldUp);
	}
}

struct Vertex
{
	XMFLOAT3 Positon;
	XMFLOAT3 Normal;
	XMFLOAT3 Colour;
	XMFLOAT2 TexCoord;
	XMFLOAT4 Tangent;

};

enum DrawFlags
{
	DrawAlbedoOnly = 0x01,
	DrawNormalWSOnly = 0x02,
	DrawRoughnessOnly = 0x04,
	DrawMetallicOnly = 0x08,
	DrawAoOnly = 0x10,
	DrawTangentOnly = 0x20,
	DrawBiTangentOnly = 0x40,
};

struct DrawInfo
{
	uint32_t InstanceIndex;

	XMFLOAT3 Albedo;
	float Metallic;
	float Roughness;
	float Ao;

	uint32_t AlbedoTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t NormalTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t MetallicTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t RoughnessTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t AoTexIndex = INVALID_DESCRIPTOR_INDEX;

	uint32_t DrawFlags = 0;
};

struct InstanceInfo
{
	XMMATRIX WorldMatrix;
};

struct SceneInfo
{
	XMFLOAT4X4 ViewProjectionMatrix;
	XMFLOAT3 CameraPosition;
	uint32_t _padding; // I DO NOT understand why this is happening 
	XMFLOAT3 SunDirection;
	uint32_t _padding2;
	XMFLOAT3 SunColour;
	uint32_t IrradnaceMapTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t PreFilteredEnvMapTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t BrdfLUT = INVALID_DESCRIPTOR_INDEX;
};

struct EnvInfo
{
	TextureHandle SkyBox;
	TextureHandle IrradanceMap;
	TextureHandle PrefilteredMap;
};

struct Material
{
	XMFLOAT3 Albedo = { 0.0f ,0.0f, 0.0f };
	float Metallic = 0.4f;
	float Roughness = 0.1f;
	float Ao = 0.3f;

	uint32_t AlbedoTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t NormalTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t MetallicTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t RoughnessTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t AoTexIndex = INVALID_DESCRIPTOR_INDEX;
};

namespace RootParameters
{
	enum
	{
		DrawInfoCB = 0,
		SceneInfoCB,
		InstanceInfoSB,
		Count
	};
}

namespace SceneType
{
	enum
	{
		Sphere,
		SphereGrid,
		TexturedMaterials,
		Count,
	};

	static const char* Names[] = { "Sphere", "Sphere Grid", "Textured Materials" };
}



static const char* RenderModelNames[] = { "Disabled", "Enviroment", "Irradiance", "PF Radiance" };

class PbrDemo : public ApplicationDx12Base
{
public:
	PbrDemo() = default;

protected:
	void LoadContent() override;
	void Update(double elapsedTime) override;
	void Render() override;

private:
	void XM_CALLCONV ComputeMatrices(CXMMATRIX view, CXMMATRIX projection, XMFLOAT4X4& modelViewProjection);
	void XM_CALLCONV ComputeMatrices(CXMMATRIX world, CXMMATRIX view, CXMMATRIX projection, XMFLOAT4X4& modelViewProjection);

	std::vector<Vertex> InterleaveVertexData(MeshData const& meshData);

	void LoadMaterials(ICommandContext& context);
	void LoadMaterial(
		ICommandContext& context,
		std::string const& basePath,
		std::string const& albedoPath,
		std::string const& bumpMapPath,
		std::string const& roughnessPath,
		std::string const& metallicPath,
		Material& outMaterial);

	void LoadMaterial(
		ICommandContext& context,
		std::string const& basePath,
		std::string const& albedoPath,
		std::string const& bumpMapPath,
		std::string const& roughnessPath,
		std::string const& metallicPath,
		std::string const& aoPath,
		Material& outMaterial);

	void LoadEnviroments(ICommandContext& context);
	void LoadEnvData(
		ICommandContext& context,
		std::string const& basePath,
		std::string const& skyboxPath,
		std::string const& irradanceMapPath,
		std::string const& prefilteredRadanceMapPath,
		EnvInfo& outEnviroment);

	void CreateMainPso();
	void CreateSkyboxData(ICommandContext& context);

private:
	const size_t SphereGridMaxRows = 6;
	const size_t SphereGridMaxColumns = 6;
	const float SphereGridSpaceing = 1.2;

	TextureHandle m_irradanceMap;
	TextureHandle m_prefilteredMap;
	TextureHandle m_brdfLUT;

	std::unique_ptr<ImGuiRenderer> m_imguiRenderer;
	BufferHandle m_vertexbuffer;
	BufferHandle m_indexBuffer;

	BufferHandle m_gridInstanceBuffer;
	BufferHandle m_centerMeshInstanceBuffer;
	BufferHandle m_texturedMeshInstanceBuffer;

	MeshData m_sphereMesh;
	GraphicsPipelineHandle m_pipelineState;

	struct SkyboxCB
	{
		XMFLOAT4X4 ViewProjectionMatrix = {};
		uint32_t skyboxTexIndex = INVALID_DESCRIPTOR_INDEX;
	};

	MeshData m_skyboxMesh;
	BufferHandle m_skyboxVertexBuffer;
	BufferHandle m_skyboxIndexBuffer;
	GraphicsPipelineHandle m_skyboxPso;

	TextureHandle m_depthBuffer;

	XMMATRIX m_viewMatrix = XMMatrixIdentity();
	XMMATRIX m_porjMatrix = XMMatrixIdentity();

	std::vector<EnvInfo> m_enviroments;

	XMFLOAT3 m_sunDirection = { 1.25, 1.0f, -1.0f };
	XMFLOAT3 m_sunColour = { 1.0f, 1.0f, 1.0f };

	Camera m_camera;
	std::unique_ptr<ICameraController> m_cameraController;

	const XMVECTOR m_focusPoint = XMVectorSet(0, 0, 0, 1);
	const XMVECTOR m_upDirection = XMVectorSet(0, 1, 0, 0);
	const XMVECTOR m_cameraClose = XMVectorSet(0.0f, 0.0f, -3.0f, 1.0f);
	const XMVECTOR m_cameraMed = XMVectorSet(0.0f, 0.0f, -6.0f, 1.0f);
	const XMVECTOR m_cameraFar = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);
	XMVECTOR m_cameraPosition = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);

	int m_sceneType = SceneType::SphereGrid;

	struct MaterialSettings
	{
		std::vector<const char*> MaterialNames;
		int SelectedMaterialId = 0;
		bool DisableIbl = false;

	} m_materialSettings;

	std::vector<Material> m_material;

	struct EnviromentSettings
	{
		std::vector<const char*> EnvNames;
		int SelectedEnvNameId = 0;

		enum SkyboxRenderMode
		{
			Disabled,
			Enviroment,
			Irradiance,
			PrefilterdRadiance,
		};

		int RenderMode = SkyboxRenderMode::Enviroment;
	} m_enviromentSettings;

	uint32_t m_drawFlags = 0;
};


CREATE_APPLICATION(PbrDemo)


void PbrDemo::LoadContent()
{
	{
		this->m_viewMatrix = XMMatrixLookAtLH(this->m_cameraFar, this->m_focusPoint, this->m_upDirection);

		float aspectRatio =
			this->GetDevice()->GetCurrentSwapChainDesc().Width / static_cast<float>(this->GetDevice()->GetCurrentSwapChainDesc().Height);
		this->m_porjMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), aspectRatio, 0.1f, 100.0f);

		this->m_camera.SetLookAt(this->m_cameraFar, this->m_focusPoint, this->m_upDirection);
		this->m_camera.InitialzeProjectMatrix(45.0f, aspectRatio);
		this->m_cameraController = CameraControllerFactory::Create(this->m_camera, this->m_upDirection);
		this->m_cameraController->EnableDebugWindow(true);
	}

	{
		TextureDesc desc = {};
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.Width = this->GetDevice()->GetCurrentSwapChainDesc().Width;
		desc.Height = this->GetDevice()->GetCurrentSwapChainDesc().Height;
		desc.Dimension = TextureDimension::Texture2D;
		desc.DebugName = "Depth Buffer";
		desc.InitialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.DepthStencil = { 1.0f, 0 };
		desc.OptmizedClearValue = std::make_optional<D3D12_CLEAR_VALUE>(clearValue);

		this->m_depthBuffer = this->GetDevice()->CreateTexture(desc);
	}

	this->CreateMainPso();

	this->m_imguiRenderer = std::make_unique<ImGuiRenderer>();

	this->m_imguiRenderer->Initialize(this->GetWindow(), this->GetDevice());

	ICommandContext& copyContext = this->GetDevice()->BeginContext();

	this->CreateSkyboxData(copyContext);

	this->m_sphereMesh = MeshPrefabs::CreateSphere(1.0f, 16, true);

	LOG_INFO("Interleaving vertex data");
	std::vector<Vertex> vertices = this->InterleaveVertexData(this->m_sphereMesh);
	{
		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::VertexBuffer;
		bufferDesc.DebugName = L"Vertex Buffer";
		bufferDesc.SizeInBytes = sizeof(Vertex) * vertices.size();
		bufferDesc.StrideInBytes = sizeof(Vertex);

		this->m_vertexbuffer = this->GetDevice()->CreateBuffer(bufferDesc);

		// Upload Buffer
		copyContext.WriteBuffer<Vertex>(this->m_vertexbuffer, vertices);
		copyContext.TransitionBarrier(this->m_vertexbuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	{
		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::IndexBuffer;
		bufferDesc.DebugName = L"Index Buffer";
		bufferDesc.SizeInBytes = sizeof(uint16_t) * this->m_sphereMesh.Indices.size();
		bufferDesc.StrideInBytes = sizeof(uint16_t);

		this->m_indexBuffer = this->GetDevice()->CreateBuffer(bufferDesc);

		copyContext.WriteBuffer<uint16_t>(this->m_indexBuffer, this->m_sphereMesh.Indices);
		copyContext.TransitionBarrier(this->m_indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}

	// Construct instance data
	std::vector<InstanceInfo> gridInstanceBufferData((SphereGridMaxRows * SphereGridMaxColumns));
	const XMMATRIX rotationMatrix = XMMatrixIdentity();
	XMMATRIX scaleMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	{
		size_t index = 0;
		for (int iRow = 0; iRow < SphereGridMaxRows; iRow++)
		{
			for (int iCol = 0; iCol < SphereGridMaxColumns; iCol++)
			{

				XMMATRIX translationMatrix =
					XMMatrixTranslation(
						static_cast<float>(iCol - static_cast<int32_t>((SphereGridMaxColumns / 2))) * SphereGridSpaceing,
						(static_cast<float>(iRow - static_cast<int32_t>((SphereGridMaxRows / 2))) * SphereGridSpaceing) + 0.5f,
						0.0f);

				gridInstanceBufferData[index].WorldMatrix = scaleMatrix * rotationMatrix * translationMatrix;
				gridInstanceBufferData[index].WorldMatrix = XMMatrixTranspose(gridInstanceBufferData[index].WorldMatrix);
				index++;
			}
		}

		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::ShaderResource;
		bufferDesc.SizeInBytes = sizeof(InstanceInfo) * gridInstanceBufferData.size();
		bufferDesc.StrideInBytes = sizeof(InstanceInfo);
		bufferDesc.DebugName = L"Grid Instance Buffer";

		this->m_gridInstanceBuffer = GetDevice()->CreateBuffer(bufferDesc);

		copyContext.WriteBuffer<InstanceInfo>(this->m_gridInstanceBuffer, gridInstanceBufferData);
		copyContext.TransitionBarrier(this->m_gridInstanceBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	// Construct instance data
	std::vector<InstanceInfo> centreInstanceBuffer(1);
	{
		centreInstanceBuffer[0].WorldMatrix = scaleMatrix * rotationMatrix * XMMatrixTranslation(0.0f, 0.0f, 0.0f);
		centreInstanceBuffer[0].WorldMatrix = XMMatrixTranspose(centreInstanceBuffer[0].WorldMatrix);

		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::ShaderResource;
		bufferDesc.SizeInBytes = sizeof(InstanceInfo) * centreInstanceBuffer.size();
		bufferDesc.StrideInBytes = sizeof(InstanceInfo);
		bufferDesc.DebugName = L"Centre Instance Buffer";

		this->m_centerMeshInstanceBuffer = GetDevice()->CreateBuffer(bufferDesc);

		copyContext.WriteBuffer<InstanceInfo>(this->m_centerMeshInstanceBuffer, centreInstanceBuffer);
		copyContext.TransitionBarrier(this->m_centerMeshInstanceBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	this->LoadMaterials(copyContext);

	// Construct instance data
	std::vector<InstanceInfo> texturedSphereInstanceBufferData(this->m_material.size());
	{
		for (int i = 0; i < texturedSphereInstanceBufferData.size(); i++)
		{

			XMMATRIX translationMatrix =
				XMMatrixTranslation(
					static_cast<float>(i - static_cast<int32_t>((texturedSphereInstanceBufferData.size() / 2))) * SphereGridSpaceing,
					0.0f,
					0.0f);

			texturedSphereInstanceBufferData[i].WorldMatrix = scaleMatrix * rotationMatrix * translationMatrix;
			texturedSphereInstanceBufferData[i].WorldMatrix = XMMatrixTranspose(texturedSphereInstanceBufferData[i].WorldMatrix);
		}

		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::ShaderResource;
		bufferDesc.SizeInBytes = sizeof(InstanceInfo) * texturedSphereInstanceBufferData.size();
		bufferDesc.StrideInBytes = sizeof(InstanceInfo);
		bufferDesc.DebugName = L"Textured Material Instance Buffer";

		this->m_texturedMeshInstanceBuffer = GetDevice()->CreateBuffer(bufferDesc);

		copyContext.WriteBuffer<InstanceInfo>(this->m_texturedMeshInstanceBuffer, texturedSphereInstanceBufferData);
		copyContext.TransitionBarrier(this->m_texturedMeshInstanceBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	this->LoadEnviroments(copyContext);

	this->m_brdfLUT =
		this->GetTextureStore()->Load(
			"Assets\\Textures\\IBL\\BrdfLut.dds",
			copyContext,
			BindFlags::ShaderResource);

	// Load IrradanceMap
	this->GetDevice()->Submit(true);
}

void PbrDemo::Update(double elapsedTime)
{
	this->m_imguiRenderer->BeginFrame();

	static bool showWindow = true;
	ImGui::Begin("Options", &showWindow, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Combo("Scene Type", &this->m_sceneType, SceneType::Names, IM_ARRAYSIZE(SceneType::Names));
	ImGui::NewLine();

	if (this->m_sceneType == SceneType::SphereGrid)
	{
		this->m_cameraPosition = this->m_cameraFar;
	}
	else if (this->m_sceneType == SceneType::TexturedMaterials)
	{
		this->m_cameraPosition = this->m_cameraMed;
	}
	else
	{
		this->m_cameraPosition = this->m_cameraClose;
	}

	this->m_viewMatrix = XMMatrixLookAtLH(this->m_cameraPosition, this->m_focusPoint, this->m_upDirection);

	if (ImGui::CollapsingHeader("Enviroment Settings"))
	{
		ImGui::Combo("Skybox Render Mode", &this->m_enviromentSettings.RenderMode, RenderModelNames, IM_ARRAYSIZE(RenderModelNames));

		if (this->m_enviromentSettings.RenderMode != EnviromentSettings::SkyboxRenderMode::Disabled)
		{
			ImGui::Combo(
				"Skybox",
				&this->m_enviromentSettings.SelectedEnvNameId,
				this->m_enviromentSettings.EnvNames.data(),
				this->m_enviromentSettings.EnvNames.size());
		}

	}
	if (ImGui::CollapsingHeader("Material Settings"))
	{
		ImGui::Checkbox("Disable IBL", &this->m_materialSettings.DisableIbl);

		switch (this->m_sceneType)
		{
		case SceneType::SphereGrid:
		{
			this->m_materialSettings.SelectedMaterialId = 0;
			auto& selectedMaterial = this->m_material[this->m_materialSettings.SelectedMaterialId];
			ImGui::ColorEdit3("Albedo", reinterpret_cast<float*>(&selectedMaterial.Albedo));
			ImGui::DragFloat("AO", &selectedMaterial.Ao, 0.01f, 0.0f, 1.0f);
			ImGui::TextWrapped("Roughtness increases by Coloumn Left(0.0f) ---> Right(1.0f)");
			ImGui::TextWrapped("Mettalic increates by row Top(0.0f) ---> Bottom(1.0f)");
		}
		break;

		case SceneType::TexturedMaterials:
			break;
		case SceneType::Sphere:
		{

			ImGui::Combo(
				"Material Type",
				&this->m_materialSettings.SelectedMaterialId,
				this->m_materialSettings.MaterialNames.data(),
				this->m_materialSettings.MaterialNames.size());

			auto& selectedMaterial = this->m_material[this->m_materialSettings.SelectedMaterialId];

			if (selectedMaterial.AlbedoTexIndex == INVALID_DESCRIPTOR_INDEX)
			{
				ImGui::ColorEdit3("Albedo", reinterpret_cast<float*>(&selectedMaterial.Albedo));
			}

			if (selectedMaterial.RoughnessTexIndex == INVALID_DESCRIPTOR_INDEX)
			{
				ImGui::DragFloat("Roughness", &selectedMaterial.Roughness, 0.01f, 0.1f, 1.0f);
			}

			if (selectedMaterial.MetallicTexIndex == INVALID_DESCRIPTOR_INDEX)
			{
				ImGui::DragFloat("Metallic", &selectedMaterial.Metallic, 0.01f, 0.0f, 1.0f);
			}

			if (selectedMaterial.AoTexIndex == INVALID_DESCRIPTOR_INDEX)
			{
				ImGui::DragFloat("AO", &selectedMaterial.Ao, 0.01f, 0.0f, 1.0f);
			}
		}
		}
	}

	ImGui::NewLine();

	if (ImGui::CollapsingHeader("Directional Light Parameters"))
	{
		ImGui::DragFloat3("Direction", reinterpret_cast<float*>(&this->m_sunDirection), 0.01f, -1.0f, 1.0f);
		ImGui::ColorEdit3("Colour", reinterpret_cast<float*>(&this->m_sunColour));
	}

	ImGui::NewLine();
	if (ImGui::CollapsingHeader("Draw Flags"))
	{
		ImGui::CheckboxFlags("Alebdo Only", &this->m_drawFlags, DrawFlags::DrawAlbedoOnly);
		ImGui::CheckboxFlags("Roughness Only", &this->m_drawFlags, DrawFlags::DrawRoughnessOnly);
		ImGui::CheckboxFlags("Metallic Only", &this->m_drawFlags, DrawFlags::DrawMetallicOnly);
		ImGui::CheckboxFlags("Ao Only", &this->m_drawFlags, DrawFlags::DrawAoOnly);
		ImGui::CheckboxFlags("Normal Only", &this->m_drawFlags, DrawFlags::DrawNormalWSOnly);
		ImGui::CheckboxFlags("Tangent Only", &this->m_drawFlags, DrawFlags::DrawTangentOnly);
		ImGui::CheckboxFlags("BiTangent Only", &this->m_drawFlags, DrawFlags::DrawBiTangentOnly);
	}
	ImGui::End();


	this->m_cameraController->Update(elapsedTime);
}

void PbrDemo::Render()
{
	ICommandContext& gfxContext = this->GetDevice()->BeginContext();

	ITexture* backBuffer = this->GetDevice()->GetCurrentBackBuffer();

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Clearing Render Target");
		gfxContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		gfxContext.ClearTextureFloat(backBuffer, { 0.0f, 0.0f, 0.0f, 1.0f });

		gfxContext.ClearDepthStencilTexture(this->m_depthBuffer, true, 1.0f, false, 0);
	}

	const SwapChainDesc& swapChainDesc = this->GetDevice()->GetCurrentSwapChainDesc();

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Main Render Pass");

		GraphicsState s = {};
		s.VertexBuffer = this->m_vertexbuffer;
		s.IndexBuffer = this->m_indexBuffer;
		s.PipelineState = this->m_pipelineState;
		s.Viewports.push_back(Viewport(swapChainDesc.Width, swapChainDesc.Height));
		s.ScissorRect.push_back(Rect(LONG_MAX, LONG_MAX));

		// TODO: Device should return a handle rather then a week refernce so the context
		// can track the resource.
		s.RenderTargets.push_back(this->GetDevice()->GetCurrentBackBuffer());
		s.DepthStencil = this->m_depthBuffer;

		gfxContext.SetGraphicsState(s);

		SceneInfo sceneInfo = {};
		/*
		this->ComputeMatrices(
			this->m_viewMatrix,
			this->m_porjMatrix,
			sceneInfo.ViewProjectionMatrix);
		*/
		// XMStoreFloat3(&sceneInfo.CameraPosition, this->m_cameraPosition);
		sceneInfo.ViewProjectionMatrix = this->m_camera.GetViewProjMatrix();
		sceneInfo.CameraPosition = this->m_camera.GetPosition();
		sceneInfo.SunColour = this->m_sunColour;
		sceneInfo.SunDirection = this->m_sunDirection;

		if (!this->m_materialSettings.DisableIbl)
		{
			auto& envInfo = this->m_enviroments[this->m_enviromentSettings.SelectedEnvNameId];
			sceneInfo.IrradnaceMapTexIndex = this->GetDevice()->GetDescritporIndex(envInfo.IrradanceMap);
			sceneInfo.PreFilteredEnvMapTexIndex = this->GetDevice()->GetDescritporIndex(envInfo.PrefilteredMap);
			sceneInfo.BrdfLUT = this->GetDevice()->GetDescritporIndex(this->m_brdfLUT);
		}

		gfxContext.BindDynamicConstantBuffer<SceneInfo>(RootParameters::SceneInfoCB, sceneInfo);

		DrawInfo drawInfo = {};

		Material& selectedMaterial = this->m_material[this->m_materialSettings.SelectedMaterialId];
		drawInfo.Albedo = selectedMaterial.Albedo;
		drawInfo.AlbedoTexIndex = selectedMaterial.AlbedoTexIndex;
		drawInfo.Roughness = selectedMaterial.Roughness;
		drawInfo.RoughnessTexIndex = selectedMaterial.RoughnessTexIndex;
		drawInfo.Metallic = selectedMaterial.Metallic;
		drawInfo.MetallicTexIndex = selectedMaterial.MetallicTexIndex;
		drawInfo.Ao = selectedMaterial.Ao;
		drawInfo.AoTexIndex = selectedMaterial.AoTexIndex;
		drawInfo.NormalTexIndex = selectedMaterial.NormalTexIndex;

		drawInfo.DrawFlags = this->m_drawFlags;

		if (this->m_sceneType == SceneType::SphereGrid)
		{
			gfxContext.BindStructuredBuffer(RootParameters::InstanceInfoSB, this->m_gridInstanceBuffer);
			drawInfo.InstanceIndex = 0;
			for (int iRow = 0; iRow < SphereGridMaxRows; iRow++)
			{
				drawInfo.Metallic = static_cast<float>(iRow) / static_cast<float>(SphereGridMaxRows);
				for (int iCol = 0; iCol < SphereGridMaxColumns; iCol++)
				{
					drawInfo.Roughness = std::clamp(static_cast<float>(iCol) / static_cast<float>(SphereGridMaxColumns), 0.05f, 1.0f);
					gfxContext.BindGraphics32BitConstants<DrawInfo>(RootParameters::DrawInfoCB, drawInfo);
					gfxContext.DrawIndexed(this->m_sphereMesh.Indices.size());

					drawInfo.InstanceIndex++;
				}
			}
		}
		else if (this->m_sceneType == SceneType::TexturedMaterials)
		{
			gfxContext.BindStructuredBuffer(RootParameters::InstanceInfoSB, this->m_texturedMeshInstanceBuffer);
			for (int i = 0; i < this->m_material.size(); i++)
			{
				drawInfo.InstanceIndex = i;

				Material& selectedMaterial = this->m_material[i];
				drawInfo.Albedo = selectedMaterial.Albedo;
				drawInfo.AlbedoTexIndex = selectedMaterial.AlbedoTexIndex;
				drawInfo.Roughness = selectedMaterial.Roughness;
				drawInfo.RoughnessTexIndex = selectedMaterial.RoughnessTexIndex;
				drawInfo.Metallic = selectedMaterial.Metallic;
				drawInfo.MetallicTexIndex = selectedMaterial.MetallicTexIndex;
				drawInfo.Ao = selectedMaterial.Ao;
				drawInfo.AoTexIndex = selectedMaterial.AoTexIndex;
				drawInfo.NormalTexIndex = selectedMaterial.NormalTexIndex;

				gfxContext.BindGraphics32BitConstants<DrawInfo>(RootParameters::DrawInfoCB, drawInfo);
				gfxContext.DrawIndexed(this->m_sphereMesh.Indices.size());
			}
		}
		else
		{
			gfxContext.BindStructuredBuffer(RootParameters::InstanceInfoSB, this->m_centerMeshInstanceBuffer);
			drawInfo.InstanceIndex = 0;
			gfxContext.BindGraphics32BitConstants<DrawInfo>(RootParameters::DrawInfoCB, drawInfo);
			gfxContext.DrawIndexed(this->m_sphereMesh.Indices.size());
		}
	}

	if (this->m_enviromentSettings.RenderMode != EnviromentSettings::SkyboxRenderMode::Disabled)
	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Render Skybox");

		GraphicsState s = {};
		s.VertexBuffer = this->m_skyboxVertexBuffer;
		s.IndexBuffer = this->m_skyboxIndexBuffer;
		s.PipelineState = this->m_skyboxPso;
		s.Viewports.push_back(Viewport(swapChainDesc.Width, swapChainDesc.Height));
		s.ScissorRect.push_back(Rect(LONG_MAX, LONG_MAX));

		// TODO: Device should return a handle rather then a week refernce so the context
		// can track the resource.
		s.RenderTargets.push_back(this->GetDevice()->GetCurrentBackBuffer());
		s.DepthStencil = this->m_depthBuffer;
		gfxContext.SetGraphicsState(s);

		SkyboxCB skyboxCb = {};

		auto& selectedEnv = this->m_enviroments[this->m_enviromentSettings.SelectedEnvNameId];

		switch (this->m_enviromentSettings.RenderMode)
		{
		case  EnviromentSettings::SkyboxRenderMode::Irradiance:
			skyboxCb.skyboxTexIndex = this->GetDevice()->GetDescritporIndex(selectedEnv.IrradanceMap);
			break;
		case  EnviromentSettings::SkyboxRenderMode::PrefilterdRadiance:
			skyboxCb.skyboxTexIndex = this->GetDevice()->GetDescritporIndex(selectedEnv.PrefilteredMap);
			break;
		case  EnviromentSettings::SkyboxRenderMode::Enviroment:
		default:
			skyboxCb.skyboxTexIndex = this->GetDevice()->GetDescritporIndex(selectedEnv.SkyBox);
		}

		// The view matrix should only consider the camera's rotation, but not the translation.
		// Camera position, 
		/*
		this->ComputeMatrices(
			this->m_viewMatrix,
			this->m_porjMatrix,
			skyboxCb.ViewProjectionMatrix);
			*/

		skyboxCb.ViewProjectionMatrix = this->m_camera.GetViewProjMatrix();
		gfxContext.BindGraphics32BitConstants<SkyboxCB>(0, skyboxCb);
		gfxContext.DrawIndexed(this->m_skyboxMesh.Indices.size());

	}

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Draw ImGui UI");

		this->m_imguiRenderer->Draw(gfxContext, this->GetDevice());
	}

	{
		ScopedMarker m = gfxContext.BeginScropedMarker("Transition Render Target");
		gfxContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}

	this->GetDevice()->Submit();
}

void XM_CALLCONV PbrDemo::ComputeMatrices(CXMMATRIX view, CXMMATRIX projection, XMFLOAT4X4& viewProjection)
{
	XMMATRIX m = view * projection;
	XMStoreFloat4x4(&viewProjection, XMMatrixTranspose(m));
}

void XM_CALLCONV PbrDemo::ComputeMatrices(CXMMATRIX world, CXMMATRIX view, CXMMATRIX projection, XMFLOAT4X4& modelViewProjection)
{
	XMMATRIX m = world * view * projection;
	XMStoreFloat4x4(&modelViewProjection, XMMatrixTranspose(m));
}

std::vector<Vertex> PbrDemo::InterleaveVertexData(MeshData const& meshData)
{
	std::vector<Vertex> interleavedData(meshData.Positions.size());

	for (int i = 0; i < meshData.Positions.size(); i++)
	{
		interleavedData[i].Positon = meshData.Positions[i];
		interleavedData[i].TexCoord = meshData.TexCoords[i];
		interleavedData[i].Normal = meshData.Normal[i];
		interleavedData[i].Colour = meshData.Colour[i];
		interleavedData[i].Tangent = meshData.Tangents[i];
	}

	return interleavedData;
}

void PbrDemo::LoadMaterials(ICommandContext& context)
{
	// Load Custom Material
	{
		Material& material = m_material.emplace_back();
		material.Albedo = XMFLOAT3{ 1.0f, 0.0f, 0.0f };
		material.Ao = 0.3f;
		material.Metallic = 0.4f;
		material.Roughness = 1.0f;

		this->m_materialSettings.MaterialNames.emplace_back("Custom Material");
	}

	const std::string BaseDir = "Assets\\Textures\\\Materials\\";
	{
		Material& material = this->m_material.emplace_back();
		this->LoadMaterial(
			context,
			BaseDir,
			"rustediron2\\rustediron2_basecolor.png",
			"rustediron2\\rustediron2_normal.png",
			"rustediron2\\rustediron2_roughness.png",
			"rustediron2\\rustediron2_metallic.png",
			material);

		this->m_materialSettings.MaterialNames.emplace_back("rusted iron");
	}

	{
		Material& material = this->m_material.emplace_back();
		this->LoadMaterial(
			context,
			BaseDir,
			"gray-granite-flecks\\gray-granite-flecks-albedo.png",
			"gray-granite-flecks\\gray-granite-flecks-Normal-dx.png",
			"gray-granite-flecks\\gray-granite-flecks-Roughness.png",
			"gray-granite-flecks\\gray-granite-flecks-Metallic.png",
			"gray-granite-flecks\\gray-granite-flecks-ao.png",
			material);

		this->m_materialSettings.MaterialNames.emplace_back("gray granite flecks");
	}

	{
		Material& material = this->m_material.emplace_back();
		this->LoadMaterial(
			context,
			BaseDir,
			"modern-brick1\\modern-brick1_albedo.png",
			"modern-brick1\\modern-brick1_normal-dx.png",
			"modern-brick1\\modern-brick1_roughness.png",
			"modern-brick1\\modern-brick1_metallic.png",
			"modern-brick1\\modern-brick1_ao.png",
			material);

		this->m_materialSettings.MaterialNames.emplace_back("modern bricks");
	}

	{
		Material& material = this->m_material.emplace_back();
		this->LoadMaterial(
			context,
			BaseDir,
			"worn-shiny-metal\\worn-shiny-metal-albedo.png",
			"worn-shiny-metal\\worn-shiny-metal-Normal-dx.png",
			"worn-shiny-metal\\worn-shiny-metal-Roughness.png",
			"worn-shiny-metal\\worn-shiny-metal-Metallic.png",
			"worn-shiny-metal\\worn-shiny-metal-ao.png",
			material);

		this->m_materialSettings.MaterialNames.emplace_back("stworn shiny metal");
	}

	{
		Material& material = this->m_material.emplace_back();
		this->LoadMaterial(
			context,
			BaseDir,
			"scuffed-plastic\\scuffed-plastic4-alb.png",
			"scuffed-plastic\\scuffed-plastic-normal.png",
			"scuffed-plastic\\scuffed-plastic-rough.png",
			"scuffed-plastic\\scuffed-plastic-metal.png",
			"scuffed-plastic\\scuffed-plastic-ao.png",
			material);

		this->m_materialSettings.MaterialNames.emplace_back("scuffed plastic");
	}

	/*
	{
		Material& material = this->m_material.emplace_back();
		this->LoadMaterial(
			context,
			BaseDir,
			"Grass\\grass1-albedo3.png",
			"Grass\\grass1-normal1-dx.png",
			"Grass\\grass1-rough.png",
			"Grass\\worn-shiny-metal-Metallic.png",
			"Grass\\grass1-ao.png",
			material);

		this->m_materialSettings.MaterialNames.emplace_back("grass");
	}
	*/
	{
		Material& material = this->m_material.emplace_back();
		this->LoadMaterial(
			context,
			BaseDir,
			"gold\\lightgold_albedo.png",
			"gold\\lightgold_normal-dx.png",
			"gold\\lightgold_roughness.png",
			"gold\\lightgold_metallic.png",
			material);

		this->m_materialSettings.MaterialNames.emplace_back("gold");
	}
}

void PbrDemo::LoadMaterial(
	ICommandContext& context,
	std::string const& basePath,
	std::string const& albedoPath,
	std::string const& bumpMapPath,
	std::string const& roughnessPath,
	std::string const& metallicPath,
	Material& outMaterial)
{
	auto albedo =
		this->GetTextureStore()->Load(
			basePath + albedoPath,
			context,
			BindFlags::ShaderResource);
	auto normal =
		this->GetTextureStore()->Load(
			basePath + bumpMapPath,
			context,
			BindFlags::ShaderResource);
	auto roughness =
		this->GetTextureStore()->Load(
			basePath + roughnessPath,
			context,

			BindFlags::ShaderResource);
	auto metallic =
		this->GetTextureStore()->Load(
			basePath + metallicPath,
			context,
			BindFlags::ShaderResource);


	outMaterial.AlbedoTexIndex = this->GetDevice()->GetDescritporIndex(albedo);
	outMaterial.NormalTexIndex = this->GetDevice()->GetDescritporIndex(normal);
	outMaterial.RoughnessTexIndex = this->GetDevice()->GetDescritporIndex(roughness);
	outMaterial.MetallicTexIndex = this->GetDevice()->GetDescritporIndex(metallic);
}

void PbrDemo::LoadMaterial(
	ICommandContext& context,
	std::string const& basePath,
	std::string const& albedoPath,
	std::string const& bumpMapPath,
	std::string const& roughnessPath,
	std::string const& metallicPath,
	std::string const& aoPath,
	Material& outMaterial)
{
	this->LoadMaterial(
		context,
		basePath,
		albedoPath,
		bumpMapPath,
		roughnessPath,
		metallicPath,
		outMaterial);

	auto ao =
		this->GetTextureStore()->Load(
			basePath + aoPath,
			context,
			BindFlags::ShaderResource);

	outMaterial.AoTexIndex = this->GetDevice()->GetDescritporIndex(ao);
}

void PbrDemo::LoadEnviroments(ICommandContext& context)
{
	const std::string BaseDir = "Assets\\Textures\\\IBL\\";
	{
		EnvInfo& envInfo = this->m_enviroments.emplace_back();
		this->LoadEnvData(
			context,
			BaseDir,
			"PaperMill_Ruins_E\\PaperMill_Skybox.dds",
			"PaperMill_Ruins_E\\PaperMill_IrradianceMap.dds",
			"PaperMill_Ruins_E\\PaperMill_RadianceMap.dds",
			envInfo);

		this->m_enviromentSettings.EnvNames.emplace_back("Paper Mill Ruins");
	}

	{
		EnvInfo& envInfo = this->m_enviroments.emplace_back();
		this->LoadEnvData(
			context,
			BaseDir,
			"Serpentine_Valley\\output_skybox.dds",
			"Serpentine_Valley\\output_irradiance.dds",
			"Serpentine_Valley\\output_radiance.dds",
			envInfo);

		this->m_enviromentSettings.EnvNames.emplace_back("Serpentine Valley");
	}

	{
		EnvInfo& envInfo = this->m_enviroments.emplace_back();
		this->LoadEnvData(
			context,
			BaseDir,
			"cmftStudio\\output_skybox.dds",
			"cmftStudio\\output_irradiance.dds",
			"cmftStudio\\output_radiance.dds",
			envInfo);

		this->m_enviromentSettings.EnvNames.emplace_back("Cmft Studio");
	}
}

void PbrDemo::LoadEnvData(
	ICommandContext& context,
	std::string const& basePath,
	std::string const& skyboxPath,
	std::string const& irradanceMapPath,
	std::string const& prefilteredRadanceMapPath,
	EnvInfo& outEnviroment)
{
	outEnviroment.SkyBox =
		this->GetTextureStore()->Load(
			basePath + skyboxPath,
			context,
			BindFlags::ShaderResource);

	outEnviroment.IrradanceMap =
		this->GetTextureStore()->Load(
			basePath + irradanceMapPath,
			context,
			BindFlags::ShaderResource);

	outEnviroment.PrefilteredMap =
		this->GetTextureStore()->Load(
			basePath + prefilteredRadanceMapPath,
			context,
			BindFlags::ShaderResource);
}

void PbrDemo::CreateMainPso()
{
	ShaderDesc d = {};
	d.shaderType = ShaderType::Vertex;

	ShaderHandle vs = this->GetDevice()->CreateShader(d, gPbrDemoVS, sizeof(gPbrDemoVS));

	d.shaderType = ShaderType::Pixel;
	ShaderHandle ps = this->GetDevice()->CreateShader(d, gPbrDemoPS, sizeof(gPbrDemoPS));

	GraphicsPipelineDesc pipelineDesc = {};
	pipelineDesc.InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOUR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	pipelineDesc.VS = vs;
	pipelineDesc.PS = ps;
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineDesc.RenderState.RtvFormats.push_back(this->GetDevice()->GetCurrentSwapChainDesc().Format);
	pipelineDesc.RenderState.DsvFormat = DXGI_FORMAT_D32_FLOAT;

	BindlessShaderParameterLayout bindlessParameterLayout = {};
	bindlessParameterLayout.MaxCapacity = UINT_MAX;
	bindlessParameterLayout.AddParameterSRV(101);
	bindlessParameterLayout.AddParameterSRV(102);

	ShaderParameterLayout parameterLayout = {};
	parameterLayout.AddConstantParameter<0, 0>(sizeof(DrawInfo) / 4);
	parameterLayout.AddCBVParameter<1, 0>();
	parameterLayout.AddSRVParameter<0, 0>();
	parameterLayout.AddStaticSampler<0, 0>(
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		1.0f);
	parameterLayout.AddStaticSampler<1, 0>(
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
		);

	pipelineDesc.UseShaderParameters = true;
	pipelineDesc.ShaderParameters.Binding = &parameterLayout;
	pipelineDesc.ShaderParameters.Bindless = &bindlessParameterLayout;
	pipelineDesc.ShaderParameters.AllowInputLayout();
	this->m_pipelineState = this->GetDevice()->CreateGraphicPipeline(pipelineDesc);

}

void PbrDemo::CreateSkyboxData(ICommandContext& context)
{
	// Intentioanlly don't reverse coords so we can render the inside of the cube.
	this->m_skyboxMesh = MeshPrefabs::CreateCube();
	{
		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::VertexBuffer;
		bufferDesc.DebugName = L"Skybox Vertex Buffer";
		bufferDesc.SizeInBytes = sizeof(XMFLOAT3) * this->m_skyboxMesh.Positions.size();
		bufferDesc.StrideInBytes = sizeof(XMFLOAT3);

		this->m_skyboxVertexBuffer = this->GetDevice()->CreateBuffer(bufferDesc);

		// Upload Buffer
		context.WriteBuffer<XMFLOAT3>(this->m_skyboxVertexBuffer, this->m_skyboxMesh.Positions);
		context.TransitionBarrier(this->m_skyboxVertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	{
		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::IndexBuffer;
		bufferDesc.DebugName = L"Skybox Index Buffer";
		bufferDesc.SizeInBytes = sizeof(uint16_t) * this->m_skyboxMesh.Indices.size();
		bufferDesc.StrideInBytes = sizeof(uint16_t);

		this->m_skyboxIndexBuffer = this->GetDevice()->CreateBuffer(bufferDesc);

		context.WriteBuffer<uint16_t>(this->m_skyboxIndexBuffer, this->m_skyboxMesh.Indices);
		context.TransitionBarrier(this->m_skyboxIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}

	ShaderDesc d = {};
	d.shaderType = ShaderType::Vertex;

	ShaderHandle vs = this->GetDevice()->CreateShader(d, gSkyboxVS, sizeof(gSkyboxVS));

	d.shaderType = ShaderType::Pixel;
	ShaderHandle ps = this->GetDevice()->CreateShader(d, gSkyboxPS, sizeof(gSkyboxPS));

	// TODO I AM HERE: Add Colour and push Constants
	GraphicsPipelineDesc pipelineDesc = {};
	pipelineDesc.InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	pipelineDesc.VS = vs;
	pipelineDesc.PS = ps;
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineDesc.RenderState.RtvFormats.push_back(this->GetDevice()->GetCurrentSwapChainDesc().Format);
	pipelineDesc.RenderState.DsvFormat = DXGI_FORMAT_D32_FLOAT;

	BindlessShaderParameterLayout bindlessParameterLayout = {};
	bindlessParameterLayout.MaxCapacity = UINT_MAX;
	bindlessParameterLayout.AddParameterSRV(102);

	ShaderParameterLayout parameterLayout = {};
	parameterLayout.AddConstantParameter<0, 0>(sizeof(SkyboxCB) / 4);
	parameterLayout.AddStaticSampler<0, 0>(
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		1.0f);

	pipelineDesc.UseShaderParameters = true;
	pipelineDesc.ShaderParameters.Binding = &parameterLayout;
	pipelineDesc.ShaderParameters.Bindless = &bindlessParameterLayout;
	pipelineDesc.ShaderParameters.AllowInputLayout();
	CD3DX12_DEPTH_STENCIL_DESC depthState = {};
	depthState.DepthEnable = true;
	depthState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	pipelineDesc.RenderState.DepthStencilState = &depthState;
	this->m_skyboxPso = this->GetDevice()->CreateGraphicPipeline(pipelineDesc);
}
