# This is a Meson Build System script.
# Please refer to the following website for more details:
# https://mesonbuild.com/

# Project definition
project(
    'HashColon.CPP',
    'cpp',    
    default_options:['cpp_std=c++17']
)

# souces, headers, dependencies
HashColon_sources = []
HashColon_include_directory = include_directories('include')
HashColon_dependencies = []

# Get compiler information to check compatibility
cpp_compiler = meson.get_compiler('cpp')

###########################################################################

opt_cpp_std = get_option('cpp_std')

# check c++ standard 
assert(
    cpp_compiler.get_define('__cplusplus') >= '201703L',
    'C++17 or later version is required for HashColon.CPP'
)

# ###########################################################################

# # HashColon Modules 

# # HashColon/datetime
# if enable_module_datetime
#     # HashColon dependencies
#     enable_module_exception = true
#     # src files
#     HashColon_sources += ''
# endif


# # sources 
# HashColon_sources = [
#     'src/date/tz.cpp',
#     'src/HashColon/datetime.cpp',
#     'src/HashColon/filesystem.cpp',
#     'src/HashColon/log.cpp',
#     'src/HashColon/real.cpp',
#     'src/HashColon/singleton-cli.cpp'
# ]

# # include directories
# HashColon_include_directory = include_directories('include')

# # additional dependencies may be needed for compatibility
# HashColon_dependencies = [
#     # The following libraries are added inside the source codes
#     # dependency('CLI11', version:'2.3.2')
#     # dependency('date')
# ]

# ###########################################################################

# # Get compiler information to check compatibility
# cpp_compiler = meson.get_compiler('cpp')

# # check c++ standard (at least c++17 of later required)
# assert(
#     cpp_compiler.get_define('__cplusplus') >= '201703L' ,
#     'C++17 or later version is required for HashColon.CPP'
# )

# # some compiler versions(such as gcc 7.5.0) may support c++17 partially,

# # check <filesystem> or <experimental/filesystem>
# if not cpp_compiler.check_header('filesystem')
#     if not cpp_compiler.check_header('experimental/filesystem')
#         # add boost/filesystem to dependency
#         HashColon_dependencies += dependency(
#             'Boost', 
#             version:'>=1.65', 
#             modules: ['filesystem', 'system']
#         )
#     endif
# endif

# # boost::regex is more prefered than std::regex
# boost_regex_dependency = dependency(
#     'Boost', 
#     version:'>=1.65', 
#     modules: ['regex']
# )
# if boost_regex_dependency.found()
#     HashColon_dependencies += boost_regex_dependency
# else
#     assert(
#         cpp_compiler.check_header('regex'), 
#         'boost::regex or std::regex is required for HashColcon.CPP'
#     )
# endif

# # Check Doxygen support
# doxygen_app = find_program('doxygen')

# #########################################################################

# # Library HashColon definition
# library(    
#     'HashColon',
#     HashColon_sources,
#     dependencies: [
#         HashColon_dependencies,
#         dependency('threads')
#     ],
#     include_directories: HashColon_include_directory,

#     # compiler options
#     cpp_args: [
#         '-pthread',
#         '-fpermissive',
#         '-O3'
#     ],
#     pic: true,   # equivalent to -fPIC

#     # linker options
#     link_args:[
#         '-fopenmp',
#         '-pthread',
#         '-O3'
#     ],

#     # enable installation
#     install: true
# )

# # build doxygen
# if doxygen_app.found()
#     run_command(doxygen_app, 'docs/Doxyfile') 
# endif


