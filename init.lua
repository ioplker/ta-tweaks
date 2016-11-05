--if not CURSES then ui.set_theme('base16-tomorrow-dark') end
if not CURSES then ui.set_theme('ggg') end

TA_MAYOR_VER= tonumber(_RELEASE:match('^Textadept (.+)%..+$'))

-- Control+F4 = RESET textadept
keys.cf4 = reset

function my_goto_view(view)
  if TA_MAYOR_VER < 9 then
    ui.goto_view(view)
  else
    ui.goto_view(_VIEWS[view])
  end
end

--goto line= 0...
function my_goto_line(p_buffer,line)
  p_buffer:ensure_visible_enforce_policy(line)
  p_buffer:goto_line(line)
end

--https://foicica.com/wiki/export
export = require('export')
--export.browser = 'chromium-browser'

require('project')
require('goto_nearest')
require('ctrl_tab_mru')
require('quicktype')

textadept.file_types.extensions.mas = 'mas'
textadept.editing.comment_string.ansi_c = '//'

events.connect(events.LEXER_LOADED, function(lang)
  if lang == 'vala' then
    buffer.tab_width = 4
    buffer.use_tabs = false
  elseif lang == 'lua' or lang == 'text' then
    buffer.tab_width = 2
    buffer.use_tabs = false
  else
    buffer.tab_width = 2
    buffer.use_tabs = false
  end
  if toolbar then
    --show vertical toolbar only in html files
    toolbar.show_html_toolbar(lang)
  end
end)

if toolbar then
  require('toolbar')
  require('htmltoolbar')

  --read configuration file
  toolbar.load_config(true)
  local theme= toolbar.get_radio_val("tbtheme",3)
  --load toolbar theme from USERHOME
  if theme == 2 then
    toolbar.set_theme("bar-th-dark")
  elseif theme == 3 then
    toolbar.set_theme("bar-ch-dark")
  else
    toolbar.set_theme("bar-sm-light") --default
  end

  --change theme defaults here
  --toolbar.tabwithclose=true
  --toolbar.tabxmargin=1
  --toolbar.back[2]="ttb-back2-same"
  --toolbar.back[2]="ttb-back2-down"

  local tabpos= toolbar.get_radio_val("tbtabs",4) -1
  if tabpos < 0 then tabpos= 1 end
  --create the toolbar (tabpos, nvertcols, stbar)
  --tabpos=0: 1 row, use default tabs
  --tabpos=1: 1 row, tabs & buttons in the same line
  --tabpos=2: 2 rows, tabs at the top
  --tabpos=3: 2 rows, tabs at the bottom
  --nvertcols= 0..2 = number of columns in vertical toolbar
  --stbar=0: use default status bar
  --stbar=1: use toolbar's status bar
  toolbar.create(tabpos,1,1)

  --add some buttons
  if Proj then
    toolbar.cmd("tog-projview", Proj.toggle_projview,"Hide project [Shift+F4]", "ttb-proj-o")
    toolbar.addspace(4,true)
    toolbar.cmd("go-previous",  Proj.goto_prev_pos,  "Previous position [Shift+F11]")
    toolbar.cmd("go-next",      Proj.goto_next_pos,  "Next position [Shift+F12]")
    Proj.update_go_toolbar()
    toolbar.addspace()
  end

  toolbar.cmd("document-new",     Proj.new_file,   "New [Ctrl+N]")
  toolbar.cmd("document-save",    io.save_file,    "Save [Ctrl+S]")
  toolbar.cmd("document-save-as", io.save_file_as, "Save as [Ctrl+Shift+S]")
  toolbar.addspace()

  toolbar.cmd("tog-book", textadept.bookmarks.toggle, "Toggle bookmark [Ctrl+F2]", "gnome-app-install-star" )
  if Proj then
    toolbar.cmd("trimsp", Proj.trim_trailing_spaces, "Trim trailing spaces","dialog-ok")
  end

  --HTML quicktype toolbar
  toolbar.add_html_toolbar()

  --add a button to show/hide the config panel
  toolbar.add_showconfig_button()

  --toolbar ready, show it
  toolbar.ready()

  --create config panel
  toolbar.add_config_panel()
end
