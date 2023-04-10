# HashColon.CPP Library

C++ libraries including common functions for server-application development.

This library is inspired by [HashColon.CPP](https://github.com/HashColon/HashColon.CPP/tree/master/HashColon/src).

## Installation 

### Prerequisites

* C++17 support
* External libraries:

| Library | Version | Mandatory/<br/>Conditional | Related Module |
|---------|---------|----------------------------|----------------|
| [hiredis](https://github.com/redis/hiredis) | 1.1.0 | Mandatory | redis-manager |
| [CLI11](https://github.com/CLIUtils/CLI11) | 2.3.2 | Mandatory<br/>_Included in include/CLI11._ | singleton-cli |
| [boost::filesystem](https://www.boost.org/doc/libs/1_65_1/libs/filesystem/doc/index.htm) | 1.65.1 | Optional<br/>_If std::filesystem is partially supported._ | filesystem |
| [boost::regex](https://www.boost.org/doc/libs/1_65_1/libs/regex/doc/html/index.html) | 1.65.1 | Optional, but recommended<br/>_std::regex is notoriously slow. Slower than python._ <br/> _Better stick to boost::regex_. | filesystem<br/> datetime (_if C++20 not supported_)|
| [HowardHinnant/date](https://github.com/HowardHinnant/date) | 3.0.1 | Conditional<br/> _If C++20 not supported._<br/> _Included in include/date, src/date._ | datetime |
| [rapidjson](https://github.com/Tencent/rapidjson) | _recent version_ | Conditional<br/> _If JSON config file is used. Enable it by HASHCOLON_CONFIGURATION_BY_JSON._ | singleton-cli |

### Install 

_I'm seriously thinking about ditching CMAKE, cuz CMAKE sucks._ 

_The Meson build system, on the other hand, is actually pretty nice. And to top it off, python-like code is infinitely superior to that CMAKE shits._

## Doxygen Supports

### Building Doxygen

```shell
sudo apt install doxygen graphviz
cd docs
doxygen 
```
### using Doxygen documentation
Open `docs/html/index.html` with any of the web browser. 

## Modules



