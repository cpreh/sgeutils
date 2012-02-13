if exists('g:IncludeHelper_Loaded')
	finish
endif

let g:IncludeHelper_Loaded = 1

function! AddIncludeLine()
	call inputsave()
	let name = input('#include <')
	call inputrestore()

	if name != ''
		call append(0,'#include <'.name.'>')
	endif
endfunction

command! AddIncludeLine call AddIncludeLine()
