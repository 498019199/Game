#include <render/IRenderMaterial.h>

namespace CoreWorker
{
void IRenderMaterial::SetTextureName(TextureType solt, const std::string_view& name)
{
	textures_[solt].first = name;
}

const std::string& IRenderMaterial::GetTextureName(TextureType slot) const
{
    return textures_[slot].first;
}

void IRenderMaterial::LoadTextureSlots()
{
	for (size_t i = 0; i < IRenderMaterial::TS_NumTextureSlots; ++i)
	{
	    auto slot = static_cast<IRenderMaterial::TextureType>(i);
		auto const& tex_name = textures_[slot].first;
		if (!tex_name.empty())
        {
            // if (!res_loader.Locate(tex_name).empty()
			// 	|| !res_loader.Locate(tex_name + ".dds").empty())
			// {
            //     SetTexture(i, );
            // }
        }
    }
}

void IRenderMaterial::SetTexture(TextureType slot, ShaderResourceViewPtr srv)
{
    switch (slot)
    {
    case TextureType::TS_Albedo:
        {
            
        }
        break;
    case TextureType::TS_MetalnessGlossiness:
        {
            
        }
        break;
    case TextureType::TS_Emissive:
        {
            
        }
        break;
    case TextureType::TS_Normal:
        {
            
        }
        break;
    case TextureType::TS_Height:
        {
            
        }
        break;
    case TextureType::TS_Occlusion:
        break;
    default:
        LOGER_ERROR() << ("Invalid texture slot");
    }

    textures_[slot].second = std::move(srv);
}
}
