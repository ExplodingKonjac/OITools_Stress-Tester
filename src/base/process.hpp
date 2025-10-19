#pragma once

#include <memory>
#include <filesystem>
#include <vector>
#include <cstdio>
#include <chrono>

#if defined(_WIN32)

#include <winbase.h>
#include <handleapi.h>
#include <fileapi.h>
#include <psapi.h>
#include <processthreadsapi.h>

namespace detail
{

template<auto close_func>
struct HandleCloser
{
	HandleCloser(bool _c=false): close{_c}
	{}
	void operator()(HANDLE h) const
	{ if(close) close_func(h); }

	bool close{true};
};

} // namespace detail

template<typename T,auto close_func>
using auto_handle=std::unique_ptr<
	std::remove_pointer_t<T>,
	detail::HandleCloser<close_func>
>;

using normal_handle=auto_handle<HANDLE,&::CloseHandle>;
using attr_list_handle=auto_handle<LPPROC_THREAD_ATTRIBUTE_LIST,&::DeleteProcThreadAttributeList>;

extern "C" intptr_t _get_osfhandle(int fd);

struct ProcessInfo
{
	enum class Types{ EXITED,TIMEOUT,OOM };
	Types type{Types::EXITED};
	DWORD exit_code{-1};
	std::uint64_t time_used{-1};
	std::uint64_t memory_used{-1};
};

class Process
{
 private:
	using Path=std::filesystem::path;

	template<DWORD Target>
	struct BindIO
	{
		normal_handle h{::GetStdHandle(Target)};

		BindIO()=default;
		BindIO(HANDLE _h): h{_h,false}
		{}
		BindIO(const Path &path):
			h{::CreateFileW(
				path.c_str(),
				Target==STD_INPUT_HANDLE?GENERIC_READ:GENERIC_WRITE,
				FILE_SHARE_READ|FILE_SHARE_WRITE,
				nullptr,
				OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				nullptr
			),true}
		{}
		BindIO(FILE *file):
			h{::_get_osfhandle(_fileno(file)),false}
		{}
	};

	static void throwError(const char *msg)
	{ throw std::system_error(::GetLastError(),std::system_category(),msg); }

 public:
	using native_exit_code_t=DWORD;
	using native_pid_t=DWORD;

 	struct RedirectIO
	{
		BindIO<STD_INPUT_HANDLE> in;
		BindIO<STD_OUTPUT_HANDLE> out;
		BindIO<STD_ERROR_HANDLE> err;
	};
	struct StartDir
	{ std::filesystem::path path; };

	Process()=default;
	~Process()=default;

	template<typename ...Inits>
	Process(const Path &exe,
			const std::vector<std::string> &args,
			Inits &&...inits)
	{
		LPSECURITY_ATTRIBUTES process_attributes{};
		LPSECURITY_ATTRIBUTES thread_attributes{};
		bool inherit_handles{};
		DWORD creation_flags=EXTENDED_STARTUPINFO_PRESENT;
		void *environment{};
		Path current_directory{};
		STARTUPINFOEXW startup_info{
			{
				.cb=sizeof(STARTUPINFOEXW),
				.hStdInput=INVALID_HANDLE_VALUE,
				.hStdOutput=INVALID_HANDLE_VALUE,
				.hStdError=INVALID_HANDLE_VALUE,
			},
		};
		PROCESS_INFORMATION process_information{};

		// process initializers
		auto process_init=[&]<typename T>(T &init) {
			if constexpr(std::is_same_v<T,RedirectIO>)
			{
				startup_info.StartupInfo.dwFlags|=STARTF_USESTDHANDLES;
				startup_info.StartupInfo.hStdInput=init.in.h.get();
				startup_info.StartupInfo.hStdOutput=init.out.h.get();
				startup_info.StartupInfo.hStdError=init.err.h.get();
			}
			else if constexpr(std::is_same_v<T,StartDir>)
				current_directory=std::move(init.path);
		};
		process_init(init),...;

		// construct command line
		std::wstring cmdline=L"\""+exe.native()+L"\"";
		for(const auto &arg: args)
		{
			auto warg=std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.from_bytes(arg);
			cmdline+=L" \"";
			for(std::wstring::size_type pos=0,nxt;pos<warg.size();pos=nxt+1)
			{
				nxt=warg.find(L"\\\"",pos);
				cmdline+=warg.substr(pos,nxt-pos);
				if(nxt==std::wstring::npos)
					break;
				else if(warg[nxt]==L'\\')
					cmdline+=L"\\\\";
				else
					cmdline+=L"\\\"";
			}
			cmdline+='\"';
		}

		// create process
		if(!::CreateProcessW(
			nullptr,
			cmdline,
			process_attributes,
			thread_attributes,
			inherit_handles,
			creation_flags,
			environment,
			current_directory.c_str(),
			&startup_info.StartupInfo,
			&process_information
		))
			throwError("CreateProcessW failed");

		_process_id=process_information.dwProcessId;
		_process_handle=process_information.hProcess;
	}

