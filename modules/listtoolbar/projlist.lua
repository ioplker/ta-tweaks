-- Copyright 2016-2019 Gabriel Dubatti. See LICENSE.

if toolbar then
  local itemsgrp, itselected, currproj, projmod, first_row
  local collarow= {}
  local Proj = Proj
  local data= Proj.data

  --right-click context menu
  local proj_context_menu = {
    {--"open_projsel",
     "open_projectdir",SEPARATOR,
     "toggle_editproj","toggle_viewproj","copyfilename",SEPARATOR,
     "adddirfiles_proj",SEPARATOR,
     --"show_documentation",
     "search_project"
     --,"search_sel_dir","search_sel_file"
    }
  }

  local function clear_selected()
    if itselected then
      toolbar.selected(itselected,false,false)
      itselected= nil
    end
  end

  local function sel_file(cmd) --click= select
    local rmexp= string.match(cmd,"exp%-(.*)")
    if rmexp then cmd=rmexp end
    local linenum= toolbar.getnum_cmd(cmd)
    if linenum then
      clear_selected()
      itselected= cmd
      toolbar.selected(cmd,false,true)
      toolbar.ensurevisible(cmd)
    end
    return linenum
  end

  local function sel_file_num(linenum)
    if not linenum then linenum=first_row end
    if linenum > #data.proj_files then linenum=#data.proj_files end
    if linenum > 0 then sel_file("gofile#"..linenum) end
  end

  local function gofile_rclick(cmd) --right click
    if sel_file(cmd) then
      ui.toolbar_context_menu= create_uimenu_fromactions(proj_context_menu)
      return true --open context menu
    end
  end

  local function gofile_dclick(cmd) --double click
    local linenum= sel_file(cmd)
    if linenum then
      local fn= data.proj_files[linenum]
      local ft= data.proj_filestype[linenum]
      if ft == Proj.PRJF_FILE or ft == Proj.PRJF_CTAG then
        Proj.go_file(fn)
      elseif ft == Proj.PRJF_RUN then
        Proj.run_command(fn)
      else
        local name= "exp-"..cmd
        local r= collarow[name]
        if r ~= nil then if r then expand_list(name) else collapse_list(name) end end
      end
    end
  end

  local function list_clear()
    --remove all items
    toolbar.listright= toolbar.listwidth-3
    toolbar.sel_left_bar(itemsgrp,true) --empty items group
    collarow= {}
  end

  local function currproj_change()
    if not Proj.data.proj_parsed then return false end  --wait until the project is parsed
    local prjfn= Proj.data.filename
    if prjfn == "" then
      if currproj then
        currproj= nil
        projmod= nil
        return true --closed: changed
      end
      return false
    end
    local modt= lfs.attributes(prjfn, 'modification')
    if prjfn == currproj then --same file, check modification time
      if modt == projmod then return false end --SAME
    else
      clear_selected()
    end
    currproj= prjfn
    projmod= modt
    return true --new or modified
  end

  local function set_expand_icon(cmd,icon)
    toolbar.setthemeicon(cmd, icon, toolbar.TTBI_TB.IT_NORMAL)
    toolbar.setthemeicon(cmd, icon.."-hilight", toolbar.TTBI_TB.IT_HILIGHT)
    toolbar.setthemeicon(cmd, icon.."-hilight", toolbar.TTBI_TB.IT_HIPRESSED)
  end

  function expand_list(cmd)
    sel_file(cmd)
    set_expand_icon(cmd,"list-colapse2")
    toolbar.cmds_n[cmd]= collapse_list
    toolbar.collapse(cmd, false)
    collarow[cmd]= false
  end

  function collapse_list(cmd)
    sel_file(cmd)
    set_expand_icon(cmd,"list-expand2")
    toolbar.cmds_n[cmd]= expand_list
    toolbar.collapse(cmd, true)
    collarow[cmd]= true
  end

  local function load_proj()
    changed= currproj_change()
    toolbar.list_init_title() --add a resize handle
    toolbar.list_addaction("close_project")
    if (not Proj) then
      toolbar.list_addinfo('The Project module is not installed')
      return
    end
    if data.filename == "" then
      toolbar.list_addinfo('No open project', true)
      list_clear()
      return
    end
    if #data.proj_files < 1 then
      toolbar.list_addinfo('The project is empty', true)
      list_clear()
      return
    end

    first_row= 1
    local fname= data.proj_rowinfo[first_row][1]
    if fname == "" then fname= 'Project' else first_row=2 end
    toolbar.list_addinfo(fname, true)

    if not changed then return end

    local linenum= toolbar.getnum_cmd(itselected)
    list_clear()

    toolbar.sel_left_bar(itemsgrp)
    if first_row <= #data.proj_files then
      local y= 3
      local rowh= toolbar.cfg.butsize-2
      for i=first_row, #data.proj_files do
        local fname= data.proj_rowinfo[i][1]
        if fname ~= "" then
          local idlen= data.proj_rowinfo[i][3] --indent-len
          local ind= (data.proj_rowinfo[i][2] or 0) * 12 --indentation
          if idlen > 0 then ind= ind + 10 end
          local bicon= nil
          local tip= data.proj_files[i]
          local ft= data.proj_filestype[i]
          if ft == Proj.PRJF_FILE then
            bicon= toolbar.icon_fname(data.proj_files[i])
          elseif ft == Proj.PRJF_CTAG then
            bicon= "t_type"
            tip= "CTAG: "..tip
          elseif ft == Proj.PRJF_RUN then
            bicon= "lpi-bug"
            tip= "RUN: "..tip
          elseif ft == Proj.PRJF_VCS then
            bicon= "package-install"
            tip= Proj.get_vcs_info(i, "\n")
          end
          local name= "gofile#"..i
          toolbar.gotopos(3, y)
          local bold= false
          local xtxt= ind
          if bicon then xtxt= toolbar.cfg.barsize+ind else bold=true end
          toolbar.addtext(name, fname, tip, toolbar.listwidth-13, false, true, bold, xtxt, 0)
          toolbar.anchor(name, 10, true)
          toolbar.gotopos(3+ind, y)
          if bicon then
            local icbut= "ico-"..name
            toolbar.cmd(icbut, sel_file, "", bicon, true)
            toolbar.enable(icbut,false,false) --non-selectable image
          end
          if idlen > 0 then
            toolbar.gotopos( ind-7, y)
            local exbut= "exp-"..name
            toolbar.cmd(exbut, collapse_list, "", "list-colapse2", true)
            set_expand_icon(exbut,"list-colapse2")
            toolbar.collapse(exbut, false, rowh*idlen, true)
            collarow[exbut]= false
          end
          toolbar.cmds_n[name]= sel_file
          y= y + rowh
        else
          toolbar.gotopos(3, y)
          toolbar.addspace()
          y= y + rowh/2
        end
      end
      --set project's default collapse items
      for i= #data.proj_fold_row, 1, -1 do
        collapse_list("exp-gofile#"..data.proj_fold_row[i] )
      end
      sel_file_num(linenum)
    end
  end

  local function track_file() --select the current buffer in the list
    if buffer._project_select == nil and buffer._type ~= Proj.PRJT_SEARCH then
      --normal file: restore current line default settings
      local file= buffer.filename
      if file ~= nil then
        local row= Proj.get_file_row(file)
        if row then sel_file_num(row) end
      end
    end
  end

  local function proj_create()
    --items group: fixed width=300 / height=use buttons + vertical scroll
    itemsgrp= toolbar.addgroup(toolbar.GRPC.ONLYME|toolbar.GRPC.EXPAND, toolbar.GRPC.LAST|toolbar.GRPC.ITEMSIZE|toolbar.GRPC.SHOW_V_SCROLL, 0, 0, true)
    toolbar.sel_left_bar(itemsgrp)
    toolbar.textfont(toolbar.cfg.textfont_sz, toolbar.cfg.textfont_yoffset, toolbar.cfg.textcolor_normal, toolbar.cfg.textcolor_grayed)

    list_clear()
    toolbar.cmd_rclick("gofile",gofile_rclick)
    toolbar.cmd_dclick("gofile",gofile_dclick)
  end

  local function proj_notify(switching)
    if not switching then load_proj() end
    track_file()
  end

  local function proj_showlist(show)
    --show/hide list items
    toolbar.sel_left_bar(itemsgrp)
    toolbar.showgroup(show)
  end

  toolbar.registerlisttb("projlist", "Project", "document-properties", proj_create, proj_notify, proj_showlist)
end
