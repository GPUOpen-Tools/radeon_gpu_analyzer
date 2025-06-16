//=============================================================================
/// Copyright (c) 2017-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for rga logger.
//=============================================================================

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
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable : 4459)
    #pragma warning(disable : 4996)
#endif
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local
#include "common/rg_optional_ref.h"

// Internally linked symbols
static const std::string kStrErrNoAvailableSlots    = "Error: the maximum number of LOG thread slots exceeded, unable to print the log message.";
static const std::string kStrErrCannotWriteLogFile = "Error: failed to write to the log file.";
static const std::string kStrAssertMessageHeader     = "Assert failed at ";
static const std::string kStrProcIdPrefix            = "PID: ";

#ifdef _WIN32
static const std::string kStrDirectoryDelimiter = "\\";
#else
static const std::string kStrDirectoryDelimiter = "/";
#endif

//
// Table of output streams_ for threads.
// This class is used by RgLog and should not be used directly.
//
class StreamTable
{
    // Element of Stream Table.
    //  tid       -- thread ID.
    //  stream    -- stringstream corresponding to tid.
    //  is_used    -- indicates whether this slot is currently used by some thread.
    //  do_flush   -- indicates whether std::endl flushes stream to file/console.
    struct StreamTableSlot
    {
        std::thread::id               tid;
        std::stringstream             stream;
        volatile std::atomic<bool>    is_used;
        volatile std::atomic<bool>    do_flush;

        StreamTableSlot() { is_used = false; do_flush = true; }
    };

public:
    // Constructor
    StreamTable() : table_size_(0) {}

    // Get the output stream for the thread with provided ID.
    // If matching slot is not found in the table, create a new slot.
    // Returns an optional reference to the found stream.
    // This function also gets or sets the "do_flush" attribute of the found stream (depending on "set_do_flush" value).
    RgOptionalRef<std::stringstream>
    GetStream(std::thread::id tid, bool& do_flush, bool set_do_flush = false)
    {
        RgOptionalRef<std::stringstream>  ret = false;
        if (RgOptionalRef<StreamTableSlot>  slot = GetSlot(tid))
        {
            if (set_do_flush)
            {
                std::atomic_store(&slot->do_flush, do_flush);
            }
            else
            {
                do_flush = std::atomic_load(&slot->do_flush);
            }
            ret = slot->stream;
        }
        return ret;
    }

    // Mark the slot for the thread with provided ID as "not used" so that new threads can use it.
    bool  ClearSlot(std::thread::id tid)
    {
        for (uint32_t i = 0, size = std::atomic_load(&table_size_); i < size && i < kMsStreamTableSize; i++)
        {
            if (streams_[i].tid == tid)
            {
                std::atomic_store(&streams_[i].is_used, false);
                return true;
            }
        }
        return false;
    }

private:
    // Get stream table slot corresponding to provided tid.
    // If no existing slot is found, a new slot is be created.
    RgOptionalRef<StreamTableSlot> GetSlot(std::thread::id& tid)
    {
        RgOptionalRef<StreamTableSlot>  ret = false;

        // First, look for slot with thread ID = tid.
        for (uint32_t i = 0, size = std::atomic_load(&table_size_); i < size && i < kMsStreamTableSize; i++)
        {
            auto& slot = streams_[i];
            if (std::atomic_load(&slot.is_used) && slot.tid == tid)
            {
                ret = slot;
            }
        }
        // If slot is not found, try to find unused slot.
        for (uint32_t i = 0, size = std::atomic_load(&table_size_); !ret && i < size && i < kMsStreamTableSize; i++)
        {
            auto& slot = streams_[i];
            bool  expected_val = false;
            if (std::atomic_compare_exchange_strong(&slot.is_used, &expected_val, true))
            {
                slot.tid = tid;
                slot.stream.str("");
                ret = slot;
            }
        }
        // Failed to find unused slot -- create a new slot.
        if (!ret)
        {
            uint32_t  offset = table_size_.fetch_add(1);
            if (offset < kMsStreamTableSize)
            {
                auto& slot = streams_[offset];
                std::atomic_store(&(slot.is_used), true);
                slot.tid = tid;
                ret = slot;
            }
            else
            {
                // Restore old table size and return {-1, empty_stream}.
                table_size_.fetch_sub(1);
            }
        }
        return ret;
    }

    // Number of streams_ in the Stream Table.
    static const size_t  kMsStreamTableSize = 64;

    // Empty stringstream
    std::stringstream  empty_stream_;

    // Array of slots
    std::array<StreamTableSlot, kMsStreamTableSize>  streams_;

    // Number of currently active (used) slots.
    std::atomic<uint32_t>                             table_size_;
};

//
// RgLog
//
class RgLog
{
    // Logging destination.
    enum LogDst
    {
        k_log_file_,
        k_std_out_,
        k_std_err_
    };

