" Test 'statuslineopt' with 'statusline'
"

source util/screendump.vim

def SetUp()
  set laststatus=2
  se ch=1
enddef

def TearDown()
  set laststatus&
  set statusline&
  set statuslineopt&
  set ch&
  :only
enddef

def s:Assert_match_statusline(winid: number, stlh: number, expect: list<string>): void
  if has('gui_running')
    redraw!
    sleep 1m
  endif
  var wi = getwininfo(winid)[0]
  var winh = wi.winrow + wi.height
  var lines = [winh, winh + wi.status_height - 1]
  var actual = mapnew(g:ScreenLines(lines, &columns), (_, v) =>
              v[wi.wincol - 1 : wi.wincol - 1 + wi.width - 1])
  assert_equal(stlh, wi.status_height)
  for i in range(len(expect))
    assert_match(expect[i], actual[i], $'[{i}]')
  endfor
enddef

def Test_statuslineopt()
  set statuslineopt=maxheight:2
  &statusline = "AAA"
  var wid = win_getid()
  var stlh = 2
  s:Assert_match_statusline(wid, stlh, ['^AAA *', '^ *'])
  &statusline = "AAA%@BBB"
  s:Assert_match_statusline(wid, stlh, ['^AAA *', '^BBB *'])
  &statusline = "AAA%@BBB%@CCC"
  s:Assert_match_statusline(wid, stlh, ['^AAA *', '^BBB *'])

  set statuslineopt=maxheight:3
  stlh = 3
  s:Assert_match_statusline(wid, stlh, ['^AAA *', '^BBB *', '^CCC *'])
  &statusline = "AAA%@BBB"
  s:Assert_match_statusline(wid, stlh, ['^AAA *', '^BBB *', '^ *'])

  # Best effort
  &statusline = "AAA%@BBB%@CCC"
  set statuslineopt=maxheight:999
  stlh = &lines - &ch - 1
  s:Assert_match_statusline(wid, stlh, ['^AAA *', '^BBB *', '^CCC *', '^ *'])

  # Single line
  set statuslineopt=maxheight:1
  stlh = 1
  s:Assert_match_statusline(wid, stlh, ['^AAA *'])
enddef

def Test_statuslineopt_multi_win()
  &statusline = "AAA%@BBB%@BB3"
  var wid1 = win_getid()
  new ccc
  &l:statusline = "CCC%@DDD%@DD3"
  var wid2 = win_getid()
  set statuslineopt=maxheight:2
  var stlh = 2
  s:Assert_match_statusline(wid1, stlh, ['^AAA *', '^BBB *'])
  s:Assert_match_statusline(wid2, stlh, ['^CCC *', '^DDD *'])

  vnew eee
  &l:statusline = "EEE%@FFF%@FF3"
  var wid3 = win_getid()
  s:Assert_match_statusline(wid1, stlh, ['^AAA *', '^BBB *'])
  s:Assert_match_statusline(wid2, stlh, ['^CCC *', '^DDD *'])
  s:Assert_match_statusline(wid3, stlh, ['^EEE *', '^FFF *'])

  quit
  new eee
  wid3 = win_getid()
  s:Assert_match_statusline(wid1, stlh, ['^AAA *', '^BBB *'])
  s:Assert_match_statusline(wid2, stlh, ['^CCC *', '^DDD *'])
  s:Assert_match_statusline(wid3, stlh, ['^EEE *', '^FFF *'])

  # Best effort
  set statuslineopt=maxheight:999
  stlh = (&lines - &ch - 3) / 3
  s:Assert_match_statusline(wid1, stlh, ['^AAA *', '^BBB *', '^BB3 *', '^ *'])
  s:Assert_match_statusline(wid2, stlh, ['^CCC *', '^DDD *', '^DD3 *', '^ *'])
  s:Assert_match_statusline(wid3, stlh, ['^EEE *', '^FFF *', '^FF3 *', '^ *'])

  # Single line
  set statuslineopt=maxheight:1
  stlh = 1
  s:Assert_match_statusline(wid1, stlh, ['^AAA *'])
  s:Assert_match_statusline(wid2, stlh, ['^CCC *'])
  s:Assert_match_statusline(wid3, stlh, ['^EEE *'])
enddef

let g:StloStatusVar = ''
def g:StloStatusLine(): string
  return g:StloStatusVar
enddef

def Test_statuslineopt_expr()
  new bbb.txt
  g:StloStatusVar = 'A001%@A002%@%t'
  set statuslineopt=maxheight:3
  &statusline = "%!StloStatusLine()"
  var wid = win_getid()
  var stlh = 3
  s:Assert_match_statusline(wid, stlh, ['^A001 *', '^A002 *', '^bbb\.txt *'])

  &statusline = "%{%StloStatusLine()%}"
  s:Assert_match_statusline(wid, stlh, ['^A001 *', '^A002 *', '^bbb\.txt *'])
enddef

" vim: shiftwidth=2 sts=2 expandtab
