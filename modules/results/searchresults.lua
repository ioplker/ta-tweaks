-- Copyright 2016-2019 Gabriel Dubatti. See LICENSE.

if toolbar then
  local itemsgrp
  local selitem=0
  local nitems= 0
  local yout= 1
  local full_search= {} --copy all
  local file_search= {} --files array
  local pos_search= {}  --{num_file,line-num} array

  local function search_create()
    itemsgrp= toolbar.addgroup(toolbar.GRPC.ONLYME|toolbar.GRPC.EXPAND, toolbar.GRPC.LAST|toolbar.GRPC.ITEMSIZE|toolbar.GRPC.SHOW_V_SCROLL, 0, 0, true)
    toolbar.sel_results_bar(itemsgrp)
    toolbar.textfont(toolbar.cfg.textfont_sz, toolbar.cfg.textfont_yoffset, toolbar.cfg.textcolor_normal, toolbar.cfg.textcolor_grayed)
  end

  local function get_rowname(n)
    return "sch-item#"..n
  end

  local function ensurevisible()
    if nitems > 0 then toolbar.ensurevisible(get_rowname(nitems)) end
    if selitem > 0 then toolbar.ensurevisible(get_rowname(selitem)) end
  end

  local function search_notify(switching)
    ensurevisible()
  end

  local function search_showlist(show)
    --show/hide list items
    toolbar.sel_results_bar(itemsgrp)
    toolbar.showgroup(show)
  end

  function toolbar.results_clear()
    --remove all items
    toolbar.sel_results_bar(itemsgrp,true) --empty items group
    nitems= 0
    yout= 1
    full_search= {}
    file_search= {}
    pos_search= {}
    selitem=0
  end

  --"edit-clear" / "edit-copy"
  local function search_act(name)
    if name == "edit-clear" then toolbar.results_clear() end
    if name == "edit-copy"  then buffer:copy_text(table.concat(full_search,'\n')) end
  end

  local function select_searchrow(n)
    --restore previously selected row
    toolbar.sel_results_bar(itemsgrp)
    if selitem > 0 then toolbar.setbackcolor(get_rowname(selitem), (selitem%2==1) and toolbar.cfg.backcolor_erow or -1,false,true) end
    selitem= n --highlight new
    toolbar.setbackcolor(get_rowname(n), toolbar.cfg.backcolor_hi,false,true)
  end

  local function search_click(name) --click= select row
    select_searchrow(toolbar.getnum_cmd(name))
  end

  function toolbar.search_result(txt, toolt, bold, icon)
    nitems= nitems+1
    toolbar.sel_results_bar(itemsgrp)
    local name= get_rowname(nitems)
    toolbar.listtb_y= yout
    if #txt > 2000 then txt= txt:sub(1,2000).."..." end
    local oneline= Util.str_one_line(txt)
    if #oneline > 200 then oneline= oneline:sub(1,200).."..." end
    local tt= (toolt ~= nil) and toolt or txt
    full_search[#full_search+1]= tt
    toolbar.list_add_txt_ico(name, oneline, tt, bold, search_click, icon, false, 0, 0, 0)
    yout= yout + toolbar.cfg.butsize
    toolbar.showresults("searchresults")
    return name
  end

  function toolbar.search_result_start(s_txt)
    local name= toolbar.search_result("["..s_txt.."]", nil, true)
    toolbar.ensurevisible(name)
    select_searchrow(nitems)
  end
  function toolbar.search_result_filter(s_filter)
    toolbar.search_result(' search dir '..s_filter, nil, false, nil)
  end
  function toolbar.search_result_error(s_err)
    toolbar.search_result(s_err, nil, true, "package-broken")
  end
  function toolbar.search_result_file(s_f,fname)
    toolbar.search_result(s_f, fname, true, toolbar.icon_fname(fname))
    if #file_search == 0 or file_search[#file_search] ~= fname then file_search[#file_search+1]= fname end
    pos_search[nitems]= {#file_search, 0} --open file
  end
  function toolbar.search_result_found(fname,nlin,txt)
    toolbar.search_result("  @"..('%4d'):format(nlin)..": "..Util.str_trim(txt), nil, false, nil)
    if fname and (#file_search == 0 or file_search[#file_search] ~= fname) then file_search[#file_search+1]= fname end
    if nlin > 0 then pos_search[nitems]= {#file_search, nlin} end
  end
  function toolbar.search_result_end()
    toolbar.sel_results_bar(itemsgrp)
    toolbar.listtb_y= yout
    toolbar.list_add_separator()
    yout= toolbar.listtb_y
    ensurevisible()
  end

  local function search_dclick(name) --double click= goto file
    local nr= toolbar.getnum_cmd(name)
    local pos= pos_search[nr]
    if pos then
      Proj.go_file(file_search[pos[1]], pos[2]) --goto file / linenum
    else
      local s= full_search[nr] --copy the searched text
      buffer:copy_text(s:sub(2,#s-1)) --remove "[]"
    end
  end

  toolbar.registerresultstb("searchresults", "Search results", "system-search", search_create, search_notify, search_showlist, search_act)
  toolbar.cmd_dclick("sch-item",search_dclick)
end
