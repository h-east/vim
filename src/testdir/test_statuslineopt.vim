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
enddef

def s:get_statusline_str(winid: number): list<string>
  if has('gui_running')
    redraw!
    sleep 1m
  endif
  var wi = getwininfo(winid)[0]
  var winh = wi.winrow + wi.height
  var lines = [winh, winh + wi.status_height - 1]
  return map(g:ScreenLines(lines, &columns), (_, v) =>
              v[wi.wincol - 1 : wi.wincol - 1 + wi.width - 1])
enddef

def s:get_statuslines_old(height: number): list<string>
  if has('gui_running')
    redraw!
    sleep 1m
  endif
  var lines = [ &lines - &ch - height + 1, &lines - &ch]
  return g:ScreenLines(lines, &columns)
enddef

def Test_statuslineopt()
  set statuslineopt=maxheight:2
  set statusline=AAA
  var wid = win_getid()
  var stlh = getwininfo(wid)[0].status_height
  assert_equal(2, stlh)
  var stls = s:get_statusline_str(wid)
  assert_equal(2, len(stls))
  assert_match('^AAA *', stls[0])
  assert_match('^ *', stls[1])

  set statusline=AAA%@BBB
  stlh = getwininfo()[0].status_height
  assert_equal(2, stlh)
  stls = s:get_statusline_str(wid)
  assert_equal(2, len(stls))
  assert_match('^AAA *', stls[0])
  assert_match('^BBB *', stls[1])

  set statusline=AAA%@BBB%@CCC
  stlh = getwininfo()[0].status_height
  assert_equal(2, stlh)
  stls = s:get_statusline_str(wid)
  assert_equal(2, len(stls))
  assert_match('^AAA *', stls[0])
  assert_match('^BBB *', stls[1])

  set statuslineopt=maxheight:3
  stlh = getwininfo()[0].status_height
  assert_equal(3, stlh)
  stls = s:get_statusline_str(wid)
  assert_equal(3, len(stls))
  assert_match('^AAA *', stls[0])
  assert_match('^BBB *', stls[1])
  assert_match('^CCC *', stls[2])

  set statusline=AAA%@BBB
  stlh = getwininfo()[0].status_height
  assert_equal(3, stlh)
  stls = s:get_statusline_str(wid)
  assert_equal(3, len(stls))
  assert_match('^AAA *', stls[0])
  assert_match('^BBB *', stls[1])
  assert_match('^ *', stls[2])

  # Best effort
  set statusline=AAA%@BBB%@CCC
  set statuslineopt=maxheight:999
  stlh = getwininfo()[0].status_height
  assert_equal(&lines - &ch - 1, stlh)
  stls = s:get_statusline_str(wid)
  assert_equal(stlh, len(stls))
  assert_match('^AAA *', stls[0])
  assert_match('^BBB *', stls[1])
  assert_match('^CCC *', stls[2])
  assert_match('^ *', stls[3])
enddef

" vim: shiftwidth=2 sts=2 expandtab
