_tests = ['meteoio_01.cc', 'meteoio_02.cc', 'meteoio_03.cc', 'meteoio_04.cc']
#_tests = ['meteoio_02.cc']

_src = ['../../../src/geotop/meteoio_wrapper.h']
foreach _t : _tests
    _e = executable(_t, [_t,_src], dependencies: [gtest,meteoio],  include_directories: include_dirs)
    test(_t, _e, suite: ['unit_tests','meteoio'])
endforeach
