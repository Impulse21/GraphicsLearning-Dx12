
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
#include <random>

using namespace Dx12Core;
using namespace DirectX;


#pragma region Camera Logic
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
			this->m_camera.Update();
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
#pragma endregion

#pragma region Scene

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

struct DrawableGeometry
{
	BufferHandle VertexBuffer;
	BufferHandle IndexBuffer;
	uint32_t IndexCount = 0;
};

struct InstanceData
{
	XMMATRIX Transform = XMMatrixIdentity();
};

struct OmniLight
{
	XMFLOAT4 Position = { 0.0f, 0.0f, 0.0f, 1.0f};
	XMFLOAT4 Colour = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct EnviromentInfo
{
	TextureHandle SkyBox;
	TextureHandle IrradanceMap;
	TextureHandle PrefilteredMap;
};

struct Scene
{
	DrawableGeometry SphereGeometry = {};
	Material sphereMaterial = {};

	BufferHandle SphereInstanceData;
	uint32_t SphereInstanceCount;
	DrawableGeometry PlaneGeometry = {};
	Material planeMaterial = {};

	std::vector<OmniLight> Lights;

	EnviromentInfo Environment = {};
	Camera MainCamera = {};
};

#pragma endregion

struct SceneInfoCB
{
	XMFLOAT4X4 ViewProjectionMatrix;
	XMFLOAT3 CameraPosition;
	uint32_t IrradnaceMapTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t PreFilteredEnvMapTexIndex = INVALID_DESCRIPTOR_INDEX;
	uint32_t BrdfLUT = INVALID_DESCRIPTOR_INDEX;
	uint32_t NumLights = 0;
};

struct DrawInfoCB
{
	XMFLOAT4X4 WorldTransform;
	bool IsInstanced;
	Material Material;
};

namespace RootParameters
{
	enum
	{
		DrawInfoCB = 0,
		LightInfoSB,
		SceneInfoCB,
		InstanceInfoSB,
		Count
	};
}

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

	void LoadEnvData(
		ICommandContext& context,
		std::string const& basePath,
		std::string const& skyboxPath,
		std::string const& irradanceMapPath,
		std::string const& prefilteredRadanceMapPath,
		EnviromentInfo& outEnviroment);

	void CreateMainPso();
	void CreateSkyboxData(ICommandContext& context);

	void CreateGeometry(MeshData const& meshData, std::vector<Vertex> const& vertices, ICommandContext& context, DrawableGeometry& ourGeom);
private:
	const size_t FloorWidth = 100.0f;
	const size_t FloorLength = 100.0f;
	const size_t SphereGridMaxRows = 6;
	const size_t SphereGridMaxColumns = 6;
	const float SphereGridSpaceing = 1.2;

	Scene m_scene = {};
	TextureHandle m_brdfLUT;

	std::unique_ptr<ImGuiRenderer> m_imguiRenderer;

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

	XMFLOAT3 m_sunDirection = { 1.25, 1.0f, -1.0f };
	XMFLOAT3 m_sunColour = { 1.0f, 1.0f, 1.0f };

	std::unique_ptr<ICameraController> m_cameraController;

	struct SceneSettings
	{
		int NumLights = 100;
		uint32_t NumInstances = 3;
	} m_sceneSettings;
};


CREATE_APPLICATION(PbrDemo)


