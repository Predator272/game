#include "engine/resource.hpp"
#include "tiny_obj_loader.h"

namespace Engine
{
	class CMeshLoader: public IMeshLoader
	{
	private:
		std::vector<DirectX::XMFLOAT3> Positions;
		std::vector<DirectX::XMFLOAT2> TexCoords;
		std::vector<DirectX::XMFLOAT3> Normals;
		std::vector<unsigned int> Indices;
	public:
		CMeshLoader(const std::string& Data)
		{
			tinyobj::attrib_t Attributes;
			std::vector<tinyobj::shape_t> Shapes;
			std::vector<tinyobj::material_t> Materials;

			std::stringstream Stream(Data);
			if (!tinyobj::LoadObj(&Attributes, &Shapes, &Materials, nullptr, &Stream)) throw std::runtime_error("Failed load mesh");

			for (size_t i = 0; i < Attributes.vertices.size(); i += 3)
			{
				this->Positions.push_back(DirectX::XMFLOAT3(Attributes.vertices[i + 0], Attributes.vertices[i + 1], Attributes.vertices[i + 2]));
			}

			for (size_t i = 0; i < Attributes.texcoords.size(); i += 2)
			{
				this->TexCoords.push_back(DirectX::XMFLOAT2(Attributes.texcoords[i + 0], Attributes.texcoords[i + 1]));
			}

			for (size_t i = 0; i < Attributes.normals.size(); i += 3)
			{
				this->Normals.push_back(DirectX::XMFLOAT3(Attributes.normals[i + 0], Attributes.normals[i + 1], Attributes.normals[i + 2]));
			}

			for (const auto& Shape : Shapes)
			{
				for (const auto& Index : Shape.mesh.indices)
				{
					this->Indices.push_back(Index.vertex_index);
					this->Indices.push_back(Index.texcoord_index);
					this->Indices.push_back(Index.normal_index);
				}
			}
		}
	public:
		const std::vector<DirectX::XMFLOAT3>& GetPositions() const override
		{
			return this->Positions;
		}

		const std::vector<DirectX::XMFLOAT2>& GetTexCoords() const override
		{
			return this->TexCoords;
		}

		const std::vector<DirectX::XMFLOAT3>& GetNormals() const override
		{
			return this->Normals;
		}

		const std::vector<unsigned int>& GetIndices() const override
		{
			return this->Indices;
		}
	};

	std::unique_ptr<IMeshLoader> IMeshLoader::LoadFromMemory(const std::string& Data)
	{
		return std::make_unique<CMeshLoader>(Data);
	}
}
