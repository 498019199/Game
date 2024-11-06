#pragma once

#include <core/define.h>
#include <render/ElementFormat.h>
namespace CoreWorker
{
enum class RenderType
{
	RENDER_TYPE_NONE = 0,
	RENDER_TYPE_WIREFRAME = 1, // 渲染线框
	RENDER_TYPE_TEXTURE = 2,	//渲染纹理
};

struct FWindowDesc
{
	RenderType m_nRenderType = RenderType::RENDER_TYPE_NONE;

	bool	hide_win = false;
	bool	full_screen = false;

	int		left = 0;
	int		top = 0;
	int		width = 0;
	int		height = 0;

    ElementFormat color_fmt;
    ElementFormat depth_stencil_fmt;
	uint32_t sample_count;
	uint32_t sample_quality;

    bool    keep_screen_on = false;
};

class IApp
{
public:
	IApp() = default;
    ~IApp() = default;
    
	virtual void Create() = 0;
    virtual void Run() = 0;
    virtual void Close() = 0;
protected:
	std::wstring        swcName_;
	int32_t             left_ = 0;
	int32_t             top_ = 0;
	uint32_t            width_ = 0;
	uint32_t            nHeight_ = 0;

    bool                active_ = false;
	bool                ready_ = false;
	bool                closed_ = false;
	bool                keep_screen_on_ = false;   
    bool                hide_ = false;

    float               dpi_scale_ = 1.0f;
};

using ShareAppPtr = std::shared_ptr<IApp>;
}

