/**
 * @file logger.cc
 * @Author Gianfranco Gallizia (skyglobe83@gmail.com)
 * @brief Log facility class
 *
 * GEOtop logging facility
 */

#include "logger.h"

using namespace geotop::logger;

Logger::Logger(std::ostream* stream)
{
    struct LogStream ls;
    ls.streamP = stream;
    ls.severity = DEFAULT_SEVERITY;
    mStreams.push_back(ls);
}

Logger::Logger()
{
    struct LogStream ls;
    ls.streamP = (std::ostream*)(&std::clog);
    ls.severity = DEFAULT_SEVERITY;
    mStreams.push_back(ls);
}

Logger::Logger(std::ostream* stream, severity_levels minSeverity)
{
    struct LogStream ls;
    ls.streamP = stream;
    ls.severity = minSeverity;
    mStreams.push_back(ls);
}

Logger::~Logger()
{
}

void Logger::addOStream(std::ostream* stream, severity_levels minSeverity)
{
    struct LogStream ls;
    ls.streamP = stream;
    ls.severity = minSeverity;

    //Checking for duplicates
    int foundDuplicate = 0;
    std::vector<LogStream>::iterator it = mStreams.begin();
    while (it != mStreams.end())
    {
        if (it->streamP == stream)
        {
            foundDuplicate = 1;
            break;
        }
        it++;
    }

    if (!foundDuplicate)
        mStreams.push_back(ls);
}

void Logger::log(std::string const &message, severity_levels severity)
{
    try
    {
        std::ostream* myStream;
        std::vector<LogStream>::iterator it = mStreams.begin();
        while (it != mStreams.end()){
            myStream = it->streamP;
            if (severity >= it->severity)
            *myStream << "[" << severity_labels[severity] << "]: "
                     << message << std::endl;
            myStream->flush();
            it++;
        }
    }
    catch(...)
    {
        //Catch and ignore any exception thrown
    }
}

void Logger::logf(const char* format, ...)
{
    char buffer[MAXMESSAGESIZE];
    va_list args;
    int chs;

    va_start(args, format);
    chs = vsprintf(buffer, format, args);

    if (chs != -1 && chs < MAXMESSAGESIZE)
    {
        std::string msg(buffer);
        log(msg, DEFAULT_SEVERITY);
    }
}

void Logger::logsf(severity_levels severity, const char* format, ...)
{
    char buffer[MAXMESSAGESIZE];
    va_list args;
    int chs;

    va_start(args, format);
    chs = vsprintf(buffer, format, args);

    if (chs != -1 && chs < MAXMESSAGESIZE)
    {
        std::string msg(buffer);
        log(msg, severity);
    }
}

void Logger::writeAll(std::string const &message)
{
    try
    {
        std::ostream* myStream;
        std::vector<LogStream>::iterator it = mStreams.begin();
        while (it != mStreams.end()){
            myStream = it->streamP;
            *myStream << message;
            myStream->flush();
            it++;
        }
    }
    catch(...)
    {
        //Catch and ignore any exception thrown
    }
}

void Logger::writefAll(const char* format, ...)
{
    char buffer[MAXMESSAGESIZE];
    va_list args;
    int chs;

    va_start(args, format);
    chs = vsprintf(buffer, format, args);

    if (chs != -1 && chs < MAXMESSAGESIZE)
    {
        std::string msg(buffer);
        writeAll(msg);
    }
}
