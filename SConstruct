#env = Environment(CPPPATH = ['src'])
env = Environment()

# XXX: It's not working :(
# env.Append(CCFLAGS='-DUINPUT_DEV_PATH="/dev/misc/uinput"')

emsuinput_srcs = ['src/emsuinput.c']
emsuinput_a = env.StaticLibrary('emsuinput', emsuinput_srcs)
emsuinput_so = env.SharedLibrary('emsuinput', emsuinput_srcs)

Default(emsuinput_a)

fakemouse = env.Program('fakemouse', ['example/fakemouse.c', emsuinput_a], CPPPATH = ['src'])
env.Alias('example', fakemouse)
