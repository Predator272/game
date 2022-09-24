#ifndef NDEBUG
#define __CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "helper.hpp"
#include <iostream>
#include <tchar.h>
#include <directxcollision.h>

class CameraClass
{
public:
	DirectX::XMVECTOR Eye, At, Up;

	std::unique_ptr<Engine::IConstantBuffer> ConstantBuffer;

	struct CONSTANT_BUFFER_STRUCT
	{
		DirectX::XMMATRIX World;
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX Projection;
	} ConstantBufferStruct;
public:
	CameraClass(const std::unique_ptr<Engine::IDevice>& Device): Up(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
	{
		ConstantBuffer = Engine::IConstantBuffer::Make(Device, sizeof(this->ConstantBufferStruct));
		ConstantBuffer->Set();

		this->SetPosition(0.0f, 0.0f, 0.0f);
		this->UpdateView(0.0f, 0.0f);
	}

	void SetPosition(float X, float Y, float Z)
	{
		this->Eye = DirectX::XMVectorSet(X, Y, Z, 0.0f);
	}

	void UpdateView(float X, float Y)
	{
		this->At = DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), DirectX::XMMatrixRotationRollPitchYaw(Y, X, 0.0f));
		this->ConstantBufferStruct.View = DirectX::XMMatrixLookAtLH(this->Eye, DirectX::XMVectorAdd(this->Eye, this->At), this->Up);
	}

	void UpdateProjection(float Angle, float AspectRation)
	{
		this->ConstantBufferStruct.Projection = DirectX::XMMatrixPerspectiveFovLH(Angle, AspectRation, 0.1f, 10000.0f);
	}

	DirectX::XMMATRIX& UpdateTransform()
	{
		return this->ConstantBufferStruct.World;
	}

	void UpdateBuffers()
	{
		ConstantBuffer->Update(&this->ConstantBufferStruct);
	}
};

class GameObject: public Engine::IBaseInterface
{
private:
	std::unique_ptr<Engine::IMesh> Mesh;
	std::unique_ptr<Engine::ITexture> Texture;
	DirectX::XMMATRIX Transform;
public:
	GameObject(const std::unique_ptr<Engine::IDevice>& Device, const std::string& MeshData, const std::string& TextureData): Transform(DirectX::XMMatrixIdentity())
	{
		std::unique_ptr<Engine::IMeshLoader> MeshLoader = Engine::IMeshLoader::LoadFromMemory(MeshData);
		this->Mesh = Engine::IMesh::Make(Device, MeshLoader->GetIndices(), MeshLoader->GetPositions(), MeshLoader->GetTexCoords(), MeshLoader->GetNormals());

		std::unique_ptr<Engine::ITextureLoader> TextureLoader = Engine::ITextureLoader::LoadFromMemory(TextureData);
		this->Texture = Engine::ITexture::Make(Device, TextureLoader->GetWidth(), TextureLoader->GetHeight(), TextureLoader->GetPixels());
	}

	void Set()
	{
		this->Mesh->Set();
		this->Texture->Set();
	}

	DirectX::XMMATRIX& GetTransform()
	{
		return this->Transform;
	}
};



class Main: public Engine::IMain
{
private:
	std::unique_ptr<Engine::IDevice> Device;
	std::unique_ptr<Engine::IShader> SolidShader, LightShader;

	std::unique_ptr<CameraClass> Camera;
	std::unique_ptr<GameObject> SkyBox, Cube, Sphere;

