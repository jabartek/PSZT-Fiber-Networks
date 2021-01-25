import platform

system = platform.system().lower()


test = ARGUMENTS.get('test', 0)
env = Environment() 

env.Append(CPPPATH = ['#/include', '#/lib/pugixml/include'])
VariantDir('#/src', '#/build', duplicate=0)
resources=Glob('#/src/*.cpp')
if system == "windows":
    env.Append(LINKFLAGS='/SUBSYSTEM:CONSOLE') 
    env.Append(CXXFLAGS='/EHsc /MD /std:c++17 -O2')
else:
    env.Append(CXXFLAGS="-std=c++17")

env.Program( '#/bin/PSZT_FIBER', resources)

