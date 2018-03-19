//=====================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=====================================================================

#pragma once

// c++
#include <ctime>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <mutex>
#include <sys/stat.h>

// Infra
#include "spdlog/spdlog.h"

// Local
#include "rgOptionalRef.h"

// Internally linked symbols
static const std::string STR_ERR_NO_AVAILABLE_SLOTS    = "Error: the maximum number of LOG thread slots exceeded, unable to print the log message.";
static const std::string STR_ERR_CANNOT_WRITE_LOG_FILE = "Error: failed to write to the log file.";
static const std::string STR_ASSERT_MESSAGE_HEADER     = "Assert failed at ";
static const std::string STR_PROC_ID_PREFIX            = "PID: ";

#ifdef _WIN32
static const std::string STR_DIRECTORY_DELIMITER = "\\";
#else
static const std::string STR_DIRECTORY_DELIMITER = "/";
#endif


//
// Table of output streams for threads.
// This class is used by rgLog and should not be used directly.
//
class StreamTable
{
    // Element of Stream Table.
    //  m_tid       -- thread ID.
    //  m_stream    -- stringstream corresponding to tid.
    //  m_isUsed    -- indicates whether this slot is currently used by some thread.
    //  m_doFlush   -- indicates whether std::endl flushes stream to file/console.
    struct StreamTableSlot
    {
        std::thread::id               m_tid;
        std::stringstream             m_stream;
        volatile std::atomic<bool>    m_isUsed;
        volatile std::atomic<bool>    m_doFlush;

        StreamTableSlot() { m_isUsed = false; m_doFlush = true; }
    };

public:
    // Constructor
    StreamTable() : m_tableSize(0) {}

    // Get the output stream for the thread with provided ID.
    // If matching slot is not found in the table, create a new slot.
    // Returns an optional reference to the found stream.
    // This function also gets or sets the "doFlush" attribute of the found stream (depending on "setDoFlush" value).
    rgOptionalRef<std::stringstream>
    GetStream(std::thread::id tid, bool& doFlush, bool setDoFlush = false)
    {
        rgOptionalRef<std::stringstream>  ret = false;
        if (rgOptionalRef<StreamTableSlot>  slot = GetSlot(tid))
        {
            if (setDoFlush)
            {
                std::atomic_store(&slot->m_doFlush, doFlush);
            }
            else
            {
                doFlush = std::atomic_load(&slot->m_doFlush);
            }
            ret = slot->m_stream;
        }
        return ret;
    }

    // Mark the slot for the thread with provided ID as "not used" so that new threads can use it.
    bool  ClearSlot(std::thread::id tid)
    {
        for (uint32_t i = 0, size = std::atomic_load(&m_tableSize); i < size && i < ms_STREAM_TABLE_SIZE; i++)
        {
            if (m_streams[i].m_tid == tid)
            {
                std::atomic_store(&m_streams[i].m_isUsed, false);
                return true;
            }
        }
        return false;
    }

private:
    // Get stream table slot corresponding to provided tid.
    // If no existing slot is found, a new slot is be created.
    rgOptionalRef<StreamTableSlot> GetSlot(std::thread::id& tid)
    {
        rgOptionalRef<StreamTableSlot>  ret = false;

        // First, look for slot with thread ID = tid.
        for (uint32_t i = 0, size = std::atomic_load(&m_tableSize); i < size && i < ms_STREAM_TABLE_SIZE; i++)
        {
            auto& slot = m_streams[i];
            if (std::atomic_load(&slot.m_isUsed) && slot.m_tid == tid)
            {
                ret = slot;
            }
        }
        // If slot is not found, try to find unused slot.
        for (uint32_t i = 0, size = std::atomic_load(&m_tableSize); !ret && i < size && i < ms_STREAM_TABLE_SIZE; i++)
        {
            auto& slot = m_streams[i];
            bool  expectedVal = false;
            if (std::atomic_compare_exchange_strong(&slot.m_isUsed, &expectedVal, true))
            {
                slot.m_tid = tid;
                slot.m_stream.str("");
                ret = slot;
            }
        }
        // Failed to find unused slot -- create a new slot.
        if (!ret)
        {
            uint32_t  offset = m_tableSize.fetch_add(1);
            if (offset < ms_STREAM_TABLE_SIZE)
            {
                auto& slot = m_streams[offset];
                std::atomic_store(&(slot.m_isUsed), true);
                slot.m_tid = tid;
                ret = slot;
            }
            else
            {
                // Restore old table size and return {-1, empty_stream}.
                m_tableSize.fetch_sub(1);
            }
        }
        return ret;
    }


    // Number of streams in the Stream Table.
    static const size_t  ms_STREAM_TABLE_SIZE = 64;

    // Empty stringstream
    std::stringstream  m_emptyStream;