	DirectX::XMFLOAT2 Mouse = {};
	bool Keys[0xff] = {};
public:
	void Create(const std::unique_ptr<Engine::IApplication>& App) override
	{
		App->SetTitle("Game");
		const int ScreenWidth = Engine::GetSceenWidth(), ScreenHeight = Engine::GetSceenHeight(), WindowWidth = 1280, WindowHeight = 720;
		App->SetPosition((ScreenWidth - WindowWidth) / 2, (ScreenHeight - WindowHeight) / 2, WindowWidth, WindowHeight);

		this->Device = Engine::IDevice::Make(App);

		this->Camera = std::make_unique<CameraClass>(this->Device);

		const std::string BasePath = Engine::GetBasePath();

		this->SolidShader = Engine::IShader::Make(this->Device);
		this->SolidShader->SetVertexShader(Engine::Compile(Engine::TypeVertexShader, Engine::ReadFile(BasePath + "/shaders/SolidVertexShader.hlsl")));
		this->SolidShader->SetPixelShader(Engine::Compile(Engine::TypePixelShader, Engine::ReadFile(BasePath + "/shaders/SolidPixelShader.hlsl")));

		this->LightShader = Engine::IShader::Make(this->Device);
		this->LightShader->SetVertexShader(Engine::Compile(Engine::TypeVertexShader, Engine::ReadFile(BasePath + "/shaders/LightVertexShader.hlsl")));
		this->LightShader->SetPixelShader(Engine::Compile(Engine::TypePixelShader, Engine::ReadFile(BasePath + "/shaders/LightPixelShader.hlsl")));

		this->SkyBox = std::make_unique<GameObject>(this->Device, Engine::ReadFile(BasePath + "/resources/SkyBox.obj"), Engine::ReadFile(BasePath + "/resources/SkyBox.png"));
		this->Cube = std::make_unique<GameObject>(this->Device, Engine::ReadFile(BasePath + "/resources/Cube.obj"), Engine::ReadFile(BasePath + "/resources/Cube.png"));
		this->Sphere = std::make_unique<GameObject>(this->Device, Engine::ReadFile(BasePath + "/resources/Sphere.obj"), Engine::ReadFile(BasePath + "/resources/Sphere.png"));
	}