void PbrDemo::LoadContent()
{
	std::random_device rd;
	std::default_random_engine eng(rd());

	{
		const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
		const XMVECTOR upDir = XMVectorSet(0, 1, 0, 0);
		const XMVECTOR initCameraPos = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);

		this->m_scene.MainCamera.SetLookAt(initCameraPos, focusPoint, upDir);

		float aspectRatio =
			this->GetDevice()->GetCurrentSwapChainDesc().Width / static_cast<float>(this->GetDevice()->GetCurrentSwapChainDesc().Height);
		this->m_scene.MainCamera.InitialzeProjectMatrix(45.0f, aspectRatio);
		this->m_cameraController = CameraControllerFactory::Create(this->m_scene.MainCamera, upDir);
	}

	// Create Depth Buffer
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

	MeshData sphereMesh = MeshPrefabs::CreateSphere(1.0f, 16, true);

	// Create a Mesh
	auto& geometry = this->m_scene.SphereGeometry;

	std::vector<Vertex> sphereVertices = this->InterleaveVertexData(sphereMesh);
	this->CreateGeometry(sphereMesh, sphereVertices, copyContext, geometry);

	MeshData floorMesh = MeshPrefabs::CreatePlane(FloorWidth, FloorLength, true);
	std::vector<Vertex> floorVertices = this->InterleaveVertexData(floorMesh);
	this->CreateGeometry(floorMesh, floorVertices, copyContext, this->m_scene.PlaneGeometry);

	// Construct Mesh Indices
	{
		std::vector<InstanceData> instanceData(this->m_sceneSettings.NumInstances);

		// Construct Instance Data
		std::uniform_int_distribution<int> xDistr(-(FloorWidth / 2.0f), (FloorWidth / 2.0f));
		std::uniform_int_distribution<int> zDistr(-(FloorLength / 2.0f), (FloorLength / 2.0f));
		std::uniform_int_distribution<int> scaleDistr(0.3f, 3.0f);
		for (int i = 0; i < this->m_sceneSettings.NumInstances; i++)
		{
			auto& instance = instanceData[i];

			XMVECTOR scaleVec = XMVectorScale({ 1.0f, 1.0f, 1.0f }, scaleDistr(eng));
			XMVECTOR translation = XMVectorSet(xDistr(eng), 0.0f, zDistr(eng), 1.0f);
			instance.Transform = XMMatrixTranspose(XMMatrixAffineTransformation(scaleVec, g_XMIdentityR0, XMQuaternionIdentity(), translation));
		}

		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::ShaderResource;
		bufferDesc.SizeInBytes = sizeof(InstanceData) * instanceData.size();
		bufferDesc.StrideInBytes = sizeof(InstanceData);
		bufferDesc.DebugName = L"Sphere Instance Buffer";

		this->m_scene.SphereInstanceData = GetDevice()->CreateBuffer(bufferDesc);
		this->m_scene.SphereInstanceCount = instanceData.size();

		copyContext.WriteBuffer<InstanceData>(this->m_scene.SphereInstanceData, instanceData);
		copyContext.TransitionBarrier(this->m_scene.SphereInstanceData, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	this->LoadMaterials(copyContext);

	this->LoadEnvData(
		copyContext,
		"Assets\\Textures\\\IBL\\",
		"Milkyway\\skybox.dds",
		"Milkyway\\irradiance_map.dds",
		"Milkyway\\prefiltered_radiance_map.dds",
		this->m_scene.Environment);

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
	ImGui::Begin("Deffered Render Example", &showWindow, ImGuiWindowFlags_AlwaysAutoResize);
	
	ImGui::Text("Average Frame Time %f", this->GetFrameStats().AverageFrameTime);
	ImGui::DragInt("Number Of Omni Lights", &this->m_sceneSettings.NumLights, 1.0f, 0, 4000);
	ImGui::End();

	if (this->m_scene.Lights.size() != this->m_sceneSettings.NumLights)
	{
		std::random_device rd;
		std::default_random_engine eng(rd());
		std::uniform_real_distribution<float> xDistr(-(FloorWidth / 2.0f), (FloorWidth / 2.0f));
		std::uniform_real_distribution<float> yDistr(2.0f, 5.0f);
		std::uniform_real_distribution<float> zDistr(-(FloorLength / 2.0f), (FloorLength / 2.0f));
		std::uniform_real_distribution<float> colourDistr(0.01f, 1.0f);

		this->m_scene.Lights.resize(this->m_sceneSettings.NumLights);

		for (int i = 0; i < this->m_scene.Lights.size(); i++)
		{
			auto& light = this->m_scene.Lights[i];
			light.Position = { xDistr(eng), yDistr(eng), zDistr(eng) , 1.0f};
			light.Colour = { colourDistr(eng), colourDistr(eng), colourDistr(eng), 1.0f};
		}
	}

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
		s.VertexBuffer = this->m_scene.SphereGeometry.VertexBuffer;
		s.IndexBuffer = this->m_scene.SphereGeometry.IndexBuffer;
		s.PipelineState = this->m_pipelineState;
		s.Viewports.push_back(Viewport(swapChainDesc.Width, swapChainDesc.Height));
		s.ScissorRect.push_back(Rect(LONG_MAX, LONG_MAX));

		// TODO: Device should return a handle rather then a week refernce so the context
		// can track the resource.
		s.RenderTargets.push_back(this->GetDevice()->GetCurrentBackBuffer());
		s.DepthStencil = this->m_depthBuffer;

		gfxContext.SetGraphicsState(s);

		SceneInfoCB sceneInfo = {};
		sceneInfo.ViewProjectionMatrix = this->m_scene.MainCamera.GetViewProjMatrix();
		sceneInfo.CameraPosition = this->m_scene.MainCamera.GetPosition();
		sceneInfo.NumLights = this->m_scene.Lights.size();
		sceneInfo.IrradnaceMapTexIndex = this->GetDevice()->GetDescritporIndex(this->m_scene.Environment.IrradanceMap);
		sceneInfo.PreFilteredEnvMapTexIndex = this->GetDevice()->GetDescritporIndex(this->m_scene.Environment.PrefilteredMap);
		sceneInfo.BrdfLUT = this->GetDevice()->GetDescritporIndex(this->m_brdfLUT);
		gfxContext.BindDynamicConstantBuffer<SceneInfoCB>(RootParameters::SceneInfoCB, sceneInfo);

		gfxContext.BindStructuredBuffer(RootParameters::InstanceInfoSB, this->m_scene.SphereInstanceData);
		gfxContext.BindDynamicStructuredBuffer(RootParameters::LightInfoSB, this->m_scene.Lights);

		{
			DrawInfoCB drawInfo = {};
			drawInfo.IsInstanced = true;
			drawInfo.Material = this->m_scene.sphereMaterial;

			gfxContext.BindGraphics32BitConstants<DrawInfoCB>(RootParameters::DrawInfoCB, drawInfo);
			gfxContext.DrawIndexed(this->m_scene.SphereGeometry.IndexCount, this->m_scene.SphereInstanceCount);
		}

		// Draw Plane
		{
			DrawInfoCB drawInfo = {};
			drawInfo.IsInstanced = false;
			drawInfo.Material = this->m_scene.sphereMaterial;
			XMStoreFloat4x4(&drawInfo.WorldTransform, XMMatrixTranspose(XMMatrixIdentity()));

			gfxContext.BindGraphics32BitConstants<DrawInfoCB>(RootParameters::DrawInfoCB, drawInfo);
			gfxContext.DrawIndexed(this->m_scene.SphereGeometry.IndexCount, this->m_scene.SphereInstanceCount);
		}
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
		interleavedData[i].Colour = meshData.Colour.empty() ? XMFLOAT3(0.0f, 0.0f, 0.0f) : meshData.Colour[i];
		interleavedData[i].Tangent = meshData.Tangents[i];
	}

	return interleavedData;
}

