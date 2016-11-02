function toolbar.toggle_showconfig()
  --toggle shown state
  local b="showconfig"
  if toolbar.config_toolbar_shown then
    toolbar.config_toolbar_shown= false
    toolbar.setthemeicon(b, "visualization")
    toolbar.settooltip(b, "Show configuration panel [F9]")
  else
    toolbar.config_toolbar_shown= true
    toolbar.setthemeicon(b, "ttb-proj-c")
    toolbar.settooltip(b, "Hide configuration panel [Esc]")
  end
  toolbar.sel_config_bar()
  toolbar.show(toolbar.config_toolbar_shown)
end

function toolbar.hide_config()
  if toolbar.config_toolbar_shown then
    toolbar.toggle_showconfig()
    return true
  end
  return false
end

--show config panel / next config tab
function toolbar.next_configtab()
  if not toolbar.config_toolbar_shown then
    toolbar.toggle_showconfig()
    return
  end
  toolbar.sel_config_bar()
  toolbar.gototab(1)
end

--show config panel / prev config tab
function toolbar.prev_configtab()
  if not toolbar.config_toolbar_shown then
    toolbar.toggle_showconfig()
    return
  end
  toolbar.sel_config_bar()
  toolbar.gototab(-1)
end

--add a button to show/hide the config panel
function toolbar.add_showconfig_button()
  --add tab group if pending
  toolbar.addpending()
  --add a group of buttons after tabs
  toolbar.addrightgroup()
  toolbar.cmd("showconfig", toolbar.toggle_showconfig, "Show configuration panel [F9]", "visualization")
end

function toolbar.config_tab_click(ntab)
  toolbar.sel_config_bar()
  toolbar.activatetab(ntab)
  toolbar.settext("cfgtit", toolbar.cfgpnl_tit[ntab], "", true)
  if toolbar.cfgpnl_group[ntab] > 0 then
    toolbar.sel_toolbar_n(3,toolbar.cfgpnl_group[toolbar.cfgpnl_curgroup])
    toolbar.showgroup(false)
    toolbar.sel_toolbar_n(3,toolbar.cfgpnl_group[ntab])
    toolbar.showgroup(true)
    toolbar.cfgpnl_curgroup= ntab
  end
end

--create the "vertical right (config)" panel
local function add_config_start(startgroup)
  toolbar.cfgpnl_tit={}
  toolbar.cfgpnl_group={}
  toolbar.cfgpnl_curgroup= startgroup
  toolbar.cfgpnl_chkval={}

  toolbar.new(toolbar.cfgpnl_width, 24, 16, 3, toolbar.themepath)
  toolbar.current_toolbar= 3
  toolbar.current_tb_group= 0
  toolbar.seticon("TOOLBAR", "ttb-cback", 0, true)  --vertical back

  --config title: width=expand / height=27
  toolbar.addgroup(7, 0, 0, 27)
  toolbar.seticon("GROUP", "ttb-cback2", 0, true)
  toolbar.textfont(toolbar.textfont_sz+4, toolbar.textfont_yoffset, toolbar.statcolor_normal, toolbar.statcolor_normal)
  toolbar.addtext("cfgtit", "", "", toolbar.cfgpnl_width) --group title (set later)
  toolbar.enable("cfgtit",false,true)

  toolbar.tabwithclose=false
  toolbar.tabwidthmode=0
  toolbar.tabwidthmin=0
  if toolbar.img[4]  == "" then toolbar.img[4]=  "ttb-tab-back" end
  if toolbar.img[7]  == "" then toolbar.img[7]=  "ttb-ntab3nc" end
  if toolbar.img[10] == "" then toolbar.img[10]= "ttb-dtab3nc" end
  if toolbar.img[13] == "" then toolbar.img[13]= "ttb-htab3nc" end
  if toolbar.img[16] == "" then toolbar.img[16]= "ttb-atab3nc" end
  for i, img in ipairs(toolbar.img) do
    if img ~= "" then toolbar.seticon("TOOLBAR", img, i, true) end
  end
  toolbar.add_tabs_here()

  toolbar.seticon("GROUP", toolbar.back[1], 0, true)  --horizontal back x 1row
end

local function add_config_tabgroup(name,title,ngrp)
  local n=#toolbar.cfgpnl_tit+1
  if ngrp == nil then ngrp=n+1 end
  toolbar.cfgpnl_tit[n]=title
  toolbar.cfgpnl_group[n]=ngrp
  toolbar.settab(n, name, "")
  --create a group for each tab to hide its controls
  local hidegrp=(n ~= toolbar.cfgpnl_curgroup) --only one tab group is visible at a time
  toolbar.addgroup(7,8,0,0,hidegrp)
  toolbar.adjust(48,24,2,1,3,3)
  toolbar.textfont(toolbar.textfont_sz, toolbar.textfont_yoffset, toolbar.statcolor_normal, toolbar.statcolor_normal)
  if n == toolbar.cfgpnl_curgroup then
    toolbar.settext("cfgtit", title, "", true)
    toolbar.activatetab(toolbar.cfgpnl_curgroup)
  end
  toolbar.cfgpnl_y= toolbar.cfgpnl_ymargin
  --toolbar.seticon("GROUP", "ttb-cback2", 0, true)
end

local function add_config_label(text,extrasep)
  if extrasep then
    --add extra separation (1/2 row)
    toolbar.cfgpnl_y= toolbar.cfgpnl_y + toolbar.cfgpnl_rheight/2
  end
  toolbar.gotopos(toolbar.cfgpnl_xmargin, toolbar.cfgpnl_y)
  toolbar.addlabel(text, "", toolbar.cfgpnl_width-toolbar.cfgpnl_xtext*2,true)
  toolbar.cfgpnl_y= toolbar.cfgpnl_y + toolbar.cfgpnl_rheight
