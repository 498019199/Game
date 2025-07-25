/**
 * @file CpuInfo.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KFL, a subproject of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <common/common.h>
#include <common/Util.h>

#if defined(ZENGINE_PLATFORM_WINDOWS)
#include <windows.h>
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
#include <VersionHelpers.h>
#endif
#endif
#ifdef ZENGINE_COMPILER_MSVC
#include <intrin.h>
#endif
#elif defined(ZENGINE_PLATFORM_LINUX)
#include <sched.h>
#include <unistd.h>
#endif
#include <cstring>
#include <vector>


#include <base/CpuInfo.h>

namespace
{
	using namespace CommonWorker;

#if (defined(ZENGINE_CPU_X86) || defined(ZENGINE_CPU_X64)) && !defined(ZENGINE_PLATFORM_ANDROID)
	void get_cpuid(uint32_t* peax, uint32_t* pebx, uint32_t* pecx, uint32_t* pedx)
	{	
#if defined(ZENGINE_COMPILER_MSVC)
		std::array<int, 4> id;
		__cpuidex(&id[0], *peax, *pecx);
		*peax = id[0];
		*pebx = id[1];
		*pecx = id[2];
		*pedx = id[3];
#elif defined(ZENGINE_COMPILER_GCC) || defined(ZENGINE_COMPILER_CLANG) || defined(ZENGINE_COMPILER_CLANGCL)
	#ifdef ZENGINE_CPU_X64
		__asm__
		(
			"cpuid"
			: "=a" (*peax), "=b" (*pebx), "=c" (*pecx), "=d" (*pedx)
			: "a" (*peax), "c" (*pecx)
		);
	#elif defined(ZENGINE_PLATFORM_IOS)
		// TODO: iOS Device compile complains: error: invalid output constraint '=a' in asm
		KFL_UNUSED(peax);
		KFL_UNUSED(pebx);
		KFL_UNUSED(pecx);
		KFL_UNUSED(pedx);
	#else
		__asm__
		(
			"pushl  %%ebx			\n\t"
			"cpuid					\n\t"
			"movl   %%ebx, %%edi	\n\t"
			"popl   %%ebx			\n\t"
			: "=a" (*peax), "=D" (*pebx), "=c" (*pecx), "=d" (*pedx)
			: "a" (*peax), "c" (*pecx)
		);
	#endif
#else
		// TODO: Supports other compiler
#endif
	}

	enum CPUIDFeatureMask : uint32_t
	{
		// In EBX of type 1. Intel only.
		CFM_LogicalProcessorCount_Intel = 0x00FF0000,
		CFM_ApicId_Intel = 0xFF000000,

		// In ECX of type 1
		CFM_SSE3		= 1UL << 0,		// SSE3
		CFM_SSSE3		= 1UL << 9,		// SSSE3
		CFM_FMA3		= 1UL << 12,	// 256-bit FMA (Intel Haswell, AMD Piledriver)
		CFM_SSE41		= 1UL << 19,	// SSE4.1 (Intel Core 2 Penryn, Intel Core i7 Nehalem, AMD Bulldozer)
		CFM_SSE42		= 1UL << 20,	// SSE4.2 (Intel Core i7 Nehalem, AMD Bulldozer)
		CFM_MOVBE		= 1UL << 22,	// MOVBE (Intel)
		CFM_POPCNT		= 1UL << 23,	// POPCNT
		CFM_AES			= 1UL << 25,	// AES support (Intel)
		CFM_OSXSAVE		= 1UL << 27,	// OSX save
		CFM_AVX			= 1UL << 28,	// 256-bit AVX (Intel Sandy Bridge, AMD Bulldozer)
		CFM_F16C		= 1UL << 29,	// F16C (Intel Ivy Bridge, AMD Piledriver)

		// In EDX of type 1
		CFM_MMX			= 1UL << 23,	// MMX Technology
		CFM_SSE			= 1UL << 25,	// SSE
		CFM_SSE2		= 1UL << 26,	// SSE2
		CFM_HTT			= 1UL << 28,	// Hyper-threading technology

		// In EBX of type 7
		CFM_AVX2		= 1UL << 5,

		// In EAX of type 4. Intel only.
		CFM_NC_Intel				= 0xFC000000,

		// In ECX of type 0x80000001. AMD only.
		CFM_CmpLegacy_AMD			= 0x00000002,
		CFM_LZCNT_AMD				= 1UL << 5,
		CFM_SSE4A_AMD				= 1UL << 6,
		CFM_MisalignedSSE_AMD		= 1UL << 7,
		CFM_FMA4					= 1UL << 16,	// FMA4 (AMD Bulldozer)

		// In EDX of type 0x80000001
		CFM_X64						= 1UL << 29,

		// In ECX of type 0x80000008. AMD only.
		CFM_NC_AMD					= 0x000000FF,
		CFM_ApicIdCoreIdSize_AMD	= 0x0000F000,
	};

#if defined ZENGINE_PLATFORM_LINUX
	class ApicExtractor
	{
	public:
		ApicExtractor(uint8_t log_procs_per_pkg, uint8_t cores_per_pkg) noexcept
			: log_procs_per_pkg_(log_procs_per_pkg), cores_per_pkg_(cores_per_pkg)
		{
			smt_id_mask_.width = this->GetMaskWidth(log_procs_per_pkg_ / cores_per_pkg_);
			core_id_mask_.width = this->GetMaskWidth(cores_per_pkg_);
			pkg_id_mask_.width = 8 - (smt_id_mask_.width + core_id_mask_.width);

			pkg_id_mask_.mask = static_cast<uint8_t>(0xFF << (smt_id_mask_.width + core_id_mask_.width));
			core_id_mask_.mask = static_cast<uint8_t>((0xFF << smt_id_mask_.width) ^ pkg_id_mask_.mask);
			smt_id_mask_.mask = static_cast<uint8_t>(~(0xFF << smt_id_mask_.width));
		}

		uint8_t SmtId(uint8_t apic_id) const noexcept
		{
			return apic_id & smt_id_mask_.mask;
		}

		uint8_t CoreId(uint8_t apic_id) const noexcept
		{
			return (apic_id & core_id_mask_.mask) >> smt_id_mask_.width;
		}

		uint8_t PackageId(uint8_t apic_id) const noexcept
		{
			return (apic_id & pkg_id_mask_.mask) >> (smt_id_mask_.width + core_id_mask_.width);
		}

		uint8_t PackageCoreId(uint8_t apic_id) const noexcept
		{
			return (apic_id & (pkg_id_mask_.mask | core_id_mask_.mask)) >> smt_id_mask_.width;
		}

		uint8_t LogProcsPerPkg() const noexcept
		{
			return log_procs_per_pkg_;
		}

		uint8_t CoresPerPkg() const noexcept
		{
			return cores_per_pkg_;
		}

	private:
		static uint8_t GetMaskWidth(uint8_t max_ids) noexcept
		{
			-- max_ids;

			// find index of msb
			uint8_t msb_idx = 8;
			uint8_t msb_mask = 0x80;
			while (msb_mask && !(msb_mask & max_ids))
			{
				-- msb_idx;
				msb_mask >>= 1;
			}
			return msb_idx;
		}

	private:
		struct id_mask_t
		{
			uint8_t width;
			uint8_t mask;
		};

		uint8_t		log_procs_per_pkg_;
		uint8_t		cores_per_pkg_;
		id_mask_t	smt_id_mask_;
		id_mask_t	core_id_mask_;
		id_mask_t	pkg_id_mask_;
	};
#endif

	class Cpuid
	{
	public:
		Cpuid()
			: eax_(0), ebx_(0), ecx_(0), edx_(0)
		{
		}

		uint32_t Eax() const
		{
			return eax_;
		}
		uint32_t Ebx() const
		{
			return ebx_;
		}
		uint32_t Ecx() const
		{
			return ecx_;
		}
		uint32_t Edx() const
		{
			return edx_;
		}

		void Call(uint32_t fn)
		{
			eax_ = fn;
			get_cpuid(&eax_, &ebx_, &ecx_, &edx_);
		}

	private:
		uint32_t eax_;
		uint32_t ebx_;
		uint32_t ecx_;
		uint32_t edx_;
	};

	char const GenuineIntel[] = "GenuineIntel";
	char const AuthenticAMD[] = "AuthenticAMD";
#endif
}


namespace RenderWorker
{
	CpuInfo::CpuInfo()
	{
		num_hw_threads_ = 1;
		num_cores_ = 1;

#if (defined(ZENGINE_CPU_X86) || defined(ZENGINE_CPU_X64)) && !defined(ZENGINE_PLATFORM_ANDROID)
		Cpuid cpuid;

		cpuid.Call(0);
		uint32_t const max_std_fn = cpuid.Eax();

		cpu_string_.resize(12);
		uint32_t temp = cpuid.Ebx();
		memcpy(&cpu_string_[0], &temp, sizeof(temp));
		temp = cpuid.Edx();
		memcpy(&cpu_string_[4], &temp, sizeof(temp));
		temp = cpuid.Ecx();
		memcpy(&cpu_string_[8], &temp, sizeof(temp));

		bool const is_intel = (&GenuineIntel[0] == cpu_string_);
		bool const is_amd = (&AuthenticAMD[0] == cpu_string_);

		KFL_UNUSED(is_intel);

		if (max_std_fn >= 1)
		{
			cpuid.Call(1);

			feature_mask_ |= (cpuid.Edx() & CFM_MMX) ? CF_MMX : 0;
			feature_mask_ |= (cpuid.Edx() & CFM_SSE) ? CF_SSE : 0;
			feature_mask_ |= (cpuid.Edx() & CFM_SSE2) ? CF_SSE2 : 0;
			feature_mask_ |= (cpuid.Ecx() & CFM_SSE3) ? CF_SSE3 : 0;
			feature_mask_ |= (cpuid.Edx() & CFM_HTT) ? CF_HTT : 0;
			feature_mask_ |= (cpuid.Ecx() & CFM_SSSE3) ? CF_SSSE3 : 0;
			feature_mask_ |= (cpuid.Ecx() & CFM_SSE41) ? CF_SSE41 : 0;
			feature_mask_ |= (cpuid.Ecx() & CFM_SSE42) ? CF_SSE42 : 0;
			feature_mask_ |= (cpuid.Ecx() & CFM_MOVBE) ? CF_MOVBE : 0;
			feature_mask_ |= (cpuid.Ecx() & CFM_POPCNT) ? CF_POPCNT : 0;
			feature_mask_ |= (cpuid.Ecx() & CFM_AES) ? CF_AES : 0;
			if (cpuid.Ecx() & CFM_OSXSAVE)
			{
				feature_mask_ |= (cpuid.Ecx() & CFM_FMA3) ? CF_FMA3 : 0;
				feature_mask_ |= (cpuid.Ecx() & CFM_AVX) ? CF_AVX : 0;
				feature_mask_ |= (cpuid.Ecx() & CFM_F16C) ? CF_F16C : 0;
			}

			if (max_std_fn >= 7)
			{
				cpuid.Call(7);

				feature_mask_ |= cpuid.Ebx() & CFM_AVX2 ? CF_AVX2 : 0;
			}
		}

		cpuid.Call(0x80000000);
		uint32_t const max_ext_fn = cpuid.Eax();
		if (max_ext_fn & 0x80000000)
		{
			if (max_ext_fn >= 0x80000001)
			{
				cpuid.Call(0x80000001);
				if (is_amd)
				{
					feature_mask_ |= cpuid.Ecx() & CFM_LZCNT_AMD ? CF_LZCNT : 0;
					feature_mask_ |= cpuid.Ecx() & CFM_SSE4A_AMD ? CF_SSE4A : 0;
					feature_mask_ |= cpuid.Ecx() & CFM_MisalignedSSE_AMD ? CF_MisalignedSSE : 0;
				}
				feature_mask_ |= cpuid.Edx() & CFM_X64 ? CF_X64 : 0;
				if (cpuid.Ecx() & CFM_OSXSAVE)
				{
					feature_mask_ |= cpuid.Ecx() & CFM_FMA4 ? CF_FMA4 : 0;
				}
			}

			if (max_ext_fn >= 0x80000004)
			{
				cpu_brand_string_.resize(48);

				cpuid.Call(0x80000002);
				temp = cpuid.Eax();
				memcpy(&cpu_brand_string_[0], &temp, sizeof(temp));
				temp = cpuid.Ebx();
				memcpy(&cpu_brand_string_[4], &temp, sizeof(temp));
				temp = cpuid.Ecx();
				memcpy(&cpu_brand_string_[8], &temp, sizeof(temp));
				temp = cpuid.Edx();
				memcpy(&cpu_brand_string_[12], &temp, sizeof(temp));

				cpuid.Call(0x80000003);
				temp = cpuid.Eax();
				memcpy(&cpu_brand_string_[16], &temp, sizeof(temp));
				temp = cpuid.Ebx();
				memcpy(&cpu_brand_string_[20], &temp, sizeof(temp));
				temp = cpuid.Ecx();
				memcpy(&cpu_brand_string_[24], &temp, sizeof(temp));
				temp = cpuid.Edx();
				memcpy(&cpu_brand_string_[28], &temp, sizeof(temp));

				cpuid.Call(0x80000004);
				temp = cpuid.Eax();
				memcpy(&cpu_brand_string_[32], &temp, sizeof(temp));
				temp = cpuid.Ebx();
				memcpy(&cpu_brand_string_[36], &temp, sizeof(temp));
				temp = cpuid.Ecx();
				memcpy(&cpu_brand_string_[40], &temp, sizeof(temp));
				temp = cpuid.Edx();
				memcpy(&cpu_brand_string_[44], &temp, sizeof(temp));
			}
		}
#if defined ZENGINE_PLATFORM_WINDOWS
		{
			SYSTEM_INFO si;
			::GetSystemInfo(&si);
			num_hw_threads_ = si.dwNumberOfProcessors;
		}
#elif defined ZENGINE_PLATFORM_LINUX
		// Linux doesn't easily allow us to look at the Affinity Bitmask directly,
		// but it does provide an API to test affinity maskbits of the current process
		// against each logical processor visible under OS.
		num_hw_threads_ = sysconf(_SC_NPROCESSORS_CONF);	// This will tell us how many CPUs are currently enabled.
#endif

#if defined ZENGINE_PLATFORM_WINDOWS
		std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> slpis_;

		DWORD buffer_size = 0;
		::GetLogicalProcessorInformation(nullptr, &buffer_size);

		slpis_.resize(buffer_size / sizeof(slpis_[0]));
		::GetLogicalProcessorInformation(slpis_.data(), &buffer_size);

		num_cores_ = 0;
		for (auto& slpi : slpis_)
		{
			if (::RelationProcessorCore == slpi.Relationship)
			{
				++ num_cores_;
			}
		}
#elif defined ZENGINE_PLATFORM_LINUX
		if (is_intel || is_amd)
		{
			uint8_t log_procs_per_pkg = 1;
			uint8_t cores_per_pkg = 1;

			// Determine if hyper-threading is enabled.
			if (this->IsFeatureSupport(CF_HTT))
			{
				cpuid.Call(1);

				// Determine the total number of logical processors per package.
				log_procs_per_pkg = static_cast<uint8_t>((cpuid.Ebx() & CFM_LogicalProcessorCount_Intel) >> 16);

				// Determine the total number of cores per package.  This info
				// is extracted differently dependending on the cpu vendor.
				if (is_intel)
				{
					if (max_std_fn >= 4)
					{
						cpuid.Call(4);
						cores_per_pkg = static_cast<uint8_t>(((cpuid.Eax() & CFM_NC_Intel) >> 26) + 1);
					}
				}
				else
				{
					BOOST_ASSERT(is_amd);

					if (max_ext_fn >= 0x80000008)
					{
						cpuid.Call(0x80000008);

						// AMD reports the msb width of the CORE_ID bit field of the APIC ID
						// in ApicIdCoreIdSize_Amd.  The maximum value represented by the msb
						// width is the theoretical number of cores the processor can support
						// and not the actual number of current cores, which is how the msb width
						// of the CORE_ID bit field has been traditionally determined.  If the
						// ApicIdCoreIdSize_Amd value is zero, then you use the traditional method
						// to determine the CORE_ID msb width.
						uint32_t msb_width = cpuid.Ecx() & CFM_ApicIdCoreIdSize_AMD;
						if (msb_width)
						{
							// Set cores_per_pkg to the maximum theortical number of cores
							// the processor package can support (2 ^ width) so the APIC
							// extractor object can be configured to extract the proper
							// values from an APIC.
							cores_per_pkg = static_cast<uint8_t>(1 << ((msb_width >> 12) - 1));
						}
						else
						{
							// Set cores_per_pkg to the actual number of cores being reported
							// by the CPUID instruction.
							cores_per_pkg = static_cast<uint8_t>((cpuid.Ecx() & CFM_NC_AMD) + 1);
						}
					}
				}
			}

			std::vector<uint8_t> apic_ids;

			// Configure the APIC extractor object with the information it needs to
			// be able to decode the APIC.
			ApicExtractor apic_extractor(log_procs_per_pkg, cores_per_pkg);

			if (1 == num_hw_threads_)
			{
				// Since we only have 1 logical processor present on the system, we
				// can explicitly set a single APIC ID to zero.
				BOOST_ASSERT(1 == log_procs_per_pkg);
				apic_ids.push_back(0);
			}
			else
			{
				cpu_set_t backup_cpu;
				sched_getaffinity(0, sizeof(backup_cpu), &backup_cpu);

				cpu_set_t current_cpu;
				// Call cpuid on each active logical processor in the system affinity.
				for (uint32_t j = 0; j < num_hw_threads_; ++j)
				{
					CPU_ZERO(&current_cpu);
					CPU_SET(static_cast<int>(j), &current_cpu);
					if (0 == sched_setaffinity(0, sizeof(current_cpu), &current_cpu))
					{
						// Allow the thread to switch to masked logical processor.
						sleep(0);

						// Store the APIC ID
						cpuid.Call(1);
						apic_ids.push_back(static_cast<uint8_t>((cpuid.Ebx() & CFM_ApicId_Intel) >> 24));
					}
				}

				// Restore the previous process and thread affinity state.
				sched_setaffinity(0, sizeof(backup_cpu), &backup_cpu);
				sleep(0);
			}
		}
#elif defined(ZENGINE_PLATFORM_DARWIN) || defined(ZENGINE_PLATFORM_IOS)
		// TODO
		num_cores_ = num_hw_threads_;
#endif
#endif
	}
}
