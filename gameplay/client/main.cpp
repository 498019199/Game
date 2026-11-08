#include "GameSession.h"
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace
{
bool ReadFrame(std::istream& input, std::string& frame)
{
    std::array<unsigned char, 4> header{};
    input.read(reinterpret_cast<char*>(header.data()), 4);
    if (input.gcount() == 0 && input.eof()) return false;
    if (input.gcount() != 4) throw std::runtime_error("Truncated frame header");
    const std::uint32_t size = (std::uint32_t(header[0]) << 24) | (std::uint32_t(header[1]) << 16)
        | (std::uint32_t(header[2]) << 8) | header[3];
    if (!size || size > Gameplay::MaxFrameBytes) throw std::runtime_error("Invalid frame size");
    frame.resize(size);
    input.read(frame.data(), size);
    if (input.gcount() != size) throw std::runtime_error("Truncated frame body");
    return true;
}
void WriteFrame(std::ostream& output, const std::string& frame)
{
    auto size = static_cast<std::uint32_t>(frame.size());
    const std::array<unsigned char, 4> header{static_cast<unsigned char>(size >> 24),
        static_cast<unsigned char>(size >> 16), static_cast<unsigned char>(size >> 8), static_cast<unsigned char>(size)};
    output.write(reinterpret_cast<const char*>(header.data()), 4);
    output.write(frame.data(), size);
    if (!output) throw std::runtime_error("Failed to write response frame");
}
}
int main(int argc, char** argv)
{
    try
    {
        auto session = Gameplay::GameSession::CreateDefault();
        if (argc == 4 && std::string_view(argv[1]) == "--replay")
        {
            std::ifstream input(argv[2], std::ios::binary);
            std::ofstream output(argv[3], std::ios::binary);
            if (!input || !output) throw std::runtime_error("Cannot open replay input/output");
            std::string frame;
            while (ReadFrame(input, frame)) WriteFrame(output, session->Exchange(frame));
            return 0;
        }
        if (argc == 2 && std::string_view(argv[1]) == "--check-isolation")
        {
            Gameplay::Protocol::Request create;
            auto* definition = create.mutable_create_entity();
            definition->set_id(1); definition->set_max_hp(100); definition->set_attack(10);
            auto one = session->Call(create);
            if (one.status() != Gameplay::Protocol::ERROR_CODE_OK) throw std::runtime_error("create failed");
            for (int i = 0; i < 20; ++i)
            {
                auto other = Gameplay::GameSession::CreateDefault();
                auto result = other->Call(create);
                if (result.status() != Gameplay::Protocol::ERROR_CODE_OK || result.entities_size() != 1)
                    throw std::runtime_error("managed handle/world isolation failed");
            }
            Gameplay::Protocol::Request snapshot;
            snapshot.mutable_snapshot();
            if (session->Call(snapshot).entities_size() != 1) throw std::runtime_error("other world disposal corrupted session");
            std::cout << "PASS native managed-world isolation and repeated create/destroy\n";
            return 0;
        }
        if (argc == 1)
        {
            for (const char* command : {"/gas smoke", "/gas skillsmoke"})
            {
                Gameplay::Protocol::Request request;
                request.mutable_gm()->set_command(command);
                auto response = session->Call(std::move(request));
                if (response.status() != Gameplay::Protocol::ERROR_CODE_OK) throw std::runtime_error(response.error());
                std::cout << response.message() << '\n';
            }
            return 0;
        }
        throw std::runtime_error("Usage: gameplay_client [--replay INPUT OUTPUT]");
    }
    catch (const std::exception& error) { std::cerr << "Gameplay: " << error.what() << '\n'; return 2; }
}
