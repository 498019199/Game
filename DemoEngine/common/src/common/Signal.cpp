#include <mutex>

#include <common/Signal.h>

namespace CommonWorker
{
	namespace Signal
	{
		namespace Detail
		{
			SignalBase::SignalBase() noexcept = default;
			SignalBase::~SignalBase() noexcept = default;

			class Mutex::Impl
			{
			public:
				Impl() noexcept = default;

				void Lock()
				{
					mutex_.lock();
				}

				void Unlock()
				{
					mutex_.unlock();
				}

			private:
				std::mutex mutex_;
			};

			Mutex::Mutex() noexcept
				: pimpl_(MakeUniquePtr<Mutex::Impl>())
			{
			}

			Mutex::~Mutex() noexcept = default;

			void Mutex::Lock()
			{
				pimpl_->Lock();
			}

			void Mutex::Unlock()
			{
				pimpl_->Unlock();
			}
		}

		Connection::Connection() noexcept = default;

		Connection::Connection(Connection&& rhs) noexcept : signal_(std::move(rhs.signal_)), slot_(std::move(rhs.slot_))
		{
		}

		Connection::Connection(Detail::SignalBase& signal, std::shared_ptr<void> const& slot) : signal_(&signal), slot_(slot)
		{
		}

		Connection& Connection::operator=(Connection&& rhs) noexcept
		{
			if (this != &rhs)
			{
				signal_ = std::move(rhs.signal_);
				slot_ = std::move(rhs.slot_);
			}
			return *this;
		}

		void Connection::Disconnect()
		{
			auto slot = slot_.lock();
			if (slot)
			{
				signal_->Disconnect(slot.get());
			}
		}

		bool Connection::Connected() const
		{
			return !slot_.expired();
		}

		void Connection::Swap(Connection& rhs)
		{
			std::swap(signal_, rhs.signal_);
			std::swap(slot_, rhs.slot_);
		}

		Detail::SignalBase& Connection::Signal() const noexcept
		{
			return *signal_;
		}

		void* Connection::Slot() const noexcept
		{
			return slot_.lock().get();
		}

		bool Connection::operator==(Connection const& rhs) const
		{
			return (signal_ == rhs.signal_) && (this->Slot() == rhs.Slot());
		}

		bool Connection::operator<(Connection const& rhs) const
		{
			return (signal_ < rhs.signal_) || (!(rhs.signal_ < signal_) && (this->Slot() < rhs.Slot()));
		}
	} // namespace Signal
} // namespace CommonWorker
