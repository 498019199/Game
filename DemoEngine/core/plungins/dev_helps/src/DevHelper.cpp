#include <base/DevHelper.h>
#include <dev_helps/TexConverter.h>
#include <string_view>


namespace RenderWorker
{
class DevHelperImp final : public DevHelper
{
public:
    ~DevHelperImp() override
    {
    }

    void GetImageInfo(std::string_view input_name, std::string_view metadata_name, RenderDeviceCaps const * caps,
        Texture::TextureType& type,
        uint32_t& width, uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size,
        ElementFormat& format, uint32_t& row_pitch, uint32_t& slice_pitch) override
    {
        std::string metadata_name_ptr(metadata_name);
        if (metadata_name.empty())
        {
            metadata_name_ptr = std::string(input_name) + ".kmeta";
        }

        auto metadata = this->LoadTexMetadata(metadata_name_ptr, caps);

        TexConverter tc;
        tc.GetImageInfo(metadata, type, width, height, depth, num_mipmaps, array_size, format, row_pitch, slice_pitch);
    }

private:
    TexMetadata LoadTexMetadata(std::string_view metadata_name, RenderDeviceCaps const * caps)
    {
        TexMetadata metadata;
        if (!metadata_name.empty())
        {
            metadata.Load(metadata_name);
        }
        if (caps)
        {
            metadata.DeviceDependentAdjustment(*caps);
        }

        return metadata;
    }
};
}

extern "C"
{
	ZENGINE_SYMBOL_EXPORT void MakeDevHelper(std::unique_ptr<RenderWorker::DevHelper>& ptr)
	{
		ptr = RenderWorker::MakeUniquePtr<RenderWorker::DevHelperImp>();
	}
}