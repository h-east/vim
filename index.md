(Since 2015-12-07)  
Update: 2016-08-29  
Add `How to get the patch`.  
  
Update: 2016-05-15  
The 'wildmode' option value name `clpum` change to `popup`.  
Remove 'clcompleteopt' options value `longest`.

### What is Vim-CLPUM?

vim-CLPUM is implementation of Command-Line mode PopUp Menu for Vim.  
Modified Vim's C source code.  


![clpum1](https://github.com/h-east/vim/wiki/images/clpum1.png)

![clpum1](https://github.com/h-east/vim/wiki/images/clpumDemo02.gif)

![clpum2](https://github.com/h-east/vim/wiki/images/clpum2.png)

***
### How to Use
- Add the following sample code to your `.vimrc`.
```vim
if has('clpum')
	set wildmode=popup
	set wildmenu
	set clpumheight=40
	set clcompletefunc=UserDefinedClComplete
	function! UserDefinedClComplete(findstart, base)
		if a:findstart
			return getcmdpos()
		else
			return [
			\   { 'word': 'Jan', 'menu': 'January' },
			\   { 'word': 'Feb', 'menu': 'February' },
			\   { 'word': 'Mar', 'menu': 'March' },
			\   { 'word': 'Apr', 'menu': 'April' },
			\   { 'word': 'May', 'menu': 'May' },
			\ ]
		endif
	endfunc
endif
```
- Run patched Vim.
- Type \<Tab\> in command-line mode.  (\<Tab\> has been set by 'wildchar' option)  
CLPUM activate by pressing the \<Tab\> in command-line mode. (When 'wildmode' option include 'popup', and 'wildmenu' is on)

example:  
```vim
:set <Tab>
:colorscheme <Tab>
:set wild<Tab>
:h time<Tab>
```
NOTE:
- Key operation during the CLPUM basically conforms to the following contents of help.  
`:h popupmenu-completion`  
`:h popupmenu-keys`
- Content to be complemented is the same as the command-line completion of the insert mode.  
`:h compl-vim`
- When 'cmdheight'-option is 2 or more, messages are displayed on the last line.
- If you want to use user defined completion in command-line mode, press \<C-X>.  
(Need to set the 'clcompletefunc' options.)

***
### Added/Changed Options, Event and etc.
**Option:** 'wildmenu', 'clcompletefunc', 'clcompleteopt', 'clpumheight'  
**Built-in Function:** clpumvisible()  
**Event:** ClCompleteDone  
**Predefined Vim variables:** v:clcompleted_item  
**Highlight groups:** ClPmenu, ClPmenuSel, ClPmenuSbar, ClPmenuThumb  

***
### Progress

Phase | %
--- | ---
Implement | 100
Debug and refactor | 70
Write documents | 25
Write tests | 0
Send patch | -
Included this patch to Vim | -

***
### How to get the patch
```bash
$ curl -L -k https://github.com/h-east/vim/compare/master...h-east:clpum.patch > clpum.patch
```

***
### Goal
Want to be included this patch to Vim.

### License
Follow the [Vim license](https://github.com/vim/vim/blob/master/README.md#copying).

### Author
h_east (@h-east)  
[Search me in vim_dev](https://groups.google.com/forum/#!searchin/vim_dev/hirohito-higashi%7Csort:date)
