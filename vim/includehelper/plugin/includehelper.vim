if exists('g:IncludeHelper_Loaded')
	finish
endif

let g:IncludeHelper_Loaded = 1

" Open begin
let openlistener = {}

function openlistener.onComplete(item, method)
	edit `=a:item`
endfunction

function openlistener.onAbort()
	echo "Abort"
endfunction
" Open end

" Insert begin
let insertlistener = {}

function insertlistener.onComplete(item, method)
	call append(line("."),'#include <'.substitute(a:item,'.*include/\(.*\)','\1','').'>')
endfunction

function insertlistener.onAbort()
	echo "Abort"
endfunction
" Insert end

function! AddIncludeLine()
	call inputsave()
	let name = input('#include <')
	call inputrestore()

	call append(0,'#include <'.name.'>')
endfunction

function! RegenerateIncludePaths()
	let g:edited_include_paths = []
	" Set some standard paths (strictly not 'topic' of this script, but helpful
	let &path = './include,/usr/include,/usr/local/include,~/local/include'
	for item in g:paths
		let g:edited_include_paths = g:edited_include_paths + split(glob(item.'/include/**/*.hpp'),'\n')
		let &path = &path.'/include/,'.item
	endfor
endfunction

command! RegenerateIncludePaths call RegenerateIncludePaths()
command! OpenIncludePath call fuf#callbackitem#launch('', 0, '>', openlistener, g:edited_include_paths, 0)
command! InsertIncludeDirective call fuf#callbackitem#launch('', 0, '#include <', insertlistener, g:edited_include_paths, 0)
command! AddIncludeLine call AddIncludeLine()

RegenerateIncludePaths
