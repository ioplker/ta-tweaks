-- Copyright 2016-2020 Gabriel Dubatti. See LICENSE.

local Util = Util
local events, events_connect = events, events.connect

local dialog_w= 800
local dialog_h= 600
local itemsgrp
local previewgrp1
local previewgrp2

local dialog_list= {}
local dialog_cols= {}
local dialog_buttons= {}
local dialog_data_icon= ""
local dialog_font_preview= false
local dialog_single_click= false

toolbar.dlg_select_it= ""
toolbar.dlg_select_ev= nil
toolbar.dlg_filter_col2= false

local filter= ""
local idx_filtered= {}
local ensure_it_vis= nil
local idx_sel_i= 0

local function get_list_itemstr(idx)
  local name= dialog_list[idx] --string list
  if type(name) == "table" then name= name[1] end --multi column list: use first column
  return name
end

local function get_list_col(idx, ncol)
  local item= dialog_list[idx]
  if type(item) == "table" and #item >= ncol then return item[ncol] end
  return ""
end

local function close_dialog()
  toolbar.popup(toolbar.DIALOG_POPUP,false) --hide dialog
end

local function close_dialog_ev(npop)
  if npop == toolbar.DIALOG_POPUP then close_dialog() end
end
events_connect("popup_close", close_dialog_ev)

local function update_preview()
  if dialog_font_preview and (idx_sel_i > 0) then
    local font= get_list_itemstr(idx_filtered[idx_sel_i])
    toolbar.sel_dialog_popup(previewgrp1,false)
    toolbar.textfont(24, 0, toolbar.cfg.textcolor_normal, toolbar.cfg.textcolor_grayed, toolbar.get_font_num(font))
    toolbar.sel_dialog_popup(previewgrp2,false)
    toolbar.textfont(12, 0, toolbar.cfg.textcolor_normal, toolbar.cfg.textcolor_grayed, toolbar.get_font_num(font))
  end
end

local function ensure_sel_view()
  if ensure_it_vis then
    toolbar.ensurevisible(ensure_it_vis, true)
    ensure_it_vis= nil
  end
end

local function change_selection(newsel)
  if newsel and idx_sel_i ~= newsel then
    if newsel < 1 then newsel=1 end
    if newsel > #idx_filtered then newsel= #idx_filtered end
    toolbar.selected("it#"..idx_filtered[idx_sel_i], false, false)
    idx_sel_i= newsel
    ensure_it_vis= "it#"..idx_filtered[idx_sel_i]
    toolbar.selected(ensure_it_vis, false, true)
    ensure_sel_view()
    update_preview()
    return true
  end
  return false
end

local function choose_item(cmd)
  local itnum= toolbar.getnum_cmd(cmd)
  if itnum then
    toolbar.dlg_select_it= get_list_itemstr(itnum)
    --ui.statusbar_text= "it selected: " .. toolbar.dlg_select_it
    if toolbar.dlg_select_ev then
      local keepopen= toolbar.dlg_select_ev(toolbar.dlg_select_it)
      if keepopen then return end --return true to keep the dialog open
    end
  end
  close_dialog()
end

local function click_item(cmd)
  if dialog_single_click then
    choose_item(cmd) --choose and close
  else
    --select item
    local id= toolbar.getnum_cmd(cmd)
    local n= 0
    for i=1, #idx_filtered do
      if idx_filtered[i]==id then n=i break end
    end
    change_selection(n)
  end
end