    // Array of slots
    std::array<StreamTableSlot, ms_STREAM_TABLE_SIZE>  m_streams;

    // Number of currently active (used) slots.
    std::atomic<uint32_t>                             m_tableSize;
};


//
// rgLog
//
class rgLog
{
    // Logging destination.
    enum LogDst
    {
        _LogFile_,
        _StdOut_,
        _StdErr_
    };

    // Logging severity level.
    enum LogLvl
    {
        _None_,
        _Minimal_,
        _Info_,
        _Debug_,
        _Trace_
    };

    // Internal types for Logging severity levels.
    typedef struct L_MINIMAL { static const LogLvl ms_LVL = LogLvl::_Minimal_; }  LVL_MINIMAL;
    typedef struct L_INFO    { static const LogLvl ms_LVL = LogLvl::_Info_;    }  LVL_INFO;
    typedef struct L_DEBUG_  { static const LogLvl ms_LVL = LogLvl::_Debug_;   }  LVL_DEBUG;
    typedef struct L_TRACE   { static const LogLvl ms_LVL = LogLvl::_Trace_;   }  LVL_TRACE;

public:
    // ========== Public types/interfaces ==========

    // Helper empty class for overloading functions/operators for different log destinations/severity levels.
    template <rgLog::LogDst DST, rgLog::LogLvl LVL>
    class Log
    {
    public:
        template <typename NEW_LVL>
        Log<DST, NEW_LVL::ms_LVL> operator()(NEW_LVL) { return Log<DST, NEW_LVL::ms_LVL>(); }
    };

    // Destinations.
    static Log<LogDst::_LogFile_, LogLvl::_Info_>   file;
    static Log<LogDst::_StdOut_,  LogLvl::_Info_>   stdOut;
    static Log<LogDst::_StdErr_,  LogLvl::_Info_>   stdErr;

    // Log severity levels.
    static LVL_MINIMAL  Minimal;
    static LVL_INFO     Info;
    static LVL_DEBUG    Debug;
    static LVL_TRACE    Trace;

    // Open a log file.
    // The arguments are:
    //   - the folder where the log file should be created.
    //   - the name of the log file to create.
    //   - the severity level (optional, default is "Info").
    // Returns "true" if successfully created a log file and "false" otherwise.
    template<typename T = LVL_INFO>
    static bool  OpenLogFile(const std::string& folder, const std::string& fileName, T logLevel = Info)
    {
        bool  ret = false;
        struct stat  s;
        if (stat(folder.c_str(), &s) == 0)
        {
            std::string  filePath = folder + STR_DIRECTORY_DELIMITER + fileName;
            ret = OpenLogFile(filePath, logLevel);
        }
        return ret;
    }

