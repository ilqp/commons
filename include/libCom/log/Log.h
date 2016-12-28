#ifndef LIBCOM_LOG_H
#define LIBCOM_LOG_H

#include <logger/LogLevel.h>

#include <vector>
#include <ostream>

/**
 * Interface a Logger has to implement.
 * For every log stream (a chain of << calls) wantsLog(LogLevel) is called. If it returns true, logged values will be
 * passed to ILogger implementation.
 */
class ILogger {
public:
    /**
     * Virtual destructor
     */
    virtual ~ILogger() {};

    /**
     * Opens the output stream. This enables logging.
     * @return Whether opening succeeded.
     */
    virtual bool open() = 0;

    /**
     * Closes the output stream. No logging is possible after calling this function until the ILogger is opened again.
     */
    virtual void close() = 0;

    /**
     * @return Whether the ILogger is ready for logging.
     */
    virtual bool isOpen() = 0;

    /**
     * @return Underlying writable std::ostream stream.
     */
    virtual std::ostream &stream() = 0;

    /**
     * @param level LogLevel
     * @return Whether ILogger implementation wants to log stream started by this log level.
     */
    virtual bool wantsLog(LogLevel level) = 0;

};

template <LogLevel Level>
class LogStream;

/**
 * General purpose logging class. ILoggers can be registered to direct different log levels to different outputs.
 */
class Log {
public:
    /**
     * Registers an ILogger to the Log. The ILogger will be opened (if it's not already), if this fails, the ILogger
     * won't be added!
     * @param logger ILogger to register
     */
    void registerLogger(ILogger *logger);

    /**
     * Unregisters an ILogger from the Log. The ILogger will be closed (if it's not already).
     * @param logger ILogger to unregister
     */
    void unregisterLogger(ILogger *logger);

    /**
     * Trace log level
     */
    static LogStream<LogLevel::TRACE> trac;
    /**
     * Debug log level
     */
    static LogStream<LogLevel::DEBUG> dbg;
    /**
     * Info log level
     */
    static LogStream<LogLevel::INFO> info;
    /**
     * Warning log level
     */
    static LogStream<LogLevel::WARNING> warn;
    /**
     * Error log level
     */
    static LogStream<LogLevel::ERROR> err;

    /**
     * @return Singleton Log instance
     */
    static Log &get() {
        return mInstance;
    }

protected:
    Log() { }

    std::vector<ILogger *> mLoggers;
    static Log mInstance;

    template <LogLevel Level>
    friend class LogStream;
};

/**
 * Provides a wrapper for std::ostream that streams all logged values to Log's registered loggers.
 * @tparam Level Assigned LogLevel
 */
template <LogLevel Level>
class LogStream {
    /**
     * Chained stream operator calls will use the following class, that logs only if a ILogger declared it wants to log
     * the current stream.
     */
    class LogStreamValue {
    public:
        /**
         * Constructor that accepts the parent LogStream and a vector of enabled ILoggers for this log stream.
         * @param parent
         * @param enabledLoggers
         */
        LogStreamValue(LogStream &parent, std::vector<ILogger*> enabledLoggers) : mParent(parent),
                                                                                  mEnabledLoggers(enabledLoggers) { }
        /**
         * Stream operator which is used for logging. This only logs the actual value by passing them to LogStream.
         * @tparam T Type of value to be logged
         * @param t Value to log
         */
        template<typename T>
        LogStreamValue &operator<<(const T &t) {
            for (ILogger *logger: mEnabledLoggers) {
                if (logger->isOpen())
                    logger->stream() << t;
            }
            return *this;
        }

    protected:
        LogStream &mParent;
        std::vector<ILogger*> mEnabledLoggers;
    };

public:
    /**
     * Stream operator which is used for logging. This redirects all logged values to the registered ILoggers.
     * The subsequent values will be logged by returned LogStreamValue.
     * @tparam T Type of value to be logged
     * @param t Value to log
     * @return Chained log stream, that accepts all other values
     */
    template<typename T>
    LogStreamValue operator<<(const T &t) {
        std::vector<ILogger *> enabledLoggers;
        enabledLoggers.reserve(mLog.mLoggers.size());

        for (ILogger *logger: mLog.mLoggers) {
            // only log if logger wants this level
            if (logger->wantsLog(Level)) {
                // add to list of enabled loggers
                enabledLoggers.push_back(logger);

                // log the first value if it's open
                if (logger->isOpen())
                    logger->stream() << t;
            }
        }
        return LogStreamValue(*this, enabledLoggers);
    }

protected:

    /**
     * @param log Global Log
     */
    LogStream(Log &log) : mLog(log) { }

    Log &mLog;

    friend class Log;
};

#endif //LIBCOM_LOG_H
