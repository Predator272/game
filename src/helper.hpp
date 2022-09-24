#pragma once

#include "engine/engine.hpp"
#include "engine/resource.hpp"

namespace Engine
{
	enum ShaderType
	{
		TypeVertexShader,
		TypePixelShader,
	};

	std::string Compile(ShaderType Type, const std::string& Source)
	{
		const char* ShaderModel[] = { "vs_5_0", "ps_5_0" };
		Microsoft::WRL::ComPtr<ID3DBlob> Data;
		D3DCompile(Source.data(), Source.size(), nullptr, nullptr, nullptr, "main", ShaderModel[Type], 0, 0, Data.ReleaseAndGetAddressOf(), nullptr);
		const char* BufferPointer = static_cast<char*>(Data->GetBufferPointer());
		return std::string(BufferPointer, BufferPointer + Data->GetBufferSize());
	}



	class IDevice: public IBaseInterface
	{
	public:
		static std::unique_ptr<IDevice> Make(const std::unique_ptr<IApplication>& App);
	public:
		virtual void ResizeSwapChain(int Width, int Height) = 0;
		virtual void Clear(float R, float G, float B, float A) = 0;
		virtual void Present(unsigned int SyncInterval = 0) = 0;
		virtual void Draw() = 0;
	};

	class CDevice: public IDevice
	{
	public:
		Microsoft::WRL::ComPtr<ID3D11Device> Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView;

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> RasterizerState;
		Microsoft::WRL::ComPtr<ID3D11BlendState> BlendState;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerState;
	public:
		CDevice(const std::unique_ptr<IApplication>& App)
		{
			D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, this->Device.ReleaseAndGetAddressOf(), nullptr, this->DeviceContext.ReleaseAndGetAddressOf());

			this->CreateSwapChain(App);

			const D3D11_RASTERIZER_DESC RasterizerDesc = {
				.FillMode = D3D11_FILL_SOLID,
				.CullMode = D3D11_CULL_BACK,
			};
			this->Device->CreateRasterizerState(&RasterizerDesc, this->RasterizerState.ReleaseAndGetAddressOf());
			this->DeviceContext->RSSetState(this->RasterizerState.Get());

			const D3D11_BLEND_DESC BlendDesc = {
				.RenderTarget = {
					{
						.BlendEnable = true,

						.SrcBlend = D3D11_BLEND_SRC_ALPHA,
						.DestBlend = D3D11_BLEND_INV_SRC_ALPHA,
						.BlendOp = D3D11_BLEND_OP_ADD,

						.SrcBlendAlpha = D3D11_BLEND_ZERO,
						.DestBlendAlpha = D3D11_BLEND_ZERO,
						.BlendOpAlpha = D3D11_BLEND_OP_ADD,

						.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL,
					},
				},
			};
			this->Device->CreateBlendState(&BlendDesc, this->BlendState.ReleaseAndGetAddressOf());
			this->DeviceContext->OMSetBlendState(this->BlendState.Get(), nullptr, 0xffffffff);

			const D3D11_SAMPLER_DESC SamplerDesc = {
				.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
				.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
				.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
				.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
				.ComparisonFunc = D3D11_COMPARISON_NEVER,
				.MinLOD = 0.0f,
				.MaxLOD = D3D11_FLOAT32_MAX,
			};
			this->Device->CreateSamplerState(&SamplerDesc, this->SamplerState.ReleaseAndGetAddressOf());
			this->DeviceContext->PSSetSamplers(0, 1, this->SamplerState.GetAddressOf());
		}

		~CDevice()
		{
			this->DeviceContext->ClearState();
		}
	public:
		void ResizeSwapChain(int Width, int Height) override
		{
			this->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
			this->RenderTargetView.Reset();
			this->DepthStencilView.Reset();
			this->SwapChain->ResizeBuffers(0, Width, Height, DXGI_FORMAT_UNKNOWN, 0);
			this->CreateView();
		}

