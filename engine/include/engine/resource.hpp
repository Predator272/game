#pragma once

#include "types.hpp"
#include <directxmath.h>

namespace Engine
{
	class ITextureLoader: public IBaseInterface
	{
	public:
		static std::unique_ptr<ITextureLoader> LoadFromMemory(const std::string& Data);
	public:
		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;
		virtual const std::vector<uint32_t>& GetPixels() const = 0;
	};

	class IMeshLoader: public IBaseInterface
	{
	public:
		static std::unique_ptr<IMeshLoader> LoadFromMemory(const std::string& Data);
	public:
		virtual const std::vector<DirectX::XMFLOAT3>& GetPositions() const = 0;
		virtual const std::vector<DirectX::XMFLOAT2>& GetTexCoords() const = 0;
		virtual const std::vector<DirectX::XMFLOAT3>& GetNormals() const = 0;
		virtual const std::vector<unsigned int>& GetIndices() const = 0;
	};
}
