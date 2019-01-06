-- Copyright 2016-2018 Gabriel Dubatti. See LICENSE.
-- Control+F4 = RESET textadept
keys.cf4 = reset

--require('log')

require('util')
if not CURSES then
  if Util.TA_MAYOR_VER < 10 then ui.set_theme('ggg') else
    for _, buff in ipairs(_BUFFERS) do buff:set_theme('ggg') end
  end
end

require('export')
require('project')
require('goto_nearest')
require('ctrl_tab_mru')
require('quicktype')

--cf5 "List commands in a new buffer"
--if actions then
--  actions.add("dump_cmds", "Dump commands", function() actions.select_command(true) end, "cf5")
--end

textadept.file_types.extensions.MAS = 'mas'
textadept.file_types.extensions.mas = 'mas'
textadept.file_types.extensions.inc = 'mas'
textadept.file_types.extensions.INC = 'mas'
textadept.editing.comment_string.ansi_c = '//'
textadept.editing.comment_string.asm = ';'

if toolbar then
  require('toolbar')
  --require('htmltoolbar')
  require('listtoolbar')
  --toolbar.listwidth= 250
  require('listtoolbar.recentprojlist')
  require('listtoolbar.projlist')
  require('listtoolbar.ctaglist')

  --set the configured theme
  toolbar.set_theme_from_config()

  --change theme defaults here
  --toolbar.set_backimg(2, "ttb-back2-same")

  --create the configured toolbars
  toolbar.create_from_config()

  --add some buttons
  --create list toolbar
  if toolbar.createlisttb then
    toolbar.createlisttb()
    toolbar.addaction("toggle_viewlist")
  end

  if Proj then
    toolbar.addaction("toggle_viewproj")
    toolbar.addspace(4,true)
    toolbar.addaction("prev_position")
    toolbar.addaction("next_position")
    toolbar.addspace()
    toolbar.addaction("new")
  end

  toolbar.addaction("save")
  toolbar.addaction("saveas")
  toolbar.addspace()

  toolbar.addaction("toggle_bookmark")

  toolbar.addaction("toggle_macrorec")
  toolbar.addaction("play_macrorec")

  if Proj then toolbar.addaction("trim_trailingspaces") end

--  local function showpopup()
--    toolbar.show_popup("window-new",33)
--  end
--  toolbar.cmd("window-new", showpopup, "TEST show popup")
--  toolbar.create_popup()

  -- MINI MAP (toolbar #4)
  if minimap then toolbar.minimap_setup() end

  --toolbars are ready to show
  toolbar.ready()

  --toolbar.new(50, 24, 16, toolbar.RESULTS_TOOLBAR, toolbar.themepath)
  --toolbar.themed_icon(toolbar.globalicon, "cfg-back", toolbar.TTBI_TB.BACKGROUND)
  --toolbar.show(true)
end

--reload changed files without prompting
events.connect(events.FILE_CHANGED, function()
  io.reload_file()
  return false
end,1)
