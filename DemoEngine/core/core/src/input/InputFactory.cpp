// InputFactory.cpp
// KlayGE 输入引擎抽象工厂 实现文件
// Ver 3.1.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://www.klayge.org
//
// 3.1.0
// 初次建立 (2005.10.29)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <base/Input.h>
#include <base/InputFactory.h>


namespace RenderWorker
{
	InputFactory::InputFactory() noexcept = default;
	InputFactory::~InputFactory() noexcept = default;

	InputEngine& InputFactory::InputEngineInstance()
	{
		if (!ie_)
		{
			ie_ = this->DoMakeInputEngine();
		}

		return *ie_;
	}

	void InputFactory::Suspend()
	{
		if (ie_)
		{
			ie_->Suspend();
		}
		this->DoSuspend();
	}

	void InputFactory::Resume()
	{
		this->DoResume();
		if (ie_)
		{
			ie_->Resume();
		}
	}

	// Concrete null input factory implementation
	class NullInputFactory : public InputFactory
	{
	public:
		NullInputFactory() noexcept = default;
		virtual ~NullInputFactory() noexcept = default;

		std::wstring const& Name() const override
		{
			static const std::wstring name = L"Null Input Factory";
			return name;
		}

	private:
		std::unique_ptr<InputEngine> DoMakeInputEngine() override
		{
			return nullptr;
		}

		void DoSuspend() override
		{
		}

		void DoResume() override
		{
		}
	};
}
