#include "pch.h"
#include "RzIPCFileMapping.h"
#include <array>

namespace RzFileMapping {

#define SAFE_CLOSE_NULL_HANDLE(handle) if (nullptr != (handle)) { \
      CloseHandle(handle); \
       (handle) = nullptr; \
}

 
    RzIPCFileMapping::RzIPCFileMapping(bool isWriter) : m_bIsWriter(isWriter)
    {
        TRACE("%s\n", __FUNCTION__);

        m_hWrite = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, WRITE_EVENT_NAME);
        if (nullptr == m_hWrite) {
            m_hWrite = CreateEvent(nullptr, FALSE, FALSE, WRITE_EVENT_NAME);
        }

        m_hRead = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, READ_EVENT_NAME);
        if (nullptr == m_hRead) {
            m_hRead = CreateEvent(nullptr, FALSE, FALSE, READ_EVENT_NAME);
        }
    }

    RzFileMapping::RzIPCFileMapping::~RzIPCFileMapping()
    {
        TRACE("%s\n", __FUNCTION__);

        if (m_threadRead.joinable()) {

            TRACE("%s Begin exit read thread.", __FUNCTION__);

            m_bExitRead = true;
            SetEvent(m_hExit);

            m_threadRead.join();

            TRACE("%s Read thread exited.", __FUNCTION__);
        }

        SAFE_CLOSE_NULL_HANDLE(m_hWrite)
        SAFE_CLOSE_NULL_HANDLE(m_hRead)

        SAFE_CLOSE_NULL_HANDLE(m_hExit)
    }

    bool RzIPCFileMapping::Write(const char* pData)
    {
        if (!m_bIsWriter) {
            TRACE("%s Reader should not write data.", __FUNCTION__);
            return false;
        }

        if (!IsReady()) {
            TRACE("%s Not ready.", __FUNCTION__);
            return false;
        }

        const auto dataSize = sizeof(char) * strlen(pData) + 1;
        auto hFileMapping = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            nullptr,
            PAGE_READWRITE,
            0,
            dataSize,
            FILE_MAPPING_NAME);

        if (hFileMapping == nullptr) {
            TRACE("%s Create file mapping failed, error: %d.", __FUNCTION__, ::GetLastError());
            return false;
        }

        auto lpMapAddress = MapViewOfFile(
            hFileMapping,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            0);

        if (lpMapAddress == nullptr) {
            CloseHandle(hFileMapping);

            TRACE("%s Map view file failed, error: %d.", __FUNCTION__, ::GetLastError());
            return false;
        }

        strcpy_s((LPSTR)lpMapAddress, dataSize, pData);

        // tell reader to read data
        SetEvent(m_hWrite);
        TRACE("%s Tell reader there is data.", __FUNCTION__);

        // give time to read
        constexpr int WAITTIME = 10000;
        auto dwWiat = WaitForSingleObject(m_hRead, WAITTIME);
        if (WAIT_TIMEOUT == dwWiat) {
            TRACE("%s Wait reader time out.", __FUNCTION__);
        }
        else {
            TRACE("%s Reader read the data", __FUNCTION__);
        }

        // destroy resource
        UnmapViewOfFile(lpMapAddress);
        CloseHandle(hFileMapping);

        return true;
    }

    bool RzIPCFileMapping::Read(std::string& read)
    {
        auto hMapping = OpenFileMapping(FILE_MAP_READ, FALSE, FILE_MAPPING_NAME);
        if (hMapping == nullptr) {
            TRACE("%s Open file mapping failed, error: %d.", __FUNCTION__, ::GetLastError());
            return false;
        }

        auto lpMapView = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
        if (lpMapView == nullptr) {
            CloseHandle(hMapping);
            TRACE("%s Mapp view file failed, error: %d.", __FUNCTION__, ::GetLastError());
            return false;
        }

        read = static_cast<const char*>(lpMapView);

        // tell writer already read
        SetEvent(m_hRead);

        TRACE("%s Read data, tell writer.", __FUNCTION__);

        UnmapViewOfFile(lpMapView);
        CloseHandle(hMapping);

        return true;
    }

    bool RzIPCFileMapping::IsReady() const
    {
        if (nullptr == m_hWrite) {
            TRACE("%s m_hWrite is nullptr.", __FUNCTION__);
            return false;
        }

        if (nullptr == m_hRead) {
            TRACE("%s m_hRead is nullptr.", __FUNCTION__);
            return false;
        }

        return true;
    }


    bool RzIPCFileMapping::OnRead(const std::function<void(std::string)>& callback)
    {
        if (!IsReady()) {
            return false;
        }

        if (m_bIsWriter) {
            TRACE("%s Write shold not call OnRead.", __FUNCTION__);
            return false;
        }

        if (m_IsReading) {
            TRACE("%s Alreay in read.", __FUNCTION__);
            return false;
        }

        TRACE("%s =>", __FUNCTION__);

        m_hExit = CreateEvent(nullptr, FALSE, FALSE, EXIT_EVENT_NAME);

        m_threadRead = std::thread([this](const std::function<void(std::string)>& callback) {
            constexpr int handleNum = 2;
            std::array<HANDLE, handleNum> handles{ m_hWrite, m_hExit };
            while (!m_bExitRead) {
                auto dwWait = WaitForMultipleObjects(handleNum, handles.data(), false, INFINITE);
                if (WAIT_OBJECT_0 == dwWait) {
                    std::string read;
                    if (Read(read)) {
                        callback(read);
                    }
                    else {
                        TRACE("%s Read data wrong, error: %d.", __FUNCTION__, ::GetLastError());
                    }
                }
            }

            
        }, callback);

        m_IsReading = true;

        TRACE("%s =><=", __FUNCTION__);

        return true;
    }
}


