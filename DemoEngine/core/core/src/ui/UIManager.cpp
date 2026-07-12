#include <base/UIManager.h>
#include <base/RmlUiRenderInterface.h>

#include <base/Context.h>
#include <base/ResLoader.h>
#include <base/App3D.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>
#include <common/Log.h>
#include <common/ResIdentifier.h>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementFormControlInput.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Debugger/Debugger.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <string>
#endif

namespace RenderWorker
{

class EditorRmlSystemInterface final : public ::Rml::SystemInterface
{
public:
	double GetElapsedTime() override
	{
		return Context::Instance().AppInstance().AppTime();
	}

#if defined(_WIN32)
	void SetClipboardText(Rml::String const& text_utf8) override
	{
		if (!::OpenClipboard(nullptr))
		{
			return;
		}

		::EmptyClipboard();

		int const wide_count = ::MultiByteToWideChar(CP_UTF8, 0, text_utf8.c_str(), static_cast<int>(text_utf8.size()), nullptr, 0);
		if (wide_count <= 0)
		{
			::CloseClipboard();
			return;
		}

		std::vector<wchar_t> wide_text(static_cast<size_t>(wide_count), L'\0');
		::MultiByteToWideChar(
			CP_UTF8, 0, text_utf8.c_str(), static_cast<int>(text_utf8.size()), wide_text.data(), wide_count);

		size_t const alloc_size = sizeof(wchar_t) * (static_cast<size_t>(wide_count) + 1);
		HGLOBAL clipboard_data = ::GlobalAlloc(GMEM_MOVEABLE, alloc_size);
		if (!clipboard_data)
		{
			::CloseClipboard();
			return;
		}

		void* dest = ::GlobalLock(clipboard_data);
		if (!dest)
		{
			::GlobalFree(clipboard_data);
			::CloseClipboard();
			return;
		}

		std::memcpy(dest, wide_text.data(), alloc_size);
		::GlobalUnlock(clipboard_data);

		if (::SetClipboardData(CF_UNICODETEXT, clipboard_data) == nullptr)
		{
			::GlobalFree(clipboard_data);
		}

		::CloseClipboard();
	}

	void GetClipboardText(Rml::String& text) override
	{
		text.clear();
		if (!::OpenClipboard(nullptr))
		{
			return;
		}

		HANDLE const clipboard_data = ::GetClipboardData(CF_UNICODETEXT);
		if (!clipboard_data)
		{
			::CloseClipboard();
			return;
		}

		wchar_t const* clipboard_text = static_cast<wchar_t const*>(::GlobalLock(clipboard_data));
		if (clipboard_text)
		{
			int const utf8_count =
				::WideCharToMultiByte(CP_UTF8, 0, clipboard_text, -1, nullptr, 0, nullptr, nullptr);
			if (utf8_count > 0)
			{
				text.resize(static_cast<size_t>(utf8_count - 1));
				::WideCharToMultiByte(CP_UTF8, 0, clipboard_text, -1, text.data(), utf8_count, nullptr, nullptr);
			}
			::GlobalUnlock(clipboard_data);
		}

		::CloseClipboard();
	}
#endif
};

class RmlUiFileInterface final : public ::Rml::FileInterface
{
public:
	Rml::FileHandle Open(Rml::String const& path) override
	{
		auto& res_loader = Context::Instance().ResLoaderInstance();
		ResIdentifierPtr file = res_loader.Open(path);
		if (!file)
		{
			std::ifstream* raw = new std::ifstream(path.c_str(), std::ios::binary);
			if (!raw->good())
			{
				delete raw;
				return 0;
			}
			auto* entry = new FileEntry {};
			entry->owned_stream = raw;
			entry->stream = raw;
			FileHandle const handle = next_handle_++;
			files_[handle] = entry;
			return handle;
		}

		auto* entry = new FileEntry {};
		entry->res = file;
		entry->stream = &file->input_stream();
		FileHandle const handle = next_handle_++;
		files_[handle] = entry;
		return handle;
	}

	void Close(Rml::FileHandle file) override
	{
		auto it = files_.find(file);
		if (it == files_.end())
		{
			return;
		}
		delete it->second->owned_stream;
		delete it->second;
		files_.erase(it);
	}

	size_t Read(void* buffer, size_t size, Rml::FileHandle file) override
	{
		auto it = files_.find(file);
		if (it == files_.end() || !it->second->stream)
		{
			return 0;
		}
		it->second->stream->read(static_cast<char*>(buffer), static_cast<std::streamsize>(size));
		return static_cast<size_t>(it->second->stream->gcount());
	}

