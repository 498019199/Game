#include <common/ResIdentifier.h>
#include <string_view>

struct IInArchive;

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