gtest = dependency('gtest', main: true,
                   fallback : ['gtest', 'gtest_main_dep'],
                   required: false)
gtest_without_main = dependency('gtest', main: false,
                                fallback : ['gtest', 'gtest_dep'],
                                required: false)
#ifdef WITH_METEOIO
dirs = ['logger', 'vector', 'asserts', 'formats', 'matrix', 'tensor', 'timer', 'meteoio']
#else
dirs = ['logger', 'vector', 'asserts', 'formats', 'matrix', 'tensor', 'timer']
#endif

if (gtest.found())
    foreach _d : dirs
       subdir(_d)
    endforeach
else
    warning('gtest was not found in your system nor could be downloaded from internet. Skipping test suite "unit_tests".')
endif

