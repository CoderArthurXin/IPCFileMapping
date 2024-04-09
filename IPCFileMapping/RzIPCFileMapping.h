#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>

namespace RzFileMapping {

	const wchar_t* const WRITE_EVENT_NAME = L"RzFileMapping-FB434D3D-0971-4ED6-B558-A8ED004F86BC";
	const wchar_t* const READ_EVENT_NAME = L"RzFileMapping-CAB9A17C-6E1F-498B-B107-70B4D6809AC9";
	const wchar_t* const EXIT_EVENT_NAME = L"RzFileMapping-A35F218B-9AEE-4116-A395-6C87A82462A9";
	const wchar_t* const FILE_MAPPING_NAME = L"RzFileMapping-4F6A8787-0758-40BF-898F-03E30237C71F";

	class RzIPCFileMapping final
	{

		bool m_bIsWriter{ false };
		bool m_IsReading{ false };
		HANDLE m_hWrite{ nullptr };
		HANDLE m_hRead{ nullptr };
		HANDLE m_hExit{ nullptr };
		std::atomic<bool> m_bExitRead;
		std::thread m_threadRead;

		bool Read(std::string& read);
		bool IsReady() const;
	public:

		explicit RzIPCFileMapping(bool isWriter);
		~RzIPCFileMapping();

		bool Write(const char* pData);

		bool OnRead(const std::function<void(std::string)>& callback);
	};
}



