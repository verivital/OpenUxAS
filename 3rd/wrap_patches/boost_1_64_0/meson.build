project('BoostLibs', 'cpp')

os = target_machine.system()
if os == 'linux'
  add_project_arguments('-DLINUX', language: ['c', 'cpp'])
endif

incs_boost = include_directories(
  '.'
)

lib = static_library(
  'boostlibs',
  [
    'libs/atomic/src/lockpool.cpp',
    'libs/chrono/src/chrono.cpp',
    'libs/chrono/src/process_cpu_clocks.cpp',
    'libs/chrono/src/thread_clock.cpp',
    'libs/date_time/src/gregorian/date_generators.cpp',
    'libs/date_time/src/gregorian/greg_month.cpp',
    'libs/date_time/src/gregorian/greg_weekday.cpp',
    'libs/date_time/src/gregorian/gregorian_types.cpp',
    'libs/date_time/src/posix_time/posix_time_types.cpp',
    'libs/exception/src/clone_current_exception_non_intrusive.cpp',
    'libs/filesystem/src/codecvt_error_category.cpp',
    'libs/filesystem/src/operations.cpp',
    'libs/filesystem/src/path.cpp',
    'libs/filesystem/src/path_traits.cpp',
    'libs/filesystem/src/portability.cpp',
    'libs/filesystem/src/unique_path.cpp',
    'libs/filesystem/src/utf8_codecvt_facet.cpp',
    'libs/filesystem/src/windows_file_codecvt.cpp',
    'libs/program_options/src/cmdline.cpp',
    'libs/program_options/src/config_file.cpp',
    'libs/program_options/src/convert.cpp',
    'libs/program_options/src/options_description.cpp',
    'libs/program_options/src/parsers.cpp',
    'libs/program_options/src/positional_options.cpp',
    'libs/program_options/src/split.cpp',
    'libs/program_options/src/utf8_codecvt_facet.cpp',
    'libs/program_options/src/value_semantic.cpp',
    'libs/program_options/src/variables_map.cpp',
    'libs/program_options/src/winmain.cpp',
    'libs/regex/src/c_regex_traits.cpp',
    'libs/regex/src/cpp_regex_traits.cpp',
    'libs/regex/src/cregex.cpp',
    'libs/regex/src/fileiter.cpp',
    'libs/regex/src/icu.cpp',
    'libs/regex/src/instances.cpp',
    'libs/regex/src/posix_api.cpp',
    'libs/regex/src/regex.cpp',
    'libs/regex/src/regex_debug.cpp',
    'libs/regex/src/regex_raw_buffer.cpp',
    'libs/regex/src/regex_traits_defaults.cpp',
    'libs/regex/src/static_mutex.cpp',
    'libs/regex/src/usinstances.cpp',
    'libs/regex/src/w32_regex_traits.cpp',
    'libs/regex/src/wc_regex_traits.cpp',
    'libs/regex/src/wide_posix_api.cpp',
    'libs/regex/src/winstances.cpp',
    'libs/system/src/error_code.cpp',
    'libs/thread/src/future.cpp',
    'libs/thread/src/pthread/once.cpp',
    'libs/thread/src/pthread/once_atomic.cpp',
    'libs/thread/src/pthread/thread.cpp',
    'libs/thread/src/tss_null.cpp',
    'libs/timer/src/auto_timers_construction.cpp',
    'libs/timer/src/cpu_timer.cpp',
  ],
  cpp_args: [
    '-std=c++11',
  ],
  include_directories: incs_boost,
)

dep_boost = declare_dependency(
  link_with: lib,
  include_directories: incs_boost,
)
