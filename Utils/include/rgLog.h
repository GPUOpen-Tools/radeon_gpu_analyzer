//=====================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=====================================================================

// c++
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <functional>

// Infra
#include <spdlog/spdlog.h>

// Internally linked symbols
static const std::string STR_ERR_NO_AVAILABLE_SLOTS  = "Error: the maximum number of LOG thread slots exceeded, unable to print the log message.";
static const std::string STR_ASSERT_MESSAGE_HEADER   = "Assert failed at ";

//
// Usage example:
//
//   #include "shared/include/Log.h"
//
//   static bool  threadDone = false;
//
//   void threadFunc()
//   {
//       Log<LOG::Normal> << "Thread started.";
//       Log<LOG::Detailed> << " The thread ID is: " << std::this_thread::get_id();
//       Log<LOG::Normal> << LOG::endl;
//
//       for (thread_local int i = 0; i < 256; i++) {
//           Log<LOG::Debug> << "Thread doing iteration " << i << LOG::endl;
//           Sleep(10);
//       }
//
//       threadDone = true;
//   }
//
//   int main()
//   {
//       Log<LOG::StdOut> << "Working..." << LOG::endl;
//       LOG::Open("test.log", LOG::Normal);
//       Log<LOG::Normal> << "Starting thread..." << LOG::endl;
//       std::thread T(threadFunc);
//       T.join();
//       LOG::Assert(threadDone, "Thread does not seem to have finished correctly.");
//       Log<LOG::StdOut> << "Done." << LOG::endl;
//   }

class LOG
{
public:
    // Logging severity level.
    //   "None"                   - do not log anything.
    //   "Minimal" - "Detailed"   - different levels of logging to a log file.
    //   "Debug"                  - print to log file only with DEBUG build.
    //   "StdErr"/"StdOut"        - print to console instead of log file.
    enum Level
    {
        None = 0,
        Minimal,
        Normal,
        Detailed,
        Debug,
        StdErr,
        StdOut
    };

    // Helper empty class for overloading functions/operators for different severity levels.
    template<LOG::Level LVL = Level::Normal>
    class Logger
    {
    public:
        Logger() = default;
        Logger(std::function<void(void)> func) {}
    };

    // Open a log file. If a log file with the provided name exists, it will be overwritten.
    // The arguments are:
    //   - the name of the log file to create.
    //   - the severity level (optional, default is "Normal").
    //   - the internal name of the log that will be printed as a prefix to all messages in the log file (optional, default is empty).
    // Return "true" if successfully created a log file and "false" otherwise.
    static bool  Open(const std::string& logFileName, Level level = Level::Normal, const std::string& logName = "")
    {
        m_data.level = level;
        m_data.logger = spdlog::basic_logger_mt(logName, logFileName);
        return (m_data.logger != nullptr);
    }

    // Set the logging severity level.
    static void  SetLevel(Level lvl)
    {
        m_data.level = lvl;
    }

    // Close the log file.
    static bool  Close()
    {
        m_data.logger->flush();
        return true;
    }

    // Close the thread slot associated with the current thread.
    // This must be called by a thread that does not need logging any more.
    static bool  CloseThreadLog()
    {
        return m_data.streams.ClearSlot(std::this_thread::get_id());
    }

    // Checks if provided condition is true and prints assert failure message to the log file if it's not.
    // Adds user-provided message to the error message.
    // In Debug build, makes the program crash if condition is not true.
    static void  Assert(bool cond, const std::string& userMsg = "")
    {
        if (!cond)
        {
            Logger<LOG::Normal>  log;
            log << STR_ASSERT_MESSAGE_HEADER << __FILE__ << ":" << __LINE__;
            if (!userMsg.empty())
            {
                log << " : " << userMsg;
            }
            log << LOG::endl;
        }
        assert(cond);
    }

    // Debug version of Assert. Does nothing in Release build.
    static void  AssertDebug(bool cond, const std::string& msg = "")
    {
#ifndef NDEBUG
        Assert(cond, msg);
#endif
    }

    // Add "end of line" and flush the stream associated with the current thread to the log file.
    template<Level LVL>
    static void  endl(const Logger<LVL>&)
    {
        std::thread::id  tid = std::this_thread::get_id();
        auto  stream = m_data.streams.GetStream(tid);
        if (stream.first)
        {
            if (LVL == Level::StdErr || LVL == Level::StdOut)
            {
                stream.second << std::endl;
            }
            Push(tid, LVL);
        }
        else
        {
            DumpError(STR_ERR_NO_AVAILABLE_SLOTS);
        }
    }

    // Flush the stream associated with the current thread to the log file.
    template<Level LVL>
    static void  flush(const Logger<LVL>&)
    {
        Push(std::this_thread::get_id(), LVL);
    }

    // Overloaded operators << for passing data to the log streams.
    template<Level LVL, typename T> friend
    const Logger<LVL>&  operator<<(const Logger<LVL>& lg, const T& data)
    {
        if (LOG::CheckLevel(LVL))
        {
            std::thread::id  tid = std::this_thread::get_id();
            auto  stream = m_data.streams.GetStream(tid);
            if (stream.first)
            {
                stream.second << data;
            }
            else
            {
                DumpError(STR_ERR_NO_AVAILABLE_SLOTS);
            }
        }
        return lg;
    }

