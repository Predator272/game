#include "engine/resource.hpp"
#include "stb_image.h"

namespace Engine
{
	class CTextureLoader: public ITextureLoader
	{
	private:
		unsigned int Width = 0, Height = 0;
		std::vector<uint32_t> Pixels;
	public:
		CTextureLoader(const std::string& Data)
		{
			int Width = 0, Height = 0, Channels = 0;
			uint32_t* Pixels = reinterpret_cast<uint32_t*>(stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(Data.data()), static_cast<int>(Data.size()), &Width, &Height, &Channels, 4));
			if (!Pixels || Width <= 0 || Height <= 0 || Channels != 4) throw std::runtime_error("Failed load texture");

			this->Width = Width;
			this->Height = Height;
			this->Pixels = std::vector<uint32_t>(Pixels, Pixels + (this->Width * this->Height));

			stbi_image_free(Pixels);
		}
	public:
		unsigned int GetWidth() const override
		{
			return this->Width;
		}

		unsigned int GetHeight() const override
		{
			return this->Height;
		}

		const std::vector<uint32_t>& GetPixels() const override
		{
			return this->Pixels;
		}
	};

	std::unique_ptr<ITextureLoader> ITextureLoader::LoadFromMemory(const std::string& Data)
	{
		return std::make_unique<CTextureLoader>(Data);
	}
}
