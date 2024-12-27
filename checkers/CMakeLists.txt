project("OITools::Stress-Tester::Checkers")

aux_source_directory(. src_list)
set(exe_list)

foreach(src_name ${src_list})
	get_filename_component(chk_name ${src_name} NAME_WE)
	add_executable(chk-${chk_name} ${src_name})
	set(exe_list ${exe_list} chk-${chk_name})
endforeach()

add_custom_target(checkers)
add_dependencies(checkers ${exe_list})
set(checkers_exe_list ${exe_list} PARENT_SCOPE)
