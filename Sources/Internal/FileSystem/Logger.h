/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_LOGGER_H__
#define __DAVAENGINE_LOGGER_H__

#define LOG_AS_INT(param) Logger::Debug("%s = %d", #param, param);
#define LOG_AS_FLOAT(param) Logger::Debug("%s = %.4f", #param, param);
#define LOG_AS_BOOL(param) Logger::Debug("%s = %s", #param, param?"true":"false");
#define LOG_AS_MATRIX4(param)   Logger::Debug("%s_0 = %f, %f, %f, %f", #param, param._00, param._01, param._02, param._03); \
                                Logger::Debug("%s_1 = %f, %f, %f, %f", #param, param._10, param._11, param._12, param._13); \
                                Logger::Debug("%s_2 = %f, %f, %f, %f", #param, param._20, param._21, param._22, param._23); \
                                Logger::Debug("%s_2 = %f, %f, %f, %f", #param, param._30, param._31, param._32, param._33);

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"

#include "FileSystem/FilePath.h"

#include <cstdarg>

namespace DAVA 
{

class LoggerOutput;

class Logger: public Singleton<Logger>
{
public:
    enum eLogLevel
    {
        //! Highest log level if it set all message printed out. Use it to separate framework logs from project logs
        LEVEL_FRAMEWORK = 0,

        //! Highest log level at project. If it set all message printed out.
        LEVEL_DEBUG,

        //! Normal log level prints only message not related to debug
        LEVEL_INFO,

        //! Warning messages (for usage if we reached some limits)
        LEVEL_WARNING,

        //! Error messages (critical situations)
        LEVEL_ERROR,

        //! Disable logs
        LEVEL__DISABLE
    };

    Logger();
    virtual ~Logger();

    //! Enables/disables logging to file. Disabled by default.
    //! \param[in] filename: name of log file. Empty string disables logging to file,
    //! non-empty creates log file in working directory.
    virtual void SetLogFilename(const String & filename);

    //! Enables/disables logging to file. Disabled by default.
    //! \param[in] filepath: path to log file. Empty string disables logging to file,
    //! non-empty creates log file described by filepath.
    virtual void SetLogPathname(const FilePath & filepath);

    //! Returns the current set log level.
    virtual eLogLevel GetLogLevel();

    //! Sets a new log level. With this value, texts which are sent to
    //! the logger are filtered out. For example setting this value to
    //! ELL_WARNING, only warnings and
    //! errors are printed out. Setting it to ELL_INFORMATION, which is
    //! the default setting, warnings,
    //! errors and informational texts are printed out.
    //! \param ll: new log level filter value.
    virtual void SetLogLevel(eLogLevel ll);

    //! Prints out a text into the log
    //! \param text: Text to print out.
    //! \param ll: Log level of the text. If the text is an error, set
    //! it to ELL_ERROR, if it is warning set it to ELL_WARNING, and if it
    //! is just an informational text, set it to ELL_INFORMATION. Texts are
    //! filtered with these levels. If you want to be a text displayed,
    //! independent on what level filter is set, use ELL_NONE.
    virtual void Log(eLogLevel ll, const char8* text, ...);
    virtual void Logv(eLogLevel ll, const char8* text, va_list li);

    static void FrameworkDebug(const char8 * text, ...);
    static void Debug(const char8 * text, ...);
    static void Warning(const char8 * text, ...);
    static void Info(const char8 * text, ...);
    static void Error(const char8 * text, ...);

    static void AddCustomOutput(DAVA::LoggerOutput *lo);
    static void RemoveCustomOutput(DAVA::LoggerOutput *lo);

#if defined(__DAVAENGINE_ANDROID__)
    static void SetTag(const char8 *logTag);
#endif    

    void EnableConsoleMode();

    const char8 * GetLogLevelString(eLogLevel ll);

private:
    void PlatformLog(eLogLevel ll, const char8* text);
    void FileLog(eLogLevel ll, const char8* text);
    void CustomLog(eLogLevel ll, const char8* text);
    void ConsoleLog(eLogLevel ll, const char8* text);
    void Output(eLogLevel ll, const char8* formatedMsg);

    eLogLevel logLevel;
    FilePath logFilename;
    Vector<LoggerOutput *> customOutputs;
    bool consoleModeEnabled;

};

class LoggerOutput
{
public:
    LoggerOutput() = default;
    virtual ~LoggerOutput() = default;

    virtual void Output(Logger::eLogLevel ll, const char8* text) = 0;
};

} // end namespace DAVA

#endif // __DAVAENGINE_LOGGER_H__
