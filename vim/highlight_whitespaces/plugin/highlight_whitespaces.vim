" Name: highlight_whitespaces
" Author: Philipp Middendorf <pmidden@gmx.net>
" Version: 1.0
" License: Same terms as Vim itself (see :help license)

if exists('g:pimiddy_highlight_whitespaces')
	finish
endif

let g:pimiddy_highlight_whitespaces = 1

autocmd ColorScheme * highlight ExtraWhitespace ctermbg=red guibg=red
autocmd FileType * call s:HighlightWhitespacesIfPossible()

function! s:HighlightWhitespacesIfPossible()
	if count(g:whitespace_highlight_exclusion,&filetype) == 0
		highlight ExtraWhitespace ctermbg=red guibg=red
		match ExtraWhitespace /\s\+$/
"		2match ExtraWhitespace /^ \+/
	else
		match
	endif
endfunction