    // Logging severity level.
    enum LogLvl
    {
        k_none_,
        k_minimal_,
        k_info_,
        k_debug_,
        k_trace_
    };

    // Internal types for Logging severity levels.
    typedef struct L_MINIMAL { static const LogLvl ms_LVL = LogLvl::k_minimal_; }  LVL_MINIMAL;
    typedef struct L_INFO    { static const LogLvl ms_LVL = LogLvl::k_info_;    }  LVL_INFO;
    typedef struct L_DEBUG_  { static const LogLvl ms_LVL = LogLvl::k_debug_;   }  LVL_DEBUG;
    typedef struct L_TRACE   { static const LogLvl ms_LVL = LogLvl::k_trace_;   }  LVL_TRACE;

public:
    // ========== Public types/interfaces ==========

    // Helper empty class for overloading functions/operators for different log destinations/severity levels.
    template <RgLog::LogDst DST, RgLog::LogLvl LVL>
    class Log
    {
    public:
        template <typename NEW_LVL>
        Log<DST, NEW_LVL::ms_LVL> operator()(NEW_LVL) { return Log<DST, NEW_LVL::ms_LVL>(); }
    };

    // Destinations.
    static Log<LogDst::k_log_file_, LogLvl::k_info_>   file;
    static Log<LogDst::k_std_out_,  LogLvl::k_info_>   stdOut;
    static Log<LogDst::k_std_err_,  LogLvl::k_info_>   stdErr;

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
            std::string  filePath = folder + kStrDirectoryDelimiter + fileName;
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
        std::get<LogDst::k_log_file_>(ms_data.levels) = T::ms_LVL;
        try
        {
            // Open a thread-safe unlimited log file using spdlog interface.
            std::string  procID = kStrProcIdPrefix + std::to_string(GetProcessID());
            ms_data.logger = spdlog::basic_logger_mt(procID, filePath);
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
            std::cerr << kStrErrCannotWriteLogFile << std::endl;
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
        std::get<LogDst::k_log_file_>(ms_data.levels) = T::ms_LVL;
        struct stat  s;
        if (stat(folder.c_str(), &s) == 0)
        {
            std::string  procID = kStrProcIdPrefix + std::to_string(GetProcessID());
            ret = OpenStdLogFile(folder, fileName, procID);
        }
        return ret;
    }

    // Close the log file.
    static void  Close()
    {
        if (ms_data.logger != nullptr)
        {
            ms_data.logger->flush();
        }
    }

    // Close the thread slot associated with the current thread.
    // This must be called by a thread that does not need logging any more.
    static bool  CloseThreadLog()
    {
        return ms_data.streamTable.ClearSlot(std::this_thread::get_id());
    }

    // Set the logging severity level for file, stdOut or StrErr.
    template <LogDst DST, LogLvl LVL, typename NEW_LVL>
    static void SetLevel(Log<DST, LVL>& log, NEW_LVL)
    {
        std::get<DST>(ms_data.levels) = NEW_LVL::ms_LVL;
    }

    // Add "end of line" and flush the stream associated with the current thread to the log file.
    template <LogDst DST, LogLvl LVL>
    static void  endl(const Log<DST, LVL>&)
    {
        bool  do_flush;
        if (auto  stream = ms_data.streamTable.GetStream(std::this_thread::get_id(), do_flush))
        {
            stream.get() << std::endl;
            if (do_flush)
            {
                Push<DST>(stream.get());
            }
        }
        else
        {
            DumpError(kStrErrNoAvailableSlots);
        }
    }

    // Disables "flush to destination stream" performed by using std::endl.
    // The next call to "RgLog::flush" will restore flushing behavior.
    template<LogDst DST, LogLvl LVL>
    static void  noflush(const Log<DST, LVL>&)
    {
        // Set "do_flush" to false.
        bool  do_flush = false;
        std::ignore = ms_data.streamTable.GetStream(std::this_thread::get_id(), do_flush, true);
    }

    // Flush the stream associated with the current thread to the log file.
    template<LogDst DST, LogLvl LVL>
    static void  flush(const Log<DST, LVL>&)
    {
        bool  do_flush = true;
        if (auto stream = ms_data.streamTable.GetStream(std::this_thread::get_id(), do_flush, true))
        {
            Push<DST>(stream.get());
        }
    }

    // Overloaded operators << for passing data to the log streams_.
    template<LogDst DST, LogLvl LVL, typename T> friend
    const Log<DST, LVL>&  operator<<(const Log<DST, LVL>& log, const T& data)
    {
        if (CheckLevel<DST>(LVL) && ms_data.logger != nullptr)
        {
            bool  do_flush;
            if (auto stream = ms_data.streamTable.GetStream(std::this_thread::get_id(), do_flush))
            {
                stream.get() << data;
            }
            else
            {
                DumpError(kStrErrNoAvailableSlots);
            }
        }
        return log;
    }