    // Open a log file.
    // The arguments are:
    //   - the full path to the log file to create.
    //   - the severity level (optional, default is "Info").
    // Returns "true" if successfully created a log file and "false" otherwise.
    template<typename T = LVL_INFO>
    static bool  OpenLogFile(const std::string& filePath, T logLevel = Info)
    {
        bool  ret = false;
        std::ignore = logLevel;
        std::get<LogDst::_LogFile_>(ms_data.m_levels) = T::ms_LVL;
        try
        {
            // Open a thread-safe unlimited log file using spdlog interface.
            std::string  procID = STR_PROC_ID_PREFIX + std::to_string(GetProcessID());
            ms_data.m_pLogger = spdlog::basic_logger_mt(procID, filePath);
            spdlog::set_level(spdlog::level::trace);
            ret = true;
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
        catch (...)
        {
            assert(false);
            std::cerr << STR_ERR_CANNOT_WRITE_LOG_FILE << std::endl;
        }

        return ret;
    }

    // Create a log file with date/time suffix.
    // Example:
    //    fileNamePrefix = "test.log"
    //    Created file: test-YYYYMMDD-HHMMSS.log
    template<typename T = LVL_INFO>
    static bool  OpenStadardLogFile(const std::string& folder, const std::string& fileName, T logLevel = Info)
    {
        bool  ret = false;
        std::ignore = logLevel;
        std::get<LogDst::_LogFile_>(ms_data.m_levels) = T::ms_LVL;
        struct stat  s;
        if (stat(folder.c_str(), &s) == 0)
        {
            std::string  procID = STR_PROC_ID_PREFIX + std::to_string(GetProcessID());
            ret = OpenStdLogFile(folder, fileName, procID);
        }
        return ret;
    }

    // Close the log file.
    static void  Close()
    {
        if (ms_data.m_pLogger != nullptr)
        {
            ms_data.m_pLogger->flush();
        }
    }

    // Close the thread slot associated with the current thread.
    // This must be called by a thread that does not need logging any more.
    static bool  CloseThreadLog()
    {
        return ms_data.m_streamTable.ClearSlot(std::this_thread::get_id());
    }

    // Set the logging severity level for file, stdOut or StrErr.
    template <LogDst DST, LogLvl LVL, typename NEW_LVL>
    static void SetLevel(Log<DST, LVL>& log, NEW_LVL)
    {
        std::get<DST>(ms_data.m_levels) = NEW_LVL::ms_LVL;
    }

    // Add "end of line" and flush the stream associated with the current thread to the log file.
    template <LogDst DST, LogLvl LVL>
    static void  endl(const Log<DST, LVL>&)
    {
        bool  doFlush;
        if (auto  stream = ms_data.m_streamTable.GetStream(std::this_thread::get_id(), doFlush))
        {
            stream.get() << std::endl;
            if (doFlush)
            {
                Push<DST>(stream.get());
            }
        }
        else
        {
            DumpError(STR_ERR_NO_AVAILABLE_SLOTS);
        }
    }

    // Disables "flush to destination stream" performed by using std::endl.
    // The next call to "rgLog::flush" will restore flushing behavior.
    template<LogDst DST, LogLvl LVL>
    static void  noflush(const Log<DST, LVL>&)
    {
        // Set "doFlush" to false.
        bool  doFlush = false;
        std::ignore = ms_data.m_streamTable.GetStream(std::this_thread::get_id(), doFlush, true);
    }

    // Flush the stream associated with the current thread to the log file.
    template<LogDst DST, LogLvl LVL>
    static void  flush(const Log<DST, LVL>&)
    {
        bool  doFlush = true;
        if (auto  stream = ms_data.m_streamTable.GetStream(std::this_thread::get_id(), doFlush, true))
        {
            Push<DST>(stream.get());
        }
    }

    // Overloaded operators << for passing data to the log m_streams.
    template<LogDst DST, LogLvl LVL, typename T> friend
    const Log<DST, LVL>&  operator<<(const Log<DST, LVL>& log, const T& data)
    {
        if (CheckLevel<DST>(LVL))
        {
            bool  doFlush;
            if (auto stream = ms_data.m_streamTable.GetStream(std::this_thread::get_id(), doFlush))
            {
                stream.get() << data;
            }
            else
            {
                DumpError(STR_ERR_NO_AVAILABLE_SLOTS);
            }
        }
        return log;
    }

    // Overloaded operator << for rgLog::noflush, rgLog::flush etc.
    template<LogDst DST, LogLvl LVL> friend
    const Log<DST, LVL>& operator<<(const Log<DST, LVL>& log, void(*func)(const Log<DST, LVL>&))
    {
        if (CheckLevel<DST>(LVL))
        {
            func(Log<DST, LVL>());
        }
        return log;
    }

    // Overloaded operator << for supporting STL stream manipulators
    template<LogDst DST, LogLvl LVL> friend
    const Log<DST, LVL>& operator<<(const Log<DST, LVL>& log, std::ostream&(*f)(std::ostream&))
    {
        if (CheckLevel<DST>(LVL))
        {
            bool  doFlush;
            if (auto  stream = ms_data.m_streamTable.GetStream(std::this_thread::get_id(), doFlush))
            {
                f(stream.get());
                if (f == static_cast<std::ostream& (*)(std::ostream&)>(std::endl))
                {
                    Push<DST>(stream.get());
                }
            }
            else
            {
                DumpError(STR_ERR_NO_AVAILABLE_SLOTS);
            }
        }
        return log;
    }

private:
    // ========== Internal types/methods ==========

    rgLog() = delete;

    static bool  OpenStdLogFile(const std::string& folder, const std::string& baseFileName, const std::string& logName)
    {
        bool ret = false;
        if (auto logger = OpenFileLogger(folder, baseFileName, logName))
        {
            ms_data.m_pLogger = logger;
            spdlog::set_level(spdlog::level::trace);
            ret = true;
        }
        return ret;
    }

    static std::shared_ptr<spdlog::logger>
    OpenFileLogger(const std::string& folder, const std::string& baseFileName, const std::string& logName)
    {
        std::shared_ptr<spdlog::logger>  logger = nullptr;
        std::string  dateTime = ConstructDateTimeString();
        if (!dateTime.empty())
        {
            size_t  offset = baseFileName.rfind('.');
            std::string  baseFilePath = folder + STR_DIRECTORY_DELIMITER + baseFileName;
            offset += (folder.size() + STR_DIRECTORY_DELIMITER.size());
            std::string  filePath = baseFilePath;
            std::string  suffix = dateTime;
            filePath.insert((offset == std::string::npos ? filePath.size() : offset), suffix);

            // Keep adding "-<DATE>-<TIME>-XX" suffix to the file name and trying to open the file until
            // we're able to open the file for exclusive writing (XX = i).
            for (int i = 0; i < ms_MAX_SAME_DATE_LOG_FILES_NUM; i++)
            {
                bool  tryNext = false;
                try
                {
                    logger = spdlog::basic_logger_mt(logName, filePath);
                }
                catch (std::exception&)
                {
                    tryNext = true;
                }

                if (tryNext)
                {
                    filePath = baseFilePath;
                    std::string  suffix = dateTime + "-";
                    suffix += ((i < 10 ? "0" : "") + std::to_string(i));
                    filePath.insert((offset == std::string::npos ? filePath.size() : offset), suffix);
                    continue;
                }

                break;
            }
        }
        return logger;
    }

    // Constructs a string that contains current date & time in the format: "YYYYMMDD-HHMMSS"
    static std::string  ConstructDateTimeString()
    {
        time_t  currentTime = time(0);
        std::stringstream s;
        bool success = false;
#ifdef _WIN32
        struct tm  tt;
        struct tm* pT = &tt;
        success = (localtime_s(&tt, &currentTime) == 0);
#else
        struct tm*  pT = localtime(&currentTime);
        success = (pT != nullptr);
#endif
        if (success)
        {
            s << std::to_string(pT->tm_year + 1900) << std::to_string(pT->tm_mon + 1) << std::to_string(pT->tm_mday) <<
                 "-" << std::to_string(pT->tm_hour) << std::to_string(pT->tm_min) << std::to_string(pT->tm_sec);
        }
        return s.str();
    }

    // Checks if a message should be processed based on provided message level and current log file level.
    template<LogDst DST>
    inline static bool  CheckLevel(LogLvl lvl)
    {
        return (lvl <= std::get<DST>(ms_data.m_levels));
    }

    // Push the content of the stream for the thread with provided thread ID to the log file or to console.
    // Clear the content of the stream after pushing it to the log file.
    template<LogDst DST>
    static inline void  Push(std::stringstream& stream);

    // Print error message directly to the log file.
    static void  DumpError(const std::string& msg)
    {
        assert(ms_data.m_pLogger != nullptr);
        try
        {
            ms_data.m_pLogger->error(msg);
            ms_data.m_pLogger->flush();
        }
        catch (...)
        {
            std::cerr << STR_ERR_CANNOT_WRITE_LOG_FILE << std::endl;
        }
    }

    // Return ID of current process.
    static int GetProcessID()
    {
#ifdef _WIN32
        return GetCurrentProcessId();
#else
        return ::getpid();
#endif
    }

    struct LogData
    {
        std::shared_ptr<spdlog::logger>     m_pLogger = nullptr;
        std::tuple<LogLvl, LogLvl, LogLvl>  m_levels  = std::make_tuple(LogLvl::_Info_, LogLvl::_Info_, LogLvl::_Info_);
        std::mutex                          m_consoleLock;
        StreamTable                         m_streamTable;
    };

    // Maximum number of log files with the same data/time.
    static const int  ms_MAX_SAME_DATE_LOG_FILES_NUM = 64;

    // LOG data
    static LogData    ms_data;
};


// Specialization of Push method.
template<>
inline void  rgLog::Push<rgLog::LogDst::_StdOut_>(std::stringstream& stream)
{
    std::lock_guard<std::mutex>  lock(ms_data.m_consoleLock);
    std::cout << stream.str();
    std::cout.flush();
    stream.str("");
}

template<>
inline void  rgLog::Push<rgLog::LogDst::_StdErr_>(std::stringstream& stream)
{
    std::lock_guard<std::mutex>  lock(ms_data.m_consoleLock);
    std::cerr << stream.str();
    std::cerr.flush();
    stream.str("");
}

template<>
inline void  rgLog::Push<rgLog::LogDst::_LogFile_>(std::stringstream& stream)
{
    try
    {
        ms_data.m_pLogger->info(stream.str());
        ms_data.m_pLogger->flush();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...) {}

    stream.str("");
}

#ifdef _WIN32
#define WEAK_LINK __declspec(selectany)
#else
#define WEAK_LINK __attribute__((weak))
#endif

// Static data members of class rgLog
WEAK_LINK rgLog::LogData  rgLog::ms_data;

WEAK_LINK rgLog::Log<rgLog::LogDst::_LogFile_, rgLog::LogLvl::_Info_>   rgLog::file;
WEAK_LINK rgLog::Log<rgLog::LogDst::_StdOut_,  rgLog::LogLvl::_Info_>   rgLog::stdOut;
WEAK_LINK rgLog::Log<rgLog::LogDst::_StdErr_,  rgLog::LogLvl::_Info_>   rgLog::stdErr;

WEAK_LINK rgLog::LVL_MINIMAL  rgLog::Minimal;
WEAK_LINK rgLog::LVL_INFO     rgLog::Info;
WEAK_LINK rgLog::LVL_DEBUG    rgLog::Debug;
WEAK_LINK rgLog::LVL_TRACE    rgLog::Trace;
