/* vi:set ts=8 sts=4 sw=4:
 *
 * VIM - Vi IMproved	by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */

/*
 * clpum.c: Command-line popup menu (CLPUM)
 */
#include "vim.h"

#if defined(FEAT_CLPUM) || defined(PROTO)

static pumitem_T *clpum_array = NULL;	/* items of displayed pum */
static int clpum_size = -1;		/* nr of items in "clpum_array" */
static int clpum_selected;		/* index of selected item or -1 */
static int clpum_first = 0;		/* index of top item */

static int clpum_height;			/* nr of displayed pum items */
static int clpum_width;			/* width of displayed pum items */
static int clpum_base_width;		/* width of pum items base */
static int clpum_kind_width;		/* width of pum items kind column */
static int clpum_scrollbar;		/* TRUE when scrollbar present */

static int clpum_row;			/* top row of pum */
static int clpum_col;			/* left column of pum */

static int clpum_do_redraw = FALSE;	/* do redraw anyway */

static int clpum_set_selected(int n, int repeat);

#define CLPUM_DEF_HEIGHT 10

/*
 * Show the popup menu with items "array[size]".
 * "array" must remain valid until clpum_undisplay() is called!
 * When possible the leftmost character is aligned with screen column "col".
 * The menu appears above the screen line "row" or at "row" + "height" - 1.
 */
    void
clpum_display(
    pumitem_T	*array,
    int		size,
    int		selected,	/* index of initially selected item, none if
				   out of range */
    int		disp_col)
{
    int		w;
    int		def_width;
    int		max_width;
    int		kind_width;
    int		extra_width;
    int		i;
    int		row;
    int		col;
    int		redo_count = 0;

redo:
    def_width = p_clpw;
    max_width = 0;
    kind_width = 0;
    extra_width = 0;

    /* Pretend the pum is already there to avoid that must_redraw is set when
     * 'cuc' is on. */
    clpum_array = (pumitem_T *)1;
    validate_cursor_col();
    clpum_array = NULL;

    row = cmdline_row;

    /*
     * Figure out the size and position of the pum.
     */
    if (size < CLPUM_DEF_HEIGHT)
	clpum_height = size;
    else
	clpum_height = CLPUM_DEF_HEIGHT;
    if (p_clph > 0 && clpum_height > p_clph)
	clpum_height = p_clph;

    /* pum above "row" */

    if (row >= size)
    {
	clpum_row = row - size;
	clpum_height = size;
    }
    else
    {
	clpum_row = 0;
	clpum_height = row;
    }
    if (p_clph > 0 && clpum_height > p_clph)
    {
	clpum_row += clpum_height - p_clph;
	clpum_height = p_clph;
    }

    /* don't display when we only have room for one line */
    if (clpum_height < 1 || (clpum_height == 1 && size > 1))
	return;

    /* Compute the width of the widest match and the widest extra. */
    for (i = 0; i < size; ++i)
    {
	w = vim_strsize(array[i].pum_text);
	if (max_width < w)
	    max_width = w;
	if (array[i].pum_kind != NULL)
	{
	    w = vim_strsize(array[i].pum_kind) + 1;
	    if (kind_width < w)
		kind_width = w;
	}
	if (array[i].pum_extra != NULL)
	{
	    w = vim_strsize(array[i].pum_extra) + 1;
	    if (extra_width < w)
		extra_width = w;
	}
    }
    clpum_base_width = max_width;
    clpum_kind_width = kind_width;

    col = disp_col;

    /* if there are more items than room we need a scrollbar */
    if (clpum_height < size)
    {
	clpum_scrollbar = 1;
	++max_width;
    }
    else
	clpum_scrollbar = 0;

    if (def_width < max_width)
	def_width = max_width;

    if (col < Columns - p_clpw || col < Columns - max_width)
    {
	/* align pum column with "col" */
	clpum_col = col;

	clpum_width = Columns - clpum_col - clpum_scrollbar;

	if (clpum_width > max_width + kind_width + extra_width + 1
					    && clpum_width > p_clpw)
	{
	    clpum_width = max_width + kind_width + extra_width + 1;
	    if (clpum_width < p_clpw)
		clpum_width = p_clpw;
	}
    }
    else if (Columns < def_width)
    {
	/* not enough room, will use what we have */
	clpum_col = 0;
	clpum_width = Columns - 1;
    }
    else
    {
	if (max_width > p_clpw)
	    max_width = p_clpw;	/* truncate */
	clpum_col = Columns - max_width;
	clpum_width = max_width - clpum_scrollbar;
    }

    clpum_array = array;
    clpum_size = size;

    /* Set selected item and redraw.  If the window size changed need to redo
     * the positioning.  Limit this to two times, when there is not much
     * room the window size will keep changing. */
    if (clpum_set_selected(selected, redo_count) && ++redo_count <= 2)
	goto redo;
}

