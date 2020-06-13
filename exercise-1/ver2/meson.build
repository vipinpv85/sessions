project('experiement', 'c', version: '0.0.1',license: 'MIT')

sources = files('myqueue.c', 'main.c', )
add_global_arguments('-DMEMIF', language : 'c')
add_global_arguments('-ggdb', language : 'c')

message('host: ' + host_machine.system())
depend = []
#depend = [ dep_pthread, dep_numa]

if host_machine.system() == 'linux'
	cc = meson.get_compiler('c')
	dep_pthread = dependency('threads', required: true)
	dep_numa = cc.find_library('numa', required: true)

	if dep_pthread.found()
		depend += dep_pthread
	endif

	if dep_numa.found()
		depend += dep_numa
	endif
endif

conf = configuration_data()
message('version: ' + meson.project_version())
conf.set_quoted('VERSION', meson.project_version())
configure_file(output : 'config.h', configuration : conf)

#add_project_link_arguments('-pthread', language: 'c')
executable('mytest', sources, dependencies : depend)