local function load_data()
  --remove all items
  toolbar.listtb_y= 1
  toolbar.listright= dialog_w-3
  toolbar.sel_dialog_popup(itemsgrp,true) --empty items group
  --load data
  local flt= string.lower(Util.escape_filter(filter))
  local n= 0
  idx_sel_i= 0
  idx_filtered= {}
  local i
  local icon= dialog_data_icon
  local isMime= (dialog_data_icon == "MIME")
  local x2= 0
  local w2= 0
  if dialog_cols and #dialog_cols > 1 then
    x2= dialog_cols[1]
    w2= dialog_cols[2]
  end
  for i=1, #dialog_list do
    local itstr= get_list_itemstr(i)
    local itname= string.lower(itstr)  --ignore case
    if (flt == '' or itname:match(flt)) and ((not toolbar.dlg_filter_col2) or (get_list_col(i,2) ~= '')) then --filter by name and col2
      n= n+1
      idx_filtered[n]= i
      local btname= "it#"..i
      if isMime then icon= toolbar.icon_fname(itstr) end
      toolbar.list_add_txt_ico(btname, itstr, "", false, click_item, icon, (n%2 ==1),  0, 0, 0, dialog_w-13)
      if w2 > 0 then
        txt2= get_list_col(i,2)
        if txt2 ~= "" then
          toolbar.gotopos(x2, toolbar.listtb_y - toolbar.cfg.butsize)
          toolbar.addlabel(txt2, "", w2, true) --left-align
        end
      end
      toolbar.cmd_dclick(btname, choose_item)
      if toolbar.dlg_select_it == "" then toolbar.dlg_select_it= itstr end --select first when none is provided
      if toolbar.dlg_select_it == itstr then idx_sel_i= n ensure_it_vis=btname toolbar.selected(ensure_it_vis, false, true) end
    end
  end
  if idx_sel_i == 0 and n > 0 then
    idx_sel_i= 1
    i= idx_filtered[idx_sel_i]
    toolbar.dlg_select_it= get_list_itemstr(i)
    ensure_it_vis="it#"..i
    toolbar.selected(ensure_it_vis, false, true)
  end
  update_preview()
end

local function update_filter()
  local ena= true
  local ftxt= filter
  if filter == "" then ena=false ftxt="Type to filter" end
  toolbar.settext("filter-txt", ftxt, "")
  toolbar.enable("filter-txt", ena)
  toolbar.enable("edit-find", ena)
end

local function filter_key(keycode)
  if keycode >= 32 and keycode <= 126 then --ascii
    filter= filter .. string.char(keycode)
  elseif keycode == toolbar.KEY.BACKSPACE then --remove last letter
    filter= filter:sub(1,-2)
  elseif keycode == toolbar.KEY.DELETE then --clear
    filter= ""
  end
  update_filter()
  load_data()
end

local function translate_keypad_codes(keycode)
  --convert keypad keycodes: *+-./0..9
  if keycode >= toolbar.KEY.KP_MULT and keycode <= toolbar.KEY.KP9 then return keycode-toolbar.KEY.KP_MULT+toolbar.KEY.MULT end
  return keycode
end

