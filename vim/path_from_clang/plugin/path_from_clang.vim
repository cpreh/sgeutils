" Name: path_from_clang
" Author: Philipp Middendorf <pmidden@gmx.net>
" Version: 1.0
" License: Same terms as Vim itself (see :help license)

if exists('g:loaded_path_from_clang')
	finish
endif

let g:loaded_path_from_clang = 1

autocmd FileType c,cpp,objc,objcpp call s:PathFromClangInit()

function! s:PathFromClangInit()
	let l:local_conf = findfile('.clang_complete', getcwd() . ',.;')

	if l:local_conf != '' && filereadable(l:local_conf)
		let l:lst = readfile(l:local_conf)
		let l:lstdirect = copy(l:lst)
		let l:lstsystem = copy(l:lst)
		call filter(l:lstdirect, 'v:val =~ "\\C^-I"')
		call filter(l:lstsystem, 'v:val =~ "\\C^-isystem"')
		call map(l:lstdirect, 'v:val[2:]')
		call map(l:lstsystem, 'v:val[9:]')

		let &path = join(l:lstdirect + l:lstsystem,',')
	endif
endfunction