    // Overloaded operator << for LOG::endl and LOG::flush
    template<Level LVL> friend
    const Logger<LVL>& operator<<(const Logger<LVL>& lg, void(*func)(const Logger<LVL>&))
    {
        if (LOG::CheckLevel(LVL))
        {
            func(lg);
        }
        return lg;
    }

private:
    static const size_t  STREAM_TABLE_SIZE = 64;

    // Checks if a message should be processed based on provided message level and current log file level.
    static bool  CheckLevel(Level msgLevel)
    {
        bool ret = false;
        Level logLevel = m_data.level;
#ifdef NDEBUG
        if (msgLevel != Level::Debug && (msgLevel <= logLevel || msgLevel == Level::StdOut || msgLevel == Level::StdErr))
#else
        if (msgLevel == Level::Debug || msgLevel <= logLevel || msgLevel == Level::StdOut || msgLevel == Level::StdErr)
#endif
        {
            ret = true;
        }
        return ret;
    }

    // Push the content of the stream for the thread with provided thread ID to the log file or to console.
    // Clear the content of the stream after pushing it to the log file.
    static void  Push(const std::thread::id& tid, Level lvl)
    {
        auto  stream = m_data.streams.GetStream(tid);
        if (stream.first)
        {
            switch (lvl)
            {
            case Level::StdOut:
                std::cout << stream.second.str();
                std::cout.flush();
                break;
            case Level::StdErr:
                std::cerr << stream.second.str();
                std::cerr.flush();
                break;
            default:
                m_data.logger->info(stream.second.str());
                m_data.logger->flush();
            }
            // Clear the stream.
            stream.second.str("");
        }
        else
        {
            DumpError("Error: the maximum number of LOG thread slots exceeded, unable to print the log message.");
        }
    }

    // Print error message directly to the log file.
    static void  DumpError(const std::string& msg)
    {
        m_data.logger->error(msg);
        m_data.logger->flush();
    }

    // Table of output streams for threads.
    class StreamTable
    {
    public:
        StreamTable() : tableSize(0)
        {
            for (StreamTableSlot& slot : streams)
            {
                slot.isUsed = true;
            }
        }

        // Get the output stream for the thread with provided ID.
        // If matching slot is not found in the table, create a new slot.
        std::pair<bool, std::stringstream&>  GetStream(const std::thread::id& tid)
        {
            // First, look for slot with thread ID = tid.
            for (uint32_t  i = 0, size = std::atomic_load(&tableSize); i < size && i < STREAM_TABLE_SIZE; i++)
            {
                auto& slot = streams[i];
                if (std::atomic_load(&slot.isUsed) && slot.tid == tid)
                {
                    return { true, slot.stream };
                }
            }
            // Try to find unused slot.
            for (uint32_t i = 0, size = std::atomic_load(&tableSize); i < size && i < STREAM_TABLE_SIZE; i++)
            {
                auto& slot = streams[i];
                bool  expectedVal = false;
                if (std::atomic_compare_exchange_strong(&slot.isUsed, &expectedVal, true))
                {
                    slot.tid = tid;
                    slot.stream = std::stringstream();
                    return { true, slot.stream };
                }
            }
            // Failed to find unused slot -- create a new slot.
            uint32_t  offset = tableSize.fetch_add(1);
            if (offset < STREAM_TABLE_SIZE)
            {
                std::atomic_store(&streams[offset].isUsed, true);
                streams[offset].tid = tid;
                streams[offset].stream = std::stringstream();
                return { true, streams[offset].stream };
            }
            else
            {
                // Restore old table size.
                tableSize.fetch_sub(1);
                return { false, nullStream };
            }
        }

        // Mark the slot for the thread with provided ID as "not used" so that new threads can use it.
        bool  ClearSlot(std::thread::id tid)
        {
            for (uint32_t i = 0, size = std::atomic_load(&tableSize); i < size && i < STREAM_TABLE_SIZE; i++)
            {
                if (streams[i].tid == tid)
                {
                    std::atomic_store(&streams[i].isUsed, false);
                    return true;
                }
            }
            return false;
        }

    private:
        struct StreamTableSlot
        {
            std::atomic<bool>  isUsed;
            std::thread::id    tid;
            std::stringstream  stream;
        };

        // Stream Table data
        std::array<StreamTableSlot, STREAM_TABLE_SIZE>  streams;
        std::atomic<uint32_t>                           tableSize;
        // "null" stringstream
        std::stringstream                               nullStream;
    };

    struct LogData {
        std::shared_ptr<spdlog::logger>   logger;
        Level                             level;
        StreamTable                       streams;
    };

    // LOG data
    static LogData   m_data;
};

#ifdef _WIN32
#define WEAK_LINK __declspec(selectany)
#else
#define WEAK_LINK __attribute__((weak))
#endif

// Static object of class LOG
WEAK_LINK LOG::LogData  LOG::m_data;

// Const empty instances of classes LOG::Logger for different severity levels.
template<LOG::Level LVL = LOG::Level::Normal>
const class LOG::Logger<LVL>  rgLog;