	bool Seek(Rml::FileHandle file, long offset, int origin) override
	{
		auto it = files_.find(file);
		if (it == files_.end() || !it->second->stream)
		{
			return false;
		}
		std::ios_base::seekdir dir = std::ios_base::beg;
		if (origin == SEEK_CUR)
		{
			dir = std::ios_base::cur;
		}
		else if (origin == SEEK_END)
		{
			dir = std::ios_base::end;
		}
		it->second->stream->clear();
		it->second->stream->seekg(offset, dir);
		return !it->second->stream->fail();
	}

	size_t Tell(Rml::FileHandle file) override
	{
		auto it = files_.find(file);
		if (it == files_.end() || !it->second->stream)
		{
			return 0;
		}
		return static_cast<size_t>(it->second->stream->tellg());
	}

private:
	using FileHandle = Rml::FileHandle;

	struct FileEntry
	{
		ResIdentifierPtr res;
		std::istream* stream {nullptr};
		std::ifstream* owned_stream {nullptr};
	};

	FileHandle next_handle_ {1};
	std::unordered_map<FileHandle, FileEntry*> files_;
};

UIManager::UIManager() = default;

UIManager::~UIManager()
{
	Destroy();
}

void UIManager::Init()
{
	if (rml_context_)
	{
		return;
	}

	destroyed_ = false;

		if (!Context::Instance().RenderFactoryValid())
		{
			LogError() << "UIManager::Init requires RenderFactory." << std::endl;
			return;
		}

		system_interface_ = CommonWorker::MakeUniquePtr<EditorRmlSystemInterface>();
		file_interface_ = CommonWorker::MakeUniquePtr<RmlUiFileInterface>();
		render_interface_ = CommonWorker::MakeUniquePtr<RmlUiRenderInterfaceD3D11>();
		if (!render_interface_->Ready())
		{
			LogError() << "UIManager::Init: RmlUi render pipeline failed." << std::endl;
			render_interface_.reset();
			file_interface_.reset();
			system_interface_.reset();
			return;
		}

	::Rml::SetSystemInterface(system_interface_.get());
	::Rml::SetFileInterface(file_interface_.get());
	::Rml::SetRenderInterface(render_interface_.get());

	if (!::Rml::Initialise())
	{
		LogError() << "UIManager::Init: Rml::Initialise failed." << std::endl;
		render_interface_.reset();
		file_interface_.reset();
		system_interface_.reset();
		return;
	}

	render_interface_->SetViewportSize(width_, height_);
	rml_context_ = ::Rml::CreateContext("game_ui", ::Rml::Vector2i(width_, height_));
	if (!rml_context_)
	{
		::Rml::Shutdown();
		render_interface_.reset();
		file_interface_.reset();
		system_interface_.reset();
		return;
	}

#ifdef _WIN32
	::Rml::LoadFontFace("C:\\Windows\\Fonts\\segoeui.ttf", true);
#endif

	debugger_initialized_ = ::Rml::Debugger::Initialise(rml_context_);
	if (debugger_initialized_)
	{
		::Rml::Debugger::SetContext(rml_context_);
		::Rml::Debugger::SetVisible(false);
	}

	game_image_hovered_last_ = false;
}

void UIManager::Destroy() noexcept
{
	if (destroyed_)
	{
		return;
	}
	destroyed_ = true;

	if (rml_context_)
	{
		if (debugger_initialized_)
		{
			::Rml::Debugger::Shutdown();
			debugger_initialized_ = false;
		}
		rml_context_->UnloadAllDocuments();
		rml_context_->Update();
		::Rml::RemoveContext("game_ui");
		rml_context_ = nullptr;
	}

	// Only call Rml::Shutdown if we successfully Initialise()'d (interfaces installed).
	if (system_interface_ || file_interface_ || render_interface_)
	{
		::Rml::Shutdown();
	}

	// Drop interfaces after Shutdown so Rml no longer holds dangling pointers.
	render_interface_.reset();
	file_interface_.reset();
	system_interface_.reset();
}

bool UIManager::Valid() const noexcept
{
	return static_cast<bool>(rml_context_);
}

void UIManager::SetDimensions(int width, int height)
{
	width_ = (std::max)(1, width);
	height_ = (std::max)(1, height);
	if (rml_context_)
	{
		rml_context_->SetDimensions(::Rml::Vector2i(width_, height_));
	}
	if (render_interface_)
	{
		render_interface_->SetViewportSize(width_, height_);
	}
}

void UIManager::RenderIntoGameView()
{
	if (!rml_context_)
	{
		return;
	}

	rml_context_->Update();
	rml_context_->Render();
}

Rml::ElementDocument* UIManager::LoadDocument(std::string_view path)
{
	if (!rml_context_)
	{
		Init();
	}
	if (!rml_context_)
	{
		return nullptr;
	}

	return rml_context_->LoadDocument(Rml::String(path.data(), path.size()));
}

Rml::ElementDocument* UIManager::GetDocument(std::string_view id) noexcept
{
	if (!rml_context_)
	{
		return nullptr;
	}
	return rml_context_->GetDocument(Rml::String(id.data(), id.size()));
}

void UIManager::ShowDocument(Rml::ElementDocument* document)
{
	if (document)
	{
		document->Show();
	}
}

void UIManager::HideDocument(Rml::ElementDocument* document)
{
	if (document)
	{
		document->Hide();
	}
}

void UIManager::CloseDocument(Rml::ElementDocument* document)
{
	if (document)
	{
		document->Close();
	}
}

void UIManager::PullDocumentToFront(Rml::ElementDocument* document)
{
	if (document)
	{
		document->PullToFront();
	}
}

bool UIManager::FocusElement(Rml::ElementDocument* document, std::string_view element_id)
{
	if (!document)
	{
		return false;
	}
	Rml::Element* element = document->GetElementById(Rml::String(element_id.data(), element_id.size()));
	if (!element)
	{
		return false;
	}
	element->Focus();
	return true;
}

std::string UIManager::GetInputValue(Rml::ElementDocument* document, std::string_view element_id) const
{
	if (!document)
	{
		return {};
	}
	auto* input = dynamic_cast<Rml::ElementFormControlInput*>(
		document->GetElementById(Rml::String(element_id.data(), element_id.size())));
	if (!input)
	{
		return {};
	}
	return std::string(input->GetValue());
}

void UIManager::SetInputValue(Rml::ElementDocument* document, std::string_view element_id, std::string_view value)
{
	if (!document)
	{
		return;
	}
	auto* input = dynamic_cast<Rml::ElementFormControlInput*>(
		document->GetElementById(Rml::String(element_id.data(), element_id.size())));
	if (input)
	{
		input->SetValue(Rml::String(value.data(), value.size()));
	}
}

std::string UIManager::GetInnerRml(Rml::ElementDocument* document, std::string_view element_id) const
{
	if (!document)
	{
		return {};
	}
	Rml::Element* element = document->GetElementById(Rml::String(element_id.data(), element_id.size()));
	if (!element)
	{
		return {};
	}
	return element->GetInnerRML();
}

void UIManager::SetInnerRml(Rml::ElementDocument* document, std::string_view element_id, std::string_view rml)
{
	if (!document)
	{
		return;
	}
	Rml::Element* element = document->GetElementById(Rml::String(element_id.data(), element_id.size()));
	if (element)
	{
		element->SetInnerRML(Rml::String(rml.data(), rml.size()));
	}
}

void UIManager::ProcessKeyDown(int rml_key_identifier, int key_modifier_state)
{
	if (rml_context_)
	{
		rml_context_->ProcessKeyDown(static_cast<Rml::Input::KeyIdentifier>(rml_key_identifier), key_modifier_state);
	}
}

void UIManager::ProcessKeyUp(int rml_key_identifier, int key_modifier_state)
{
	if (rml_context_)
	{
		rml_context_->ProcessKeyUp(static_cast<Rml::Input::KeyIdentifier>(rml_key_identifier), key_modifier_state);
	}
}

void UIManager::ProcessTextInput(char32_t character)
{
	if (rml_context_)
	{
		rml_context_->ProcessTextInput(character);
	}
}

#if defined(_WIN32)
namespace
{
int RmlKeyModifiersFromWin32()
{
	int mods = 0;
	if (::GetKeyState(VK_CONTROL) & 0x8000)
	{
		mods |= Rml::Input::KM_CTRL;
	}
	if (::GetKeyState(VK_SHIFT) & 0x8000)
	{
		mods |= Rml::Input::KM_SHIFT;
	}
	if (::GetKeyState(VK_MENU) & 0x8000)
	{
		mods |= Rml::Input::KM_ALT;
	}
	return mods;
}

int VkToRmlKeyIdentifier(std::uintptr_t vk)
{
	if (vk >= 'A' && vk <= 'Z')
	{
		return Rml::Input::KI_A + static_cast<int>(vk - 'A');
	}
	if (vk >= '0' && vk <= '9')
	{
		return Rml::Input::KI_0 + static_cast<int>(vk - '0');
	}

	switch (vk)
	{
	case VK_BACK:
		return Rml::Input::KI_BACK;
	case VK_TAB:
		return Rml::Input::KI_TAB;
	case VK_RETURN:
		return Rml::Input::KI_RETURN;
	case VK_DELETE:
		return Rml::Input::KI_DELETE;
	case VK_INSERT:
		return Rml::Input::KI_INSERT;
	case VK_LEFT:
		return Rml::Input::KI_LEFT;
	case VK_RIGHT:
		return Rml::Input::KI_RIGHT;
	case VK_UP:
		return Rml::Input::KI_UP;
	case VK_DOWN:
		return Rml::Input::KI_DOWN;
	case VK_HOME:
		return Rml::Input::KI_HOME;
	case VK_END:
		return Rml::Input::KI_END;
	case VK_PRIOR:
		return Rml::Input::KI_PRIOR;
	case VK_NEXT:
		return Rml::Input::KI_NEXT;
	default:
		return -1;
	}
}
} // namespace

void UIManager::ProcessGmWin32Message(unsigned msg, std::uintptr_t w_param, std::intptr_t /*l_param*/)
{
	if (!rml_context_)
	{
		return;
	}

	int const mods = RmlKeyModifiersFromWin32();
	switch (msg)
	{
	case WM_CHAR:
		if ((::GetKeyState(VK_CONTROL) & 0x8000) == 0 && w_param >= 32 && w_param != '`' && w_param != '~')
		{
			rml_context_->ProcessTextInput(static_cast<Rml::Character>(w_param));
		}
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		int const key = VkToRmlKeyIdentifier(w_param);
		if (key >= 0)
		{
			rml_context_->ProcessKeyDown(static_cast<Rml::Input::KeyIdentifier>(key), mods);
		}
		break;
	}

	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		int const key = VkToRmlKeyIdentifier(w_param);
		if (key >= 0)
		{
			rml_context_->ProcessKeyUp(static_cast<Rml::Input::KeyIdentifier>(key), mods);
		}
		break;
	}