void PbrDemo::LoadMaterials(ICommandContext& context)
{
	const std::string BaseDir = "Assets\\Textures\\\Materials\\";
	{
		Material& material = this->m_scene.sphereMaterial;
		this->LoadMaterial(
			context,
			BaseDir,
			"gray-granite-flecks\\gray-granite-flecks-albedo.png",
			"gray-granite-flecks\\gray-granite-flecks-Normal-dx.png",
			"gray-granite-flecks\\gray-granite-flecks-Roughness.png",
			"gray-granite-flecks\\gray-granite-flecks-Metallic.png",
			"gray-granite-flecks\\gray-granite-flecks-ao.png",
			material);
	}

	{
		Material& material = this->m_scene.planeMaterial;
		this->LoadMaterial(
			context,
			BaseDir,
			"hardwood-brown\\hardwood-brown-planks-albedo.png",
			"hardwood-brown\\hardwood-brown-planks-normal-dx.png",
			"hardwood-brown\\hardwood-brown-planks-roughness.png",
			"hardwood-brown\\hardwood-brown-planks-metallic.png",
			"hardwood-brown\\hardwood-brown-planks-ao.png",
			material);
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

void PbrDemo::LoadEnvData(
	ICommandContext& context,
	std::string const& basePath,
	std::string const& skyboxPath,
	std::string const& irradanceMapPath,
	std::string const& prefilteredRadanceMapPath,
	EnviromentInfo& outEnviroment)
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
	parameterLayout.AddConstantParameter<0, 0>(sizeof(DrawInfoCB) / 4);
	parameterLayout.AddSRVParameter<0, 0>();
	parameterLayout.AddCBVParameter<1, 0>();
	parameterLayout.AddSRVParameter<1, 0>();
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

void PbrDemo::CreateGeometry(MeshData const& meshData, std::vector<Vertex> const& vertices, ICommandContext& context, DrawableGeometry& outGeom)
{
	{
		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::VertexBuffer;
		bufferDesc.DebugName = L"Vertex Buffer";
		bufferDesc.SizeInBytes = sizeof(Vertex) * vertices.size();
		bufferDesc.StrideInBytes = sizeof(Vertex);

		outGeom.VertexBuffer = this->GetDevice()->CreateBuffer(bufferDesc);

		// Upload Buffer
		context.WriteBuffer<Vertex>(outGeom.VertexBuffer, vertices);
		context.TransitionBarrier(outGeom.VertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	{
		BufferDesc bufferDesc = {};
		bufferDesc.BindFlags = BindFlags::IndexBuffer;
		bufferDesc.DebugName = L"Index Buffer";
		bufferDesc.SizeInBytes = sizeof(uint16_t) * meshData.Indices.size();
		bufferDesc.StrideInBytes = sizeof(uint16_t);

		outGeom.IndexBuffer = this->GetDevice()->CreateBuffer(bufferDesc);

		context.WriteBuffer<uint16_t>(outGeom.IndexBuffer, meshData.Indices);
		context.TransitionBarrier(outGeom.IndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		outGeom.IndexCount = meshData.Indices.size();
	}
}