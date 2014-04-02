/** 
 * @file global_logger.cc 
 * @Author Gianfranco Gallizia (skyglobe83@gmail.com) 
 * 
 */ 

#include "geotop_common.h"
#include <boost/filesystem/path.hpp>
#include "global_logger.h"

using namespace geotop::logger;

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *           Defaults             *
 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

bool GlobalLogger::instanceFlag = false;
GlobalLogger* GlobalLogger::single = NULL;

/**
 * @internal
 * @brief Constructor
 *
 * Creates a new GlobalLogger and attaches it to the log file(s)
 */
GlobalLogger::GlobalLogger()
{
    logfileStream.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    
    try
    {
        logfileStream.open(getLogFilePath(), std::ofstream::out | std::ofstream::trunc);
    }
    catch (std::ofstream::failure e)
    {
        std::cerr << "[CRITICAL]: Unable to open log file " << getLogFilePath() << std::endl;
        std::cerr << "[DEBUG]: " << e.what() <<std::endl;
        exit(1);
    }

    if (logfileStream.is_open())
    {
        mLogger.addOStream(&logfileStream, mLogger.DEFAULT_SEVERITY);
    }
}

const char* GlobalLogger::getLogFilePath()
{
    boost::filesystem::path myPath;
    myPath /= geotop::common::Variables::WORKING_DIRECTORY;
    //TODO: once the migration will be complete use the correct path
    //myPath /= geotop::common::Variables::logfile;
    myPath /= "geotop_new.log";
    
    return (const char*)myPath.c_str();
}

void GlobalLogger::setSeverity(severity_levels minSeverity)
{

    logfileStream.close();
    
    logfileStream.open(getLogFilePath(),
            std::ios_base::trunc | std::ios_base::out);

    mLogger.~Logger();

    mLogger = Logger((std::ostream*)(&std::clog), minSeverity);
    mLogger.addOStream(&logfileStream, minSeverity);

}

GlobalLogger* GlobalLogger::getInstance()
{
    if (!instanceFlag)
    {
        single = new GlobalLogger();
        instanceFlag = true;
    }
    return single;
}

GlobalLogger::~GlobalLogger()
{
    instanceFlag = false;
    delete single;
}

//Logger wrap methods

void GlobalLogger::log(std::string const &message, severity_levels severity)
{
    mLogger.log(message, severity);
}

void GlobalLogger::logf(const char* format, ...)
{
    char buffer[Logger::MAXMESSAGESIZE];
    va_list args;
    int chs;

    va_start(args, format);
    chs = vsprintf(buffer, format, args);

    if (chs != -1 && chs < Logger::MAXMESSAGESIZE)
    {
        std::string msg(buffer);
        mLogger.log(msg, Logger::DEFAULT_SEVERITY);
    }
}

void GlobalLogger::logsf(severity_levels severity, const char* format, ...)
{
    char buffer[Logger::MAXMESSAGESIZE];
    va_list args;
    int chs;

    va_start(args, format);
    chs = vsprintf(buffer, format, args);

    if (chs != -1 && chs < Logger::MAXMESSAGESIZE)
    {
        std::string msg(buffer);
        mLogger.log(msg, severity);
    }
}

void GlobalLogger::writeAll(std::string const &message)
{
    mLogger.writeAll(message);
}

void GlobalLogger::writefAll(const char* format, ...)
{
    char buffer[Logger::MAXMESSAGESIZE];
    va_list args;
    int chs;

    va_start(args, format);
    chs = vsprintf(buffer, format, args);

    if (chs != -1 && chs < Logger::MAXMESSAGESIZE)
    {
        std::string msg(buffer);
        mLogger.writeAll(msg);
    }
}