	default:
		break;
	}
}
#endif // _WIN32

void UIManager::SetDebuggerVisible(bool visible)
{
	if (debugger_initialized_)
	{
		::Rml::Debugger::SetVisible(visible);
	}
}

bool UIManager::IsDebuggerVisible() const
{
	return debugger_initialized_ && ::Rml::Debugger::IsVisible();
}

void UIManager::ProcessGameViewPointer(bool image_hovered, int mouse_x, int mouse_y, int key_modifier_state,
	bool left_pressed, bool left_released, bool right_pressed, bool right_released, bool middle_pressed,
	bool middle_released, float wheel_x, float wheel_y)
{
	if (!rml_context_)
	{
		return;
	}

	if (!image_hovered)
	{
		if (game_image_hovered_last_)
		{
			rml_context_->ProcessMouseLeave();
		}
		game_image_hovered_last_ = false;
		mouse_on_ui_ = false;
		return;
	}

	game_image_hovered_last_ = true;

	int const mx = (std::clamp)(mouse_x, 0, width_ - 1);
	int const my = (std::clamp)(mouse_y, 0, height_ - 1);
	rml_context_->ProcessMouseMove(mx, my, key_modifier_state);

	if (left_pressed)
	{
		rml_context_->ProcessMouseButtonDown(0, key_modifier_state);
	}
	if (right_pressed)
	{
		rml_context_->ProcessMouseButtonDown(1, key_modifier_state);
	}
	if (middle_pressed)
	{
		rml_context_->ProcessMouseButtonDown(2, key_modifier_state);
	}

	if (left_released)
	{
		rml_context_->ProcessMouseButtonUp(0, key_modifier_state);
	}
	if (right_released)
	{
		rml_context_->ProcessMouseButtonUp(1, key_modifier_state);
	}
	if (middle_released)
	{
		rml_context_->ProcessMouseButtonUp(2, key_modifier_state);
	}

	if (wheel_x != 0.f || wheel_y != 0.f)
	{
		rml_context_->ProcessMouseWheel(::Rml::Vector2f(wheel_x, wheel_y), key_modifier_state);
	}

	if (Rml::Element* hover = rml_context_->GetHoverElement())
	{
		mouse_on_ui_ = hover != rml_context_->GetRootElement();
	}
	else
	{
		mouse_on_ui_ = false;
	}
}

bool UIManager::MouseOnUI() const noexcept
{
	return mouse_on_ui_;
}

} // namespace RenderWorker
