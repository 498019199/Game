#include <core/IApp.h>
#include <core/IContext.h>

#include <cassert>
#include <filesystem>
#include <mini/ini.h>

#include <render/IRenderModel.h>
using namespace CoreWorker;

void test_ini(std::string pathfile)
{
    std::filesystem::path filepath = pathfile + "\\config.ini";
    mINI::INIFile file(filepath.string().c_str());
    mINI::INIStructure ini;
    COMMON_ASSERT(file.read(ini) == true);
    COMMON_ASSERT(ini["graphics"]["width"] == "1920");
    COMMON_ASSERT(ini["graphics"]["height"] == "1080");
    COMMON_ASSERT(ini["graphics"]["color_fmt"] == "ARGB8");
    COMMON_ASSERT(ini["graphics"]["keep_screen_on"] == "1");
}

void test_load_obj_file(std::string pathfile)
{
    auto model = CommonWorker::MakeSharedPtr<IRenderModel>();
    std::filesystem::path filepath = pathfile + "\\model\\centaur\\source\\centaur02.obj";
    model->LoadModel(ModelFileType::MODEL_FILE_OBJECT, filepath.string().c_str());
}

void test_load_fbx_file(std::string pathfile)
{}

void test_app(std::string pathfile)
{
    std::filesystem::path currentPath = std::filesystem::current_path().parent_path().parent_path();;
    std::filesystem::path filepath = pathfile + "\\config.ini";
    Context::Instance()->LoadConfig(filepath.string().c_str());
    auto pApp = Context::Instance()->CreateAppWindow();
    pApp->Create();
    pApp->Run();
    pApp->Close();
}

int main(int argc, char** argv)
{
    test_ini(argv[1]);
    test_load_obj_file(argv[1]);
    test_load_fbx_file(argv[1]);
    //test_app(argv[1]);

    return 0;
}