local function dialog_key_ev(npop, keycode)
  if npop == toolbar.DIALOG_POPUP then
    --ui.statusbar_text= "dialog key= ".. keycode
    keycode= translate_keypad_codes(keycode)
    if keycode == toolbar.KEY.RETURN or keycode == toolbar.KEY.KPRETURN then
      if idx_sel_i > 0 then choose_item("it#"..idx_filtered[idx_sel_i]) end --select and close
    elseif keycode == toolbar.KEY.UP or keycode == toolbar.KEY.LEFT then
      change_selection( idx_sel_i-1 )  --select previous item
    elseif keycode == toolbar.KEY.DOWN or keycode == toolbar.KEY.RIGHT then
      change_selection( idx_sel_i+1 )  --select next item
    elseif keycode == toolbar.KEY.PG_UP then
      change_selection( idx_sel_i-10 )  --select previous page item
    elseif keycode == toolbar.KEY.PG_DWN then
      change_selection( idx_sel_i+10 )  --select next page item
    elseif keycode == toolbar.KEY.HOME then
      change_selection( 1 )  --select previous page item
    elseif keycode == toolbar.KEY.END then
      change_selection( #idx_filtered )  --select next page item
    else
      filter_key(keycode) --modify filter
    end
    --return true to cancel default key actions (like close on ESCAPE)
  end
end
events_connect("popup_key", dialog_key_ev)

local function db_pressed(bname)
  for i=1, #dialog_buttons do
    local bt= dialog_buttons[i] --1:bname, 2:text, 3:tooltip, 4:x, 5:width, 6:row, 7:callback, 8:button-flags=toolbar.DLGBUT...
    if bt[1] == bname then
      local flg= bt[8]
      if (flg & toolbar.DLGBUT.CLOSE) ~= 0 then close_dialog() end  --close dialog
      if bt[7] ~= nil then bt[7](bname) end --callback
      if (flg & (toolbar.DLGBUT.CLOSE|toolbar.DLGBUT.RELOAD)) == toolbar.DLGBUT.RELOAD then load_data() end --reload-list
      break
    end
  end
end

function toolbar.create_dialog(title, width, height, datalist, dataicon, show_font_preview, singleclick, config)
  dialog_w= width
  dialog_h= height
  dialog_list= datalist
  if config then
    dialog_cols= config["columns"]
    dialog_buttons= config["buttons"]
  else
    dialog_cols= {}
    dialog_buttons= {}
  end
  dialog_data_icon= dataicon
  dialog_font_preview= show_font_preview
  dialog_single_click= singleclick

  filter= ""
  toolbar.new(50, 24, 16, toolbar.DIALOG_POPUP, toolbar.themepath,1)
  toolbar.setdefaulttextfont()
  toolbar.themed_icon(toolbar.globalicon, "ttb-combo-list", toolbar.TTBI_TB.BACKGROUND) --cfg-back
  toolbar.themed_icon(toolbar.globalicon, "ttb-button-hilight", toolbar.TTBI_TB.BUT_HILIGHT)
  toolbar.themed_icon(toolbar.globalicon, "ttb-button-press", toolbar.TTBI_TB.BUT_HIPRESSED)
  toolbar.themed_icon(toolbar.globalicon, "ttb-button-active", toolbar.TTBI_TB.BUT_SELECTED)
  toolbar.themed_icon(toolbar.globalicon, "ttb-button-normal-drop", toolbar.TTBI_TB.DDBUT_NORMAL)
  toolbar.themed_icon(toolbar.globalicon, "ttb-button-hilight-drop", toolbar.TTBI_TB.DDBUT_HILIGHT)
  toolbar.themed_icon(toolbar.globalicon, "ttb-button-press-drop", toolbar.TTBI_TB.DDBUT_HIPRESSED)
  toolbar.themed_icon(toolbar.globalicon, "ttb-button-active-drop", toolbar.TTBI_TB.DDBUT_SELECTED)
  toolbar.themed_icon(toolbar.globalicon, "group-vscroll-back", toolbar.TTBI_TB.VERTSCR_BACK)
  toolbar.themed_icon(toolbar.globalicon, "group-vscroll-bar", toolbar.TTBI_TB.VERTSCR_NORM)
  toolbar.themed_icon(toolbar.globalicon, "group-vscroll-bar-hilight", toolbar.TTBI_TB.VERTSCR_HILIGHT)
  toolbar.themed_icon(toolbar.globalicon, "cfg-separator-h", toolbar.TTBI_TB.HSEPARATOR)

  --title group: align top + fixed height
  toolbar.addgroup(toolbar.GRPC.ONLYME|toolbar.GRPC.EXPAND, 0, 0, toolbar.cfg.barsize, false)
  toolbar.setdefaulttextfont()
  toolbar.themed_icon(toolbar.groupicon, "cfg-back2", toolbar.TTBI_TB.BACKGROUND)
  toolbar.gotopos(2, 3)
  toolbar.addlabel(title, "", dialog_w-toolbar.cfg.butsize-10, true, true)  --left align, bold
  toolbar.listtb_y= 2
  toolbar.list_cmdright= 2
  toolbar.list_addbutton("window-close", "Close", close_dialog)

  if dialog_font_preview then
    local prevtxt= "0123456789-AaBbCcDdEdFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz~{}[]"
    previewgrp1= toolbar.addgroup(toolbar.GRPC.ONLYME|toolbar.GRPC.EXPAND, 0, 0, 25, false)
    toolbar.textfont(24, 0, toolbar.cfg.textcolor_normal, toolbar.cfg.textcolor_grayed)
    toolbar.addlabel(prevtxt, "", dialog_w-5, true)
    previewgrp2= toolbar.addgroup(toolbar.GRPC.ONLYME|toolbar.GRPC.EXPAND, 0, 0, 25, false)
    toolbar.textfont(12, 0, toolbar.cfg.textcolor_normal, toolbar.cfg.textcolor_grayed)
    toolbar.addlabel(prevtxt, "", dialog_w-5, true)
  end

  --filter group: full width + items height
  toolbar.addgroup(toolbar.GRPC.ONLYME|toolbar.GRPC.EXPAND, 0, 0, toolbar.cfg.barsize+3, false)
  toolbar.setdefaulttextfont()
  toolbar.themed_icon(toolbar.groupicon, "ttb-combo-list", toolbar.TTBI_TB.BACKGROUND)
  toolbar.gotopos(2, 3)
  toolbar.cmd("edit-find", nil, "")
  toolbar.gotopos(2+toolbar.cfg.butsize, 3)
  toolbar.addlabel("...", "", dialog_w-toolbar.cfg.butsize-10, true, false, "filter-txt")  --left align
  update_filter()

  if dialog_buttons and #dialog_buttons > 0 then
    local nrows= 1
    for i=1, #dialog_buttons do
      local nr= dialog_buttons[i][6]
      if nr > nrows then nrows= nr end
    end
    local buttons= toolbar.addgroup(toolbar.GRPC.ONLYME|toolbar.GRPC.EXPAND, 0, 0, toolbar.cfg.barsize * nrows +1, false)
    toolbar.themed_icon(toolbar.groupicon, "ttb-button-normal", toolbar.TTBI_TB.BUT_NORMAL)
    toolbar.setdefaulttextfont()
    toolbar.themed_icon(toolbar.groupicon, "cfg-back2", toolbar.TTBI_TB.BACKGROUND)
    local sw= toolbar.cfg.butsize
    for i=1, #dialog_buttons do
      local bt= dialog_buttons[i] --1:bname, 2:text, 3:tooltip, 4:x, 5:width, 6:row, 7:callback, 8:button-flags=toolbar.DLGBUT...
      toolbar.gotopos(bt[4], (bt[6]-1)*toolbar.cfg.barsize+2)
      toolbar.cfg.butsize= bt[5]
      local flg= bt[8]
      local leftalign= ((flg & toolbar.DLGBUT.LEFT) ~= 0)
      local boldtxt= ((flg & toolbar.DLGBUT.BOLD) ~= 0)
      local dropdown= ((flg & toolbar.DLGBUT.DROPDOWN) ~= 0)
      --text,func,tooltip,name,usebutsz,dropbt,leftalign,bold
      toolbar.cmdtext(bt[2], db_pressed, bt[3], bt[1], true, dropdown, leftalign, boldtxt)
    end
    toolbar.cfg.butsize= sw
  end

  --items group: full width + items height w/scroll
  itemsgrp= toolbar.addgroup(toolbar.GRPC.ONLYME|toolbar.GRPC.EXPAND, toolbar.GRPC.LAST|toolbar.GRPC.ITEMSIZE|toolbar.GRPC.SHOW_V_SCROLL, 0, 0, false)
  toolbar.setdefaulttextfont()
  load_data()
end

function toolbar.font_chooser(title, sel_font, font_selected,btname,anchor)
  toolbar.dlg_select_it= sel_font
  toolbar.dlg_select_ev= font_selected
  toolbar.dlg_filter_col2= false
  toolbar.create_dialog(title or "Font chooser", 600, 331, toolbar.get_font_list(), "format-text-italic", true, false, nil) --show available fonts / font-preview / double-click= select and close
  if btname then
    toolbar.popup(toolbar.DIALOG_POPUP,true,btname,anchor,-dialog_w,-dialog_h) --anchor to a button (toolbar.ANCHOR)
  else
    toolbar.popup(toolbar.DIALOG_POPUP,true,300,300,-dialog_w,-dialog_h) --open at a fixed position
  end
  ensure_sel_view()
end
