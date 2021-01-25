test = ARGUMENTS.get('test', 0)
env = Environment() 

env.Append(CPPPATH = ['#/include', '#/lib/pugixml/include'])
env.Append(LINKFLAGS='/SUBSYSTEM:CONSOLE') 
env.Append(CXXFLAGS='/EHsc /MD /std:c++17 -O2')
VariantDir('#/src', '#/build', duplicate=0)
resources=Glob('#/src/*.cpp')
env.Program( '#/bin/PSZT_FIBER', resources)