# Module HashColon/Log

_HashColon/log.hpp_

## Simple Examples

### GlobalLogger: Thread-safe & global multi-stream for output.

```c++
using Avikus::SingletonCLI;
using Avikus::GlobalLogger;

int main(int argc, char *argv[])
{
    // Initialize CLI
    SingletonCLI()::Initialize();
    // Initialize GlobalLogger with parameters of CLI/config file/ENV
    GlobalLogger_::Initialize();

    try
    {
        // Parse parameters from CLI/config file/ENV
        SingletonCLI::GetInstance().Parse(argc, argv, {});

        /********************************************
         * Use Global logger to log something
        *********************************************/
       GlobalLogger.Log() << "Simple log." << std::endl;
       GlobalLogger.Error() << "Simple error log" << std::endl;
       GlobalLogger.Debug() << "Simple debug log" << std::endl;
       GlobalLogger.Message() << "Simple message" << std::endl;

       GlobalLogger.Log({Tag::lvl, 10}) << "lvl tag example: This log is not printed." << std::endl;
       GlobalLogger.Log({Tag::lvl, 10}, {Tag::maxlvl, 11}) << "lvl tag example: This log is printed." << std::endl;
       GlobalLogger.Error({__CODEINFO_TAGS__}) << "Error message with debug information" << std::endl;
    }
     catch (const CLI::Error &e)
    {
        return SingletonCLI::GetInstance().GetCLI()->exit(e);
    }
    catch (const HashColon::Exception &e)
    {
        // set logger
        Avikus::CommonLogger logger;
        logger.Error({{Tag::file, e.file()}, {Tag::func, e.func()}, {Tag::line, e.line()}}) << e.what() << std::endl;
    }    
}
```

**_Expected output_**
```shell
```

