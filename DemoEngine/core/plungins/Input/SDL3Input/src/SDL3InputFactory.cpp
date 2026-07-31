/**
 * @file SDL3InputFactory.cpp
 *
 * @section DESCRIPTION
 *
 * Entry point of the SDL3 input plugin.
 */

#include <base/ZEngine.h>
#include <base/InputFactory.h>

#include "SDL3Input.hpp"

namespace RenderWorker
{
	class SDL3InputFactory : public InputFactory
	{
	public:
		std::wstring const& Name() const override
		{
			static std::wstring const name(L"SDL3 Input Factory");
			return name;
		}

	private:
		std::unique_ptr<InputEngine> DoMakeInputEngine() override
		{
			return MakeUniquePtr<SDL3InputEngine>();
		}

		void DoSuspend() override
		{
		}
		void DoResume() override
		{
		}
	};
}

extern "C"
{
	ZENGINE_SYMBOL_EXPORT void MakeInputFactory(std::unique_ptr<RenderWorker::InputFactory>& ptr)
	{
		ptr = RenderWorker::MakeUniquePtr<RenderWorker::SDL3InputFactory>();
	}
}