    // Overloaded operator << for RgLog::noflush, RgLog::flush etc.
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
        if (CheckLevel<DST>(LVL) && ms_data.logger != nullptr)
        {
            bool  do_flush;
            if (auto stream = ms_data.streamTable.GetStream(std::this_thread::get_id(), do_flush))
            {
                f(stream.get());
                if (f == static_cast<std::ostream& (*)(std::ostream&)>(std::endl) ||
                    f == static_cast<std::ostream& (*)(std::ostream&)>(std::flush))
                {
                    Push<DST>(stream.get());
                }
            }
            else
            {
                DumpError(kStrErrNoAvailableSlots);
            }
        }
        return log;
    }

private:
    // ========== Internal types/methods ==========

    RgLog() = delete;

    static bool  OpenStdLogFile(const std::string& folder, const std::string& baseFileName, const std::string& logName)
    {
        bool ret = false;
        if (auto logger = OpenFileLogger(folder, baseFileName, logName))
        {
            ms_data.logger = logger;
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
            std::string  baseFilePath = folder + kStrDirectoryDelimiter + baseFileName;
            offset += (folder.size() + kStrDirectoryDelimiter.size());
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
                    std::string  suffix2 = dateTime + "-";
                    suffix2 += ((i < 10 ? "0" : "") + std::to_string(i));
                    filePath.insert((offset == std::string::npos ? filePath.size() : offset), suffix2);
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
        time_t  current_time = time(0);
        std::stringstream s;
        bool success = false;
#ifdef _WIN32
        struct tm  tt;
        struct tm* pT = &tt;
        success = (localtime_s(&tt, &current_time) == 0);
#else
        struct tm*  pT = localtime(&current_time);
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
        return (lvl <= std::get<DST>(ms_data.levels));
    }

    // Push the content of the stream for the thread with provided thread ID to the log file or to console.
    // Clear the content of the stream after pushing it to the log file.
    template<LogDst DST>
    static inline void  Push(std::stringstream& stream);

    // Print error message directly to the log file.
    static void  DumpError(const std::string& msg)
    {
        assert(ms_data.logger != nullptr);
        try
        {
            if (ms_data.logger != nullptr)
            {
                ms_data.logger->error(msg);
                ms_data.logger->flush();
            }
            else
            {
                std::cerr << msg << std::endl;
            }
        }
        catch (...)
        {
            std::cerr << kStrErrCannotWriteLogFile << std::endl;
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
        std::shared_ptr<spdlog::logger>     logger = nullptr;
        std::tuple<LogLvl, LogLvl, LogLvl>  levels  = std::make_tuple(LogLvl::k_info_, LogLvl::k_info_, LogLvl::k_info_);
        std::mutex                          console_lock;
        StreamTable                         streamTable;
    };

    // Maximum number of log files with the same data/time.
    static const int  ms_MAX_SAME_DATE_LOG_FILES_NUM = 64;

    // LOG data
    static LogData    ms_data;
};

// Specialization of Push method.
template<>
inline void  RgLog::Push<RgLog::LogDst::k_std_out_>(std::stringstream& stream)
{
    std::lock_guard<std::mutex>  lock(ms_data.console_lock);
    std::cout << stream.str();
    std::cout.flush();
    stream.str("");
}

template<>
inline void  RgLog::Push<RgLog::LogDst::k_std_err_>(std::stringstream& stream)
{
    {
        std::lock_guard<std::mutex>  lock(ms_data.console_lock);
        std::cerr << stream.str();
        std::cerr.flush();
    }

    // Copy the message to the log file.
    try
    {
        ms_data.logger->info(stream.str());
        ms_data.logger->flush();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...) {}

    stream.str("");
}

template<>
inline void  RgLog::Push<RgLog::LogDst::k_log_file_>(std::stringstream& stream)
{
    try
    {
        ms_data.logger->info(stream.str());
        ms_data.logger->flush();
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

// Static data members of class RgLog
WEAK_LINK RgLog::LogData  RgLog::ms_data;

WEAK_LINK RgLog::Log<RgLog::LogDst::k_log_file_, RgLog::LogLvl::k_info_>   RgLog::file;
WEAK_LINK RgLog::Log<RgLog::LogDst::k_std_out_,  RgLog::LogLvl::k_info_>   RgLog::stdOut;
WEAK_LINK RgLog::Log<RgLog::LogDst::k_std_err_,  RgLog::LogLvl::k_info_>   RgLog::stdErr;

WEAK_LINK RgLog::LVL_MINIMAL  RgLog::Minimal;
WEAK_LINK RgLog::LVL_INFO     RgLog::Info;
WEAK_LINK RgLog::LVL_DEBUG    RgLog::Debug;
WEAK_LINK RgLog::LVL_TRACE    RgLog::Trace;