	void wait() const
	{
		auto ret=::WaitForSingleObject(_process_handle.get(),INFINITE);
		if(ret==WAIT_FAILED)
			throwError("WaitForSingleObject failed");
	}

	template<typename Rep,typename Period>
	bool waitFor(const std::chrono::duration<Rep,Period> &timeout) const
	{
		auto timeout_ms=std::chrono::duration_cast<std::chrono::duration<DWORD,std::milli>>(timeout);
		auto ret=::WaitForSingleObject(_process_handle.get(),timeout_ms.count());
		if(ret==WAIT_FAILED)
			throwError("WaitForSingleObject failed");
		return ret==WAIT_OBJECT_0;
	}

	native_exit_code_t exitCode() const
	{ return _exit_code; }

	ProcessInfo watch(std::uint64_t time_limit_ms,
					  std::uint64_t memory_limit_b,
					  std::uint64_t timeout_ms) const
	{
		auto getTimeUsage=[&]()->std::size_t {
			FILETIME creation_time,exit_time,kernel_time,user_time;
			if(!::GetProcessTimes(
				_process_handle.get(),
				&creation_time,
				&exit_time,
				&kernel_time,
				&user_time
			))
				throwError("GetProcessTimes failed");
			ULARGE_INTEGER res{
				.LowPart=user_time.dwLowDateTime,
				.HighPart=user_time.dwHighDateTime,
			};
			return res.QuadPart/10'000;
		};
		auto getMemoryUsage=[&]()->std::size_t {
			PROCESS_MEMORY_COUNTERS_EX mem_info{sizeof(mem_info)};
			if(!::GetProcessMemoryInfo(
				_process_handle.get(),
				reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&mem_info),
				sizeof(mem_info)
			))
				throwError("GetProcessMemoryInfo failed");
			return std::max(mem_info.PeakWorkingSetSize,mem_info.PrivateUsage);
		};

		auto lim=std::chrono::steady_clock::now()+std::chrono::milliseconds(timeout_ms);
		while(std::chrono::steady_clock::now()<=lim)
		{
			auto wait_res=::WaitForSingleObject(_process_handle.get(),15);
			if(wait_res==WAIT_FAILED)
				throwError("WaitForSingleObject failed");
			else if(wait_res==WAIT_OBJECT_0)
			{
				if(!::GetExitCodeProcess(_process_handle.get(),&_exit_code))
					throwError("GetExitCodeProcess failed");
				ProcessInfo res{
					.type=ProcessInfo::Types::EXITED,
					.exit_code=_exit_code,
					.time_used=getTimeUsage(),
					.memory_used=getMemoryUsage(),
				};;
			}
			else if(auto mem=getMemoryUsage(); mem>memory_limit_b)
			{
				if(!::TerminateProcess(_process_handle.get(),-1))
					throwError("TerminateProcess failed");
				return ProcessInfo{
					.type=ProcessInfo::Types::OOM,
					.memory_used=mem
				};
			}
		}
		if(!::TerminateProcess(_process_handle.get(),-1))
			throwError("TerminateProcess failed");
		return ProcessInfo{
			.type=ProcessInfo::Types::TIMEOUT,
			.memory_used=getMemoryUsage()
		};
	}
	
 private:
	native_pid_t _process_id{-1};
	normal_handle _process_handle{nullptr};
	mutable native_exit_code_t _exit_code{-1};
};

#elif defined(__unix__)



#endif