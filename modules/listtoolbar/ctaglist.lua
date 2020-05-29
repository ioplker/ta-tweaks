-- Copyright 2016-2020 Gabriel Dubatti. See LICENSE.

if toolbar then
  local itemsgrp, firsttag
  local Proj = Proj
  local Util = Util
  local data= Proj.data

  local function list_clear()
    --remove all items
    toolbar.tag_list= {}
    toolbar.tag_listedfile= ""
    toolbar.tag_list_find= ""
    toolbar.listright= toolbar.listwidth-3
    toolbar.sel_left_bar(itemsgrp,true) --empty items group
    firsttag= nil
  end

  local function ctags_create()
    --items group: fixed width=300 / height=use buttons + vertical scroll
    itemsgrp= toolbar.addgroup(toolbar.GRPC.ONLYME|toolbar.GRPC.EXPAND, toolbar.GRPC.LAST|toolbar.GRPC.ITEMSIZE|toolbar.GRPC.SHOW_V_SCROLL, 0, 0, true)
    toolbar.sel_left_bar(itemsgrp)
    toolbar.textfont(toolbar.cfg.textfont_sz, toolbar.cfg.textfont_yoffset, toolbar.cfg.textcolor_normal, toolbar.cfg.textcolor_grayed)

    list_clear()

    if actions then actions.add("filter_ctaglist", 'Filter Ctag _List', toolbar.list_find_sym, "af6", "edit-find") end
  end

  local function gototag(cmd)
    Proj.getout_projview()
    local linenum= toolbar.getnum_cmd(cmd)
    Util.goto_line(buffer, linenum)
    buffer:vertical_centre_caret()
  end

  local function list_addtag(tagtext, line, ext_fields)
    --add an item to the list
    local bicon= "t_var"
    local extra
    if ext_fields:find('.-\t.+') then ext_fields,extra=ext_fields:match('(.-)\t(.+)') end
    if extra and extra:find('.-\t.+') then extra=extra:match('(.-)\t.+') end
    if ext_fields == "f" then tagtext= tagtext.." ( )" bicon="t_func"
    elseif ext_fields == "d" then bicon="t_def"
    elseif ext_fields == "t" then bicon="t_type"
    elseif ext_fields == "s" then tagtext= "struct "..tagtext bicon="t_struct"
    elseif ext_fields == "m" and extra then tagtext= extra.."."..tagtext bicon="t_struct" end

    local gotag= "gotag"..#toolbar.tag_list.."#"..line
    toolbar.tag_list[#toolbar.tag_list+1]= {gotag, tagtext, bicon}
  end

  local function filter_ctags()
    --show the tags that pass the filter
    local rowcol= toolbar.cfg.backcolor_erow
    firsttag= nil
    toolbar.sel_left_bar(itemsgrp,true) --empty items group
    toolbar.listtb_y= 3
    if #toolbar.tag_list == 0 then
      toolbar.list_addinfo('No CTAG entry found in this file')
    else
      local filter= Util.escape_filter(toolbar.tag_list_find)
      local n=0
      for i=1,#toolbar.tag_list do       --{gotag, tagtext, bicon}
        local tagtext= toolbar.tag_list[i][2]
        if filter == '' or tagtext:match(filter) then
          local gotag= toolbar.tag_list[i][1]
          local bicon= toolbar.tag_list[i][3]
          toolbar.list_add_txt_ico(gotag, tagtext, "", false, gototag, bicon, (i%2==1), 0, 0, 0)
          if not firsttag then firsttag= gotag end
          n= n+1
        end
      end
      if n == 0 then toolbar.list_addinfo('No CTAG entry match the filter') else toolbar.list_add_separator() end
    end
  end

  local function load_ctags()
    --ignore project views
    if Proj and Proj.isHiddenTabBuf(buffer) then return end
    list_clear()
    --title group
    toolbar.list_init_title() --add a resize handle
    local bname= buffer.filename
    if bname == nil then
      toolbar.list_addinfo('No filename',true)
      return
    end
    if Proj == nil then
      toolbar.list_addinfo('The project module is not installed')
      return
    end
    if data.filename == "" then
      toolbar.list_addinfo('No open project',true)
      return
    end
    local tag_files = {}
    if #data.proj_files > 0 then
      for row= 1, #data.proj_files do
        local ftype= data.proj_filestype[row]
        if ftype == Proj.PRJF_CTAG then
          tag_files[ #tag_files+1 ]= data.proj_files[row]
        end
      end
    end
    if #tag_files < 1 then
      toolbar.list_addinfo('No CTAG file found in the project')
      return
    end
    toolbar.tag_listedfile= bname
    local fname= bname:match('[^/\\]+$') -- filename only

    toolbar.list_addbutton("view-refresh", "Update Ctag List", toolbar.list_toolbar_reload)
    toolbar.list_addaction("filter_ctaglist")
    toolbar.list_addinfo(fname, true)
    for i = 1, #tag_files do
      local dir = tag_files[i]:match('^.+[/\\]')
      local f = io.open(tag_files[i])
      for line in f:lines() do
        local tag, file, linenum, ext_fields = line:match('^([_.%w]-)\t(.-)\t(.-);"\t?(.*)$')
        if tag and (file == bname) then --only show current file
          if not file:find('^%a?:?[/\\]') then file = dir..file end
          if linenum:find('^/') then linenum = linenum:match('^/^(.+)$/$') end
          if linenum then list_addtag(tag, linenum, ext_fields) end
        end
      end
      f:close()
    end
    filter_ctags()
  end

  function toolbar.list_find_sym()
    if not toolbar.list_tb then toolbar.list_toolbar_onoff() end
    toolbar.select_list("ctaglist",true) --activate this list
    local orgfind = toolbar.tag_list_find
    local word = ''
    r,word= ui.dialogs.inputbox{title = 'Tag search', width = 400, text = toolbar.tag_list_find}
    toolbar.tag_list_find= ''
    if r == 1 then toolbar.tag_list_find= word end
    if orgfind ~= toolbar.tag_list_find then --filter changed: update
      filter_ctags()
      if firsttag and toolbar.tag_list_find ~= '' then gototag(firsttag) end
    end
  end

  function toolbar.list_toolbar_reload()
    if toolbar.list_tb then
      local cmd
      --locate project RUN command that updates TAGS (ctags)
      if Proj then
        if #data.proj_filestype > 0 then
          for r=1, #data.proj_filestype do
            if data.proj_filestype[r] == Proj.PRJF_RUN then
              if data.proj_files[r]:match('ctags') then
                cmd= data.proj_files[r]
                break
              end
            end
          end
        end
      end
      if cmd then Proj.run_command(cmd) else load_ctags() end
    end
  end

  local function ctags_notify(switching)
    --when switching buffers/view: update only if the current buffer filename change
    if (not switch) or (toolbar.tag_listedfile ~= buffer.filename) then load_ctags() end
  end

  local function ctags_showlist(show)
    --show/hide list items
    toolbar.sel_left_bar(itemsgrp)
    toolbar.showgroup(show)
  end

  function toolbar.ctaglist_update() --the CTAG file was regenerated
    if toolbar.islistshown("ctaglist") then load_ctags() end
  end

  toolbar.registerlisttb("ctaglist", "Ctag List", "t_struct", ctags_create, ctags_notify, ctags_showlist)
end