	void Update(const std::unique_ptr<Engine::IApplication>& App) override
	{
		const double Time = Engine::GetTime();

		static double LastTime = Time;
		const double DeltaTime = Time - LastTime;
		LastTime = Time;

		this->Device->Clear(0.5f, 0.5f, 0.5f, 1.0f);



		const DirectX::XMVECTOR Forward = DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), DirectX::XMMatrixRotationRollPitchYaw(0.0f, this->Mouse.x, 0.0f));
		const DirectX::XMVECTOR Left = DirectX::XMVector3Transform(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), DirectX::XMMatrixRotationRollPitchYaw(0.0f, this->Mouse.x, 0.0f));
		const DirectX::XMVECTOR Up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);



		DirectX::BoundingSphere CameraCollider(DirectX::XMFLOAT3(DirectX::XMVectorGetX(this->Camera->Eye), DirectX::XMVectorGetY(this->Camera->Eye), DirectX::XMVectorGetZ(this->Camera->Eye)), 0.5f);



		this->Cube->GetTransform() = DirectX::XMMatrixTranslation(0.0f, -2.0f, 0.0f) * DirectX::XMMatrixScaling(2.0f, 1.0f, 2.0f);
		DirectX::BoundingOrientedBox CubeCollider;
		DirectX::BoundingOrientedBox().Transform(CubeCollider, this->Cube->GetTransform());



		const float Speed = (this->Keys[VK_SHIFT] ? 2.0f : (this->Keys[VK_CONTROL] ? 0.5f : 1.0f)) * 5.0f * static_cast<float>(DeltaTime);

		DirectX::XMVECTOR Delta = DirectX::XMVectorReplicate(0.0f);
		const DirectX::XMVECTOR DeltaForward = Forward;
		const DirectX::XMVECTOR DeltaBack = DirectX::XMVectorMultiply(DeltaForward, DirectX::XMVectorReplicate(-1.0f));
		const DirectX::XMVECTOR DeltaLeft = Left;
		const DirectX::XMVECTOR DeltaRight = DirectX::XMVectorMultiply(DeltaLeft, DirectX::XMVectorReplicate(-1.0f));
		const DirectX::XMVECTOR DeltaUp = Up;
		const DirectX::XMVECTOR DeltaDown = DirectX::XMVectorMultiply(DeltaUp, DirectX::XMVectorReplicate(-1.0f));

		auto PlayerMove = [&](DirectX::BoundingSphere& Player, const DirectX::XMVECTOR& Delta, const float Speed)
		{
			const DirectX::XMFLOAT3 DeltaFloat3(DirectX::XMVectorGetX(Delta) * Speed, DirectX::XMVectorGetY(Delta) * Speed, DirectX::XMVectorGetZ(Delta) * Speed);

			Player.Center.x += DeltaFloat3.x;
			Player.Center.y += DeltaFloat3.y;
			Player.Center.z += DeltaFloat3.z;

			if (Player.Intersects(CubeCollider))
			{
				Player.Center.x -= DeltaFloat3.x;
				Player.Center.y -= DeltaFloat3.y;
				Player.Center.z -= DeltaFloat3.z;
			}
		};

		if (this->Keys['W']) Delta = DirectX::XMVectorAdd(Delta, DeltaForward);
		if (this->Keys['S']) Delta = DirectX::XMVectorAdd(Delta, DeltaBack);
		if (this->Keys['D']) Delta = DirectX::XMVectorAdd(Delta, DeltaLeft);
		if (this->Keys['A']) Delta = DirectX::XMVectorAdd(Delta, DeltaRight);
		if (this->Keys['E']) Delta = DirectX::XMVectorAdd(Delta, DeltaUp);
		if (this->Keys['Q']) Delta = DirectX::XMVectorAdd(Delta, DeltaDown);

		PlayerMove(CameraCollider, DirectX::XMVector3Normalize(Delta), Speed);



		this->Camera->Eye = DirectX::XMLoadFloat3(&CameraCollider.Center);
		this->Camera->UpdateView(this->Mouse.x, this->Mouse.y);
		this->SkyBox->GetTransform() = DirectX::XMMatrixTranslationFromVector(this->Camera->Eye);

		this->Camera->UpdateTransform() = this->SkyBox->GetTransform();
		this->Camera->UpdateBuffers();
		this->SolidShader->Set();
		this->SkyBox->Set();
		this->Device->Draw();

		this->Camera->UpdateTransform() = this->Cube->GetTransform();
		this->Camera->UpdateBuffers();
		this->LightShader->Set();
		this->Cube->Set();
		this->Device->Draw();

		this->Sphere->GetTransform() = DirectX::XMMatrixTranslation(10.0f, 10.0f, 10.0f);
		this->Camera->UpdateTransform() = this->Sphere->GetTransform();
		this->Camera->UpdateBuffers();
		this->LightShader->Set();
		this->Sphere->Set();
		this->Device->Draw();

		this->Device->Present();

		static size_t FrameCounter = 0;
		FrameCounter++;
		static double TimeElapsed = 0.0;
		TimeElapsed += DeltaTime;

		if (TimeElapsed >= 0.5)
		{
			constexpr size_t StringSize = 0xff;
			char String[StringSize] = {};
			std::snprintf(String, StringSize, "Game %0.1f fps", FrameCounter / TimeElapsed);
			App->SetTitle(String);
			TimeElapsed = 0.0;
			FrameCounter = 0;
		}
	}

	void Resize(const std::unique_ptr<Engine::IApplication>& App, int Width, int Height) override
	{
		if (this->Device)
		{
			this->Device->ResizeSwapChain(Width, Height);
			this->Camera->UpdateProjection(DirectX::XM_PIDIV2, static_cast<float>(Width) / Height);
		}
	}

	void MouseMove(const std::unique_ptr<Engine::IApplication>& App, int DeltaX, int DeltaY) override
	{
		Mouse.x += DirectX::XMConvertToRadians(static_cast<float>(DeltaX * 0.1));
		Mouse.y += DirectX::XMConvertToRadians(static_cast<float>(DeltaY * 0.1));
		constexpr float MinRadian = DirectX::XMConvertToRadians(-85.0f), MaxRadian = DirectX::XMConvertToRadians(85.0f);
		Mouse.y = std::clamp(Mouse.y, MinRadian, MaxRadian);
	}

	void Keyboard(const std::unique_ptr<Engine::IApplication>& App, unsigned char KeyCode, bool Action) override
	{
		this->Keys[KeyCode] = Action;
		if (KeyCode == VK_ESCAPE) App->Close();
	}
};



int EntryPoint()
{
#ifndef NDEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	return Engine::Run<Main>();
}

int main()
{
	return EntryPoint();
}

#ifdef WIN32
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	return EntryPoint();
}
#endif