/*
 * Redraw the popup menu, using "clpum_first" and "clpum_selected".
 */
    void
clpum_redraw(void)
{
    int		row = clpum_row;
    int		col;
    int		attr_norm = highlight_attr[HLF_CLPNI];
    int		attr_select = highlight_attr[HLF_CLPSI];
    int		attr_scroll = highlight_attr[HLF_CLPSB];
    int		attr_thumb = highlight_attr[HLF_CLPST];
    int		attr;
    int		i;
    int		idx;
    char_u	*s;
    char_u	*p = NULL;
    int		totwidth, width, w;
    int		thumb_pos = 0;
    int		thumb_heigth = 1;
    int		round;
    int		n;

    /* Never display more than we have */
    if (clpum_first > clpum_size - clpum_height)
	clpum_first = clpum_size - clpum_height;

    if (clpum_scrollbar)
    {
	thumb_heigth = clpum_height * clpum_height / clpum_size;
	if (thumb_heigth == 0)
	    thumb_heigth = 1;
	thumb_pos = (clpum_first * (clpum_height - thumb_heigth)
			    + (clpum_size - clpum_height) / 2)
						/ (clpum_size - clpum_height);
    }

    for (i = 0; i < clpum_height; ++i)
    {
	idx = i + clpum_first;
	attr = (idx == clpum_selected) ? attr_select : attr_norm;

	/* prepend a space if there is room */
	if (clpum_col > 0)
	    screen_putchar(' ', row, clpum_col - 1, attr);

	/* Display each entry, use two spaces for a Tab.
	 * Do this 3 times: For the main text, kind and extra info */
	col = clpum_col;
	totwidth = 0;
	for (round = 1; round <= 3; ++round)
	{
	    width = 0;
	    s = NULL;
	    switch (round)
	    {
		case 1: p = clpum_array[idx].pum_text; break;
		case 2: p = clpum_array[idx].pum_kind; break;
		case 3: p = clpum_array[idx].pum_extra; break;
	    }
	    if (p != NULL)
		for ( ; ; MB_PTR_ADV(p))
		{
		    if (s == NULL)
			s = p;
		    w = ptr2cells(p);
		    if (*p == NUL || *p == TAB || totwidth + w > clpum_width)
		    {
			/* Display the text that fits or comes before a Tab.
			 * First convert it to printable characters. */
			char_u	*st;
			int	saved = *p;

			*p = NUL;
			st = transstr(s);
			*p = saved;
			if (st != NULL)
			{
			    screen_puts_len(st, (int)STRLEN(st), row, col,
								    attr);
			    vim_free(st);
			}
			col += width;

			if (*p != TAB)
			    break;

			/* Display two spaces for a Tab. */
			screen_puts_len((char_u *)"  ", 2, row, col, attr);
			col += 2;
			totwidth += 2;
			s = NULL;	    /* start text at next char */
			width = 0;
		    }
		    else
			width += w;
		}

	    if (round > 1)
		n = clpum_kind_width + 1;
	    else
		n = 1;

	    /* Stop when there is nothing more to display. */
	    if (round == 3
		    || (round == 2 && clpum_array[idx].pum_extra == NULL)
		    || (round == 1 && clpum_array[idx].pum_kind == NULL
					  && clpum_array[idx].pum_extra == NULL)
		    || clpum_base_width + n >= clpum_width)
		break;
	    screen_fill(row, row + 1, col, clpum_col + clpum_base_width + n,
							    ' ', ' ', attr);
	    col = clpum_col + clpum_base_width + n;
	    totwidth = clpum_base_width + n;
	}

	screen_fill(row, row + 1, col, clpum_col + clpum_width, ' ', ' ',
									attr);
	if (clpum_scrollbar > 0)
	{
	    screen_putchar(' ', row, clpum_col + clpum_width,
		    i >= thumb_pos && i < thumb_pos + thumb_heigth
						? attr_thumb : attr_scroll);
	}

	++row;
    }
}

