#include <common/ResIdentifier.h>
#include <string_view>

struct IInArchive;
// {23170F69-40C1-278A-1000-000110070000}
DEFINE_GUID(CLSID_CFormat7z,
    0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);
    
namespace RenderWorker
{

class Package final
{
public:
    explicit Package(ResIdentifierPtr const & archive_is);
    Package(ResIdentifierPtr const & archive_is, std::string_view password);

    bool Locate(std::string_view extract_file_path);
	ResIdentifierPtr Extract(std::string_view extract_file_path, std::string_view res_name);

    ResIdentifier* ArchiveStream() const;

private:
	uint32_t Find(std::string_view extract_file_path);

private:
    ResIdentifierPtr archive_is_;

    std::shared_ptr<IInArchive> archive_;
    std::string password_;

    uint32_t num_items_;
};
using PackagePtr = std::shared_ptr<Package>;
}