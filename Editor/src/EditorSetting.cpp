#include <editor/EditorPanel.h>
#include <algorithm>

namespace EditorWorker
{
void EditorSetting::SetWindowSize(uint32_t width, uint32_t height, int hierarchy, int project, int inspector)
{
    logical_width_ = width;
    logical_height_ = height;
    preferred_hierarchy_ = (std::max)(0, hierarchy);
    preferred_project_ = (std::max)(0, project);
    preferred_inspector_ = (std::max)(0, inspector);
    dpi_scale_ = 1.f;
    ResizeClient(width, height);
}

void EditorSetting::ApplyDpiScale(float scale)
{
    if (scale <= 0.f) return;
    dpi_scale_ = scale;
    ResizeClient(static_cast<uint32_t>(logical_width_ * scale + 0.5f),
        static_cast<uint32_t>(logical_height_ * scale + 0.5f));
}

void EditorSetting::ResizeClient(uint32_t width, uint32_t height)
{
    if (!width || !height) return; // Minimized windows must not create zero-sized GPU textures.
    auto scaled = [this](int value) { return static_cast<int>(value * dpi_scale_ + 0.5f); };
    srcWidth = static_cast<int>(width);
    srcHeight = static_cast<int>(height);
    mainBarWidth = srcWidth;
    mainBarHeight = (std::min)(scaled(58), srcHeight - 1);
    // Keep at least one pixel for the Game view, even below the normal usable window size.
    hierarchyWidth = scaled(preferred_hierarchy_);
    inspectorWidth = scaled(preferred_inspector_);
    int const sideWidth = hierarchyWidth + inspectorWidth;
    if (sideWidth > srcWidth - 1)
    {
        hierarchyWidth = static_cast<int>(static_cast<int64_t>(hierarchyWidth) * (srcWidth - 1) / sideWidth);
        inspectorWidth = srcWidth - 1 - hierarchyWidth;
    }
    projectHeight = (std::min)(scaled(preferred_project_), srcHeight - mainBarHeight - 1);
    gameViewWidth = srcWidth - hierarchyWidth - inspectorWidth;
    gameViewHeight = srcHeight - mainBarHeight - projectHeight;
    hierarchyHeight = gameViewHeight;
    consoleWidth = (gameViewWidth + hierarchyWidth) / 3;
    consoleHeight = projectHeight;
    projectWidth = gameViewWidth + hierarchyWidth - consoleWidth;
    inspectorHeight = gameViewHeight + projectHeight;
}
}