/*
 * Set the index of the currently selected item.  The menu will scroll when
 * necessary.  When "n" is out of range don't scroll.
 * This may be repeated when the preview window is used:
 * "repeat" == 0: open preview window normally
 * "repeat" == 1: open preview window but don't set the size
 * "repeat" == 2: don't open preview window
 * Returns TRUE when the window was resized and the location of the popup menu
 * must be recomputed.
 */
    static int
clpum_set_selected(
    int	    n,
    int	    repeat UNUSED)
{
    int	    resized = FALSE;
    int	    context = clpum_height / 2;

    clpum_selected = n;

    if (clpum_selected >= 0 && clpum_selected < clpum_size)
    {
	if (clpum_first > clpum_selected - 4)
	{
	    /* scroll down; when we did a jump it's probably a PageUp then
	     * scroll a whole page */
	    if (clpum_first > clpum_selected - 2)
	    {
		clpum_first -= clpum_height - 2;
		if (clpum_first < 0)
		    clpum_first = 0;
		else if (clpum_first > clpum_selected)
		    clpum_first = clpum_selected;
	    }
	    else
		clpum_first = clpum_selected;
	}
	else if (clpum_first < clpum_selected - clpum_height + 5)
	{
	    /* scroll up; when we did a jump it's probably a PageDown then
	     * scroll a whole page */
	    if (clpum_first < clpum_selected - clpum_height + 1 + 2)
	    {
		clpum_first += clpum_height - 2;
		if (clpum_first < clpum_selected - clpum_height + 1)
		    clpum_first = clpum_selected - clpum_height + 1;
	    }
	    else
		clpum_first = clpum_selected - clpum_height + 1;
	}

	/* Give a few lines of context when possible. */
	if (context > 3)
	    context = 3;
	if (clpum_height > 2)
	{
	    if (clpum_first > clpum_selected - context)
	    {
		/* scroll down */
		clpum_first = clpum_selected - context;
		if (clpum_first < 0)
		    clpum_first = 0;
	    }
	    else if (clpum_first < clpum_selected + context - clpum_height + 1)
	    {
		/* scroll up */
		clpum_first = clpum_selected + context - clpum_height + 1;
	    }
	}
    }

    if (!resized)
	clpum_redraw();

    return resized;
}

/*
 * Undisplay the popup menu (later).
 */
    void
clpum_undisplay(void)
{
    //FreeWild(clpum_size, clpum_array);
    clpum_array = NULL;
    //clpum_size = -1;
    redraw_all_later(NOT_VALID);
#ifdef FEAT_WINDOWS
    redraw_tabline = TRUE;
#endif
    status_redraw_all();
}

/*
 * Clear the popup menu.  Currently only resets the offset to the first
 * displayed item.
 */
    void
clpum_clear(void)
{
    clpum_first = 0;
}

/*
 * Return TRUE if the popup menu is displayed.
 * Overruled when "clpum_do_redraw" is set, used to redraw the status lines.
 */
    int
clpum_visible(void)
{
    return !clpum_do_redraw && clpum_array != NULL;
}

/*
 * Return the height of the popup menu, the number of entries visible.
 * Only valid when clpum_visible() returns TRUE!
 */
    int
clpum_get_height(void)
{
    return clpum_height;
}
#endif