		void Clear(float R, float G, float B, float A) override
		{
			const float ClearColor[4] = { R, G, B, A };
			this->DeviceContext->ClearRenderTargetView(this->RenderTargetView.Get(), ClearColor);
			this->DeviceContext->ClearDepthStencilView(this->DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}

		void Present(unsigned int SyncInterval) override
		{
			this->SwapChain->Present(SyncInterval, 0);
		}

		void Draw() override
		{
			Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer;
			this->DeviceContext->IAGetVertexBuffers(0, 1, Buffer.GetAddressOf(), nullptr, nullptr);
			if (Buffer)
			{
				D3D11_BUFFER_DESC BufferDesc = {};
				Buffer->GetDesc(&BufferDesc);
				this->DeviceContext->Draw(BufferDesc.ByteWidth / (sizeof(unsigned int) * 3), 0);
			}
		}
	private:
		void CreateSwapChain(const std::unique_ptr<Engine::IApplication>& App)
		{
			Microsoft::WRL::ComPtr<IDXGIDevice> DXGIDevice;
			Microsoft::WRL::ComPtr<IDXGIAdapter> DXGIAdapter;
			Microsoft::WRL::ComPtr<IDXGIFactory> DXGIFactory;

			DXGI_SWAP_CHAIN_DESC SwapChainDesc = {
				.BufferDesc = {
					.Width = static_cast<unsigned int>(App->GetWidth()),
					.Height = static_cast<unsigned int>(App->GetHeight()),
					.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				},
				.SampleDesc = {
					.Count = 4,
					.Quality = 0,
				},
				.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
				.BufferCount = 1,
				.OutputWindow = reinterpret_cast<HWND>(App->GetHandle()),
				.Windowed = true,
			};

			this->Device.As(&DXGIDevice);
			DXGIDevice->GetAdapter(DXGIAdapter.GetAddressOf());
			DXGIAdapter->GetParent(IID_PPV_ARGS(DXGIFactory.GetAddressOf()));
			DXGIFactory->CreateSwapChain(this->Device.Get(), &SwapChainDesc, this->SwapChain.ReleaseAndGetAddressOf());
			DXGIFactory->MakeWindowAssociation(SwapChainDesc.OutputWindow, DXGI_MWA_NO_ALT_ENTER);

			this->CreateView();
		}

		void CreateView()
		{
			Microsoft::WRL::ComPtr<ID3D11Texture2D> BackBuffer;
			this->SwapChain->GetBuffer(0, IID_PPV_ARGS(BackBuffer.GetAddressOf()));
			this->Device->CreateRenderTargetView(BackBuffer.Get(), nullptr, this->RenderTargetView.ReleaseAndGetAddressOf());

			D3D11_TEXTURE2D_DESC BackBufferDesc = {};
			BackBuffer->GetDesc(&BackBufferDesc);

			Microsoft::WRL::ComPtr<ID3D11Texture2D> DepthStencilBuffer;
			D3D11_TEXTURE2D_DESC DepthDesc = {
				.Width = BackBufferDesc.Width,
				.Height = BackBufferDesc.Height,
				.MipLevels = 1,
				.ArraySize = 1,
				.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
				.SampleDesc = {
					.Count = BackBufferDesc.SampleDesc.Count,
					.Quality = BackBufferDesc.SampleDesc.Quality,
				},
				.BindFlags = D3D11_BIND_DEPTH_STENCIL,
			};
			this->Device->CreateTexture2D(&DepthDesc, nullptr, DepthStencilBuffer.GetAddressOf());

			const D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {
				.Format = DepthDesc.Format,
				.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS,
			};
			this->Device->CreateDepthStencilView(DepthStencilBuffer.Get(), &DepthStencilViewDesc, this->DepthStencilView.ReleaseAndGetAddressOf());

			this->DeviceContext->OMSetRenderTargets(1, this->RenderTargetView.GetAddressOf(), this->DepthStencilView.Get());

			const D3D11_VIEWPORT ViewPort = {
				.Width = static_cast<FLOAT>(BackBufferDesc.Width),
				.Height = static_cast<FLOAT>(BackBufferDesc.Height),
				.MinDepth = D3D11_MIN_DEPTH,
				.MaxDepth = D3D11_MAX_DEPTH,
			};
			this->DeviceContext->RSSetViewports(1, &ViewPort);

			this->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
	};

	std::unique_ptr<IDevice> IDevice::Make(const std::unique_ptr<IApplication>& App)
	{
		return std::make_unique<CDevice>(App);
	}



	class CDeviceChild
	{
	protected:
		Microsoft::WRL::ComPtr<ID3D11Device> Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;
	public:
		CDeviceChild(Microsoft::WRL::ComPtr<ID3D11Device> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext): Device(Device), DeviceContext(DeviceContext)
		{
		}
	};



	class IShader: public IBaseInterface
	{
	public:
		static std::unique_ptr<IShader> Make(const std::unique_ptr<IDevice>& Device);
	public:
		virtual void SetVertexShader(const std::string& Data) = 0;
		virtual void SetPixelShader(const std::string& Data) = 0;
		virtual void Set() = 0;
	};

	class CShader: public CDeviceChild, public IShader
	{
	private:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
	public:
		CShader(const CDevice* Device): CDeviceChild(Device->Device, Device->DeviceContext)
		{
		}

		void SetVertexShader(const std::string& Data)
		{
			Microsoft::WRL::ComPtr<ID3D11ShaderReflection> ShaderReflection;
			D3DReflect(Data.data(), Data.size(), IID_PPV_ARGS(ShaderReflection.GetAddressOf()));

			D3D11_SHADER_DESC ShaderDesc = {};
			ShaderReflection->GetDesc(&ShaderDesc);

			constexpr DXGI_FORMAT Format[][4] = {
				{ DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_FLOAT },
				{ DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_FLOAT },
				{ DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R32G32B32_UINT, DXGI_FORMAT_R32G32B32_SINT, DXGI_FORMAT_R32G32B32_FLOAT },
				{ DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_FLOAT },
			};
			constexpr unsigned char FormatIndex[] = { 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3 };

			D3D11_INPUT_ELEMENT_DESC InputElementDesc[D3D11_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT] = {};
			for (UINT Index = 0; Index < ShaderDesc.InputParameters; Index++)
			{
				D3D11_SIGNATURE_PARAMETER_DESC SignatureParameterDesc = {};
				ShaderReflection->GetInputParameterDesc(Index, &SignatureParameterDesc);

				InputElementDesc[Index].SemanticName = SignatureParameterDesc.SemanticName;
				InputElementDesc[Index].SemanticIndex = SignatureParameterDesc.SemanticIndex;
				InputElementDesc[Index].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
				InputElementDesc[Index].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				InputElementDesc[Index].Format = Format[FormatIndex[SignatureParameterDesc.Mask]][SignatureParameterDesc.ComponentType];
			}

			this->Device->CreateInputLayout(InputElementDesc, ShaderDesc.InputParameters, Data.data(), Data.size(), this->InputLayout.ReleaseAndGetAddressOf());
			this->Device->CreateVertexShader(Data.data(), Data.size(), nullptr, this->VertexShader.ReleaseAndGetAddressOf());
		}

		void SetPixelShader(const std::string& Data)
		{
			this->Device->CreatePixelShader(Data.data(), Data.size(), nullptr, this->PixelShader.ReleaseAndGetAddressOf());
		}

		void Set()
		{
			if (this->InputLayout != this->GetCurrentInputLayout()) this->DeviceContext->IASetInputLayout(this->InputLayout.Get());
			if (this->VertexShader != this->GetCurrentVertexShader()) this->DeviceContext->VSSetShader(this->VertexShader.Get(), nullptr, 0);
			if (this->PixelShader != this->GetCurrentPixelShader()) this->DeviceContext->PSSetShader(this->PixelShader.Get(), nullptr, 0);
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> GetCurrentInputLayout()
		{
			Microsoft::WRL::ComPtr<ID3D11InputLayout> CurrentInputLayout;
			this->DeviceContext->IAGetInputLayout(CurrentInputLayout.GetAddressOf());
			return CurrentInputLayout;
		}

		Microsoft::WRL::ComPtr<ID3D11VertexShader> GetCurrentVertexShader()
		{
			Microsoft::WRL::ComPtr<ID3D11VertexShader> CurrentVertexShader;
			this->DeviceContext->VSGetShader(CurrentVertexShader.GetAddressOf(), nullptr, nullptr);
			return CurrentVertexShader;
		}

		Microsoft::WRL::ComPtr<ID3D11PixelShader> GetCurrentPixelShader()
		{
			Microsoft::WRL::ComPtr<ID3D11PixelShader> CurrentPixelShader;
			this->DeviceContext->PSGetShader(CurrentPixelShader.GetAddressOf(), nullptr, nullptr);
			return CurrentPixelShader;
		}
	};

	std::unique_ptr<IShader> IShader::Make(const std::unique_ptr<IDevice>& Device)
	{
		return std::make_unique<CShader>(dynamic_cast<CDevice*>(Device.get()));
	}



	class IConstantBuffer: public IBaseInterface
	{
	public:
		static std::unique_ptr<IConstantBuffer> Make(const std::unique_ptr<Engine::IDevice>& Device, unsigned int Size);
	public:
		virtual void Set() = 0;
		virtual void Update(const void* Data) = 0;
	};

	class CConstantBuffer: public CDeviceChild, public IConstantBuffer
	{
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBuffer;
	public:
		CConstantBuffer(const CDevice* Device, unsigned int Size): CDeviceChild(Device->Device, Device->DeviceContext)
		{
			const D3D11_BUFFER_DESC BufferDesc = {
				.ByteWidth = Size,
				.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			};
			this->Device->CreateBuffer(&BufferDesc, nullptr, this->ConstantBuffer.ReleaseAndGetAddressOf());
		}

		void Set() override
		{
			if (this->ConstantBuffer != this->GetCurrentConstantBuffer()) this->DeviceContext->VSSetConstantBuffers(0, 1, this->ConstantBuffer.GetAddressOf());
		}

		void Update(const void* Data) override
		{
			this->DeviceContext->UpdateSubresource(this->ConstantBuffer.Get(), 0, nullptr, Data, 0, 0);
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> GetCurrentConstantBuffer()
		{
			Microsoft::WRL::ComPtr<ID3D11Buffer> CurrentConstantBuffer;
			this->DeviceContext->VSGetConstantBuffers(0, 1, CurrentConstantBuffer.GetAddressOf());
			return CurrentConstantBuffer;
		}
	};

	std::unique_ptr<IConstantBuffer> IConstantBuffer::Make(const std::unique_ptr<Engine::IDevice>& Device, unsigned int Size)
	{
		return std::make_unique<CConstantBuffer>(dynamic_cast<CDevice*>(Device.get()), Size);
	}



	class IMesh: public IBaseInterface
	{
	public:
		static std::unique_ptr<IMesh> Make(const std::unique_ptr<Engine::IDevice>& Device, const std::vector<unsigned int>& Indices, const std::vector<DirectX::XMFLOAT3>& Positions, const std::vector<DirectX::XMFLOAT2>& TexCoords, const std::vector<DirectX::XMFLOAT3>& Normals);
	public:
		virtual void Set() = 0;
	};

	class CMesh: public CDeviceChild, public IMesh
	{
	private:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceView[3];
		Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
	public:
		CMesh(const CDevice* Device, const std::vector<unsigned int>& Indices, const std::vector<DirectX::XMFLOAT3>& Positions, const std::vector<DirectX::XMFLOAT2>& TexCoords, const std::vector<DirectX::XMFLOAT3>& Normals): CDeviceChild(Device->Device, Device->DeviceContext)
		{
			this->ShaderResourceView[0] = this->CreateShaderResourceView(sizeof(Positions[0]), Positions.size(), Positions.data());
			this->ShaderResourceView[1] = this->CreateShaderResourceView(sizeof(TexCoords[0]), TexCoords.size(), TexCoords.data());
			this->ShaderResourceView[2] = this->CreateShaderResourceView(sizeof(Normals[0]), Normals.size(), Normals.data());

			const D3D11_BUFFER_DESC BufferDesc = {
				.ByteWidth = static_cast<UINT>(sizeof(Indices[0]) * Indices.size()),
				.BindFlags = D3D11_BIND_VERTEX_BUFFER,
			};
			const D3D11_SUBRESOURCE_DATA InitData = { .pSysMem = Indices.data(), };
			this->Device->CreateBuffer(&BufferDesc, &InitData, this->IndexBuffer.ReleaseAndGetAddressOf());
		}

		void Set() override
		{
			this->DeviceContext->VSSetShaderResources(0, 1, this->ShaderResourceView[0].GetAddressOf());
			this->DeviceContext->VSSetShaderResources(1, 1, this->ShaderResourceView[1].GetAddressOf());
			this->DeviceContext->VSSetShaderResources(2, 1, this->ShaderResourceView[2].GetAddressOf());

			const UINT Stride = sizeof(unsigned int) * 3, Offset = 0;
			this->DeviceContext->IASetVertexBuffers(0, 1, this->IndexBuffer.GetAddressOf(), &Stride, &Offset);
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateShaderResourceView(unsigned int Stride, unsigned int Count, const void* Data)
		{
			Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer;
			const D3D11_BUFFER_DESC BufferDesc = {
				.ByteWidth = Stride * Count,
				.BindFlags = D3D11_BIND_SHADER_RESOURCE,
				.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
				.StructureByteStride = Stride,
			};
			const D3D11_SUBRESOURCE_DATA InitData = { .pSysMem = Data };
			this->Device->CreateBuffer(&BufferDesc, &InitData, Buffer.ReleaseAndGetAddressOf());

			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
			const D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc = {
				.ViewDimension = D3D11_SRV_DIMENSION_BUFFER,
				.Buffer = {
					.ElementWidth = Count,
				},
			};
			this->Device->CreateShaderResourceView(Buffer.Get(), &ShaderResourceViewDesc, ShaderResourceView.ReleaseAndGetAddressOf());

			return ShaderResourceView;
		}
	};

	std::unique_ptr<IMesh> IMesh::Make(const std::unique_ptr<Engine::IDevice>& Device, const std::vector<unsigned int>& Indices, const std::vector<DirectX::XMFLOAT3>& Positions, const std::vector<DirectX::XMFLOAT2>& TexCoords, const std::vector<DirectX::XMFLOAT3>& Normals)
	{
		return std::make_unique<CMesh>(dynamic_cast<CDevice*>(Device.get()), Indices, Positions, TexCoords, Normals);
	}



	class ITexture: public IBaseInterface
	{
	public:
		static std::unique_ptr<ITexture> Make(const std::unique_ptr<Engine::IDevice>& Device, unsigned int Width, unsigned int Height, const std::vector<unsigned int>& Pixels);
	public:
		virtual void Set() = 0;
	};

	class CTexture: public CDeviceChild, public ITexture
	{
	private:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
	public:
		CTexture(const CDevice* Device, unsigned int Width, unsigned int Height, const std::vector<unsigned int>& Pixels): CDeviceChild(Device->Device, Device->DeviceContext)
		{
			Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture;

			const D3D11_TEXTURE2D_DESC TextureDesc = {
				.Width = Width,
				.Height = Height,
				.MipLevels = 0,
				.ArraySize = 1,
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.SampleDesc = {
					.Count = 1,
					.Quality = 0,
				},
				.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
				.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS,
			};
			this->Device->CreateTexture2D(&TextureDesc, nullptr, Texture.GetAddressOf());
			this->DeviceContext->UpdateSubresource(Texture.Get(), 0, nullptr, Pixels.data(), TextureDesc.Width * sizeof(Pixels[0]), 0);

			const D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc = {
				.Format = TextureDesc.Format,
				.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS,
			};
			this->Device->CreateShaderResourceView(Texture.Get(), &ShaderResourceViewDesc, this->ShaderResourceView.ReleaseAndGetAddressOf());
			this->DeviceContext->GenerateMips(this->ShaderResourceView.Get());
		}

		void Set() override
		{
			this->DeviceContext->PSSetShaderResources(0, 1, this->ShaderResourceView.GetAddressOf());
		}
	};

	std::unique_ptr<ITexture> ITexture::Make(const std::unique_ptr<Engine::IDevice>& Device, unsigned int Width, unsigned int Height, const std::vector<unsigned int>& Pixels)
	{
		return std::make_unique<CTexture>(dynamic_cast<CDevice*>(Device.get()), Width, Height, Pixels);
	}
}