end

local function set_check_val(name,checked)
  if checked then
    toolbar.cfgpnl_chkval[name]= true
    toolbar.setthemeicon(name, "check1")
  else
    toolbar.cfgpnl_chkval[name]= false
    toolbar.setthemeicon(name, "check0")
  end
end

local function get_check_val(name)
  if toolbar.cfgpnl_chkval[name] then
    return true
  end
  return false
end

local function check_clicked(name)
  --toggle checkbox value
  set_check_val(name, not get_check_val(name))
end

local function add_config_check(name,text,tooltip,val)
  if val == nil then val=false end
  --text
  toolbar.gotopos(toolbar.cfgpnl_xtext, toolbar.cfgpnl_y)
  toolbar.addlabel(text, tooltip, toolbar.cfgpnl_xcontrol-toolbar.cfgpnl_xtext,true)
  --checkbox
  toolbar.gotopos(toolbar.cfgpnl_xcontrol, toolbar.cfgpnl_y)
  toolbar.cmd(name, check_clicked, tooltip, (val and "check1" or "check0"))
  toolbar.setthemeicon(name, "check-hi", 2)
  toolbar.setthemeicon(name, "check-pr", 3)
  toolbar.cfgpnl_y= toolbar.cfgpnl_y + toolbar.cfgpnl_rheight
  toolbar.cfgpnl_chkval[name]=val
end

local function radio_clicked(name)
  --set new radio button value
  toolbar.cfgpnl_chkval[name]= true
  toolbar.setthemeicon(name, "radio1")
  --reset the others (same rname in "rname:option-value")
  local rname=string.match(name, "(.-):.*$")
  if rname then
    local i=1
    while toolbar.cfgpnl_chkval[rname..':'..i] ~= nil do
      local rbn=rname..':'..i
      if name ~= rbn and toolbar.cfgpnl_chkval[rbn] then
        toolbar.cfgpnl_chkval[rbn]= false
        toolbar.setthemeicon(rbn, "radio0")
      end
      i=i+1
    end
  end
end

local function set_radio_val(name,val)
  radio_clicked(name..":"..val)
end

local function get_radio_val(name)
  local i=1
  while toolbar.cfgpnl_chkval[name..':'..i] ~= nil do
    if toolbar.cfgpnl_chkval[name..':'..i] then
      return i
    end
    i=i+1
  end
  return 0
end

local function _add_config_radio(name,text,tooltip,checked)
  if checked == nil then checked=false end
  --text
  toolbar.gotopos(toolbar.cfgpnl_xtext, toolbar.cfgpnl_y)
  toolbar.addlabel(text, tooltip, toolbar.cfgpnl_xcontrol-toolbar.cfgpnl_xtext, true)
  --radio button
  toolbar.gotopos(toolbar.cfgpnl_xcontrol, toolbar.cfgpnl_y)
  toolbar.cmd(name, radio_clicked, tooltip, (checked and "radio1" or "radio0"))
  toolbar.setthemeicon(name, "radio-hi", 2)
  toolbar.setthemeicon(name, "radio-pr", 3)
  toolbar.cfgpnl_y= toolbar.cfgpnl_y + toolbar.cfgpnl_rheight
  toolbar.cfgpnl_chkval[name]=checked
end

--start a new radio button: name="rname:num" or "rname" (num=1)
local function add_config_radio(name,text,tooltip,checked)
  local rname,rnum= string.match(name, "(.-):(.*)$")
  if rname then
    toolbar.last_rname= rname
    toolbar.last_rnum= tonumber(rnum)
  else
    toolbar.last_rname= name
    toolbar.last_rnum= 1
    name= name..":1"
  end
  _add_config_radio(name,text,tooltip,checked)
end

local function cont_config_radio(text,tooltip,checked)
  toolbar.last_rnum= toolbar.last_rnum+1
  _add_config_radio(toolbar.last_rname..":"..toolbar.last_rnum,text,tooltip,checked)
end

function toolbar.add_config_panel()
  --create the "vertical right (config)" panel
  add_config_start(1) --start in tabgroup #1

  add_config_tabgroup("Buffer", "Buffer configuration")
    add_config_label("Some checks")
    add_config_check("chk_a", "Some option #1", "Check test 1", false)
    add_config_check("chk_b", "Some option #2", "Check test 2", true)
    add_config_check("chk_c", "Some option #3", "Check test 3", false)

    add_config_label("Some radios",true)
    add_config_radio("rad_a", "A radio option #1", "Radio test A1")
    cont_config_radio("A radio option #2", "Radio test A2")
    cont_config_radio("A radio option #3", "Radio test A3")
    set_radio_val("rad_a", 3)

    add_config_label("More radios",true)
    add_config_radio("rad_b", "B radio option #1", "Radio test B1", true)
    cont_config_radio("B radio option #2", "Radio test B2")

  add_config_tabgroup("View", "View configuration")
    add_config_label("More radios")
    add_config_radio("rad_c:1", "C radio option #1", "Radio test C1")
    add_config_radio("rad_c:2", "C radio option #2", "Radio test C2", true)

    add_config_label("Some checks",true)
    add_config_check("chk_d", "Some option #1", "Check test 1", true)

  add_config_tabgroup("Project", "Project configuration")
    add_config_label("to do 1")

  add_config_tabgroup("Editor", "Editor configuration")
    add_config_label("to do 2")

  add_config_tabgroup("Toolbar", "Toolbar configuration")
    add_config_label("to do 3")

  --hidden for now
  toolbar.show(false)
end

--------------------------------------------------------------
-- F9            show config panel / next config tab
-- SHIFT+F9      show config panel / prev config tab
keys['f9']= toolbar.next_configtab
keys['sf9']= toolbar.prev_configtab
