----------------------------------------------------------------------
-------- Project file format --------
-- Valid project lines:
-- 1)  [b] [n] '::' [fn] '::' [opt]
-- 2)  [b] [fn]
-- 3)  [b]
--
-- [b] = optional blanks to visualy group files (folding groups)
-- [n] = optional file/group name or description
--        '['n']', '('n')', '<'n'>' are color/bold hightlighted
-- [fn] = path/filename to open
--        'path\'    absolute path definition 'P'
--        'path/'    absolute path definition 'P'
--        '.\path\'  relative path definition 'p' (added to last absolute 'P')
--        './path/'  relative path definition 'p' (added to last absolute 'P')
--        'filename' open file 'fn' (absolute) or 'P'+'fn' (relative)
-- [opt] = optional control options
--        '-'     fold this group on project load
--
-- (P= 'first' previous 'P'/'p' or project path)
--  The first project line MUST BE an "option 1)"
----------------------------------------------------------------------
-- Vars added to buffer:
--  _project_select  = mark this buffer as a valid project file
--               nil = regular file
--              true = project in SELECTION mode
--             false = project in EDIT mode
--
--  _is_working_project = this is the working project (in case more than one is open)
--              true = this is the one
--
--  proj_files[]     = array with the filename in each row (1...) or ''
--  proj_fold_row[]  = array with the row numbers to fold on open
--  proj_grp_path[]  = array with the path of each group or nil
-----------------------------------------------------------------------
local Proj = Proj

--determines the buffer type: Proj.PRJT_...
function Proj.get_buffertype(p_buffer)
  if not p_buffer then p_buffer = buffer end  --use current buffer?
  
  if p_buffer._project_select ~= nil then  --marked as a project file?
    if p_buffer._is_working_project then
      if p_buffer._project_select then
        return Proj.PRJB_PROJ_SELECT  --is a project in "selection mode"
      end
      return Proj.PRJB_PROJ_EDIT      --is a project in "edit mode"
    end
    return Proj.PRJB_PROJ_IDLE        --is a project (but not the working one)
  end
  if p_buffer._type == Proj.PRJT_SEARCH then
    return Proj.PRJB_FSEARCH          --is a search results buffer
  end
  --check if the current file is a valid project
  --The first file line MUST BE a valid "option 1)": ...##...##...
  local line= buffer:get_line(0)
  local n, fn, opt = string.match(line,'^%s*(.-)%s*::(.*)::(.-)%s*$')
  if n ~= nil then
    return Proj.PRJB_PROJ_NEW         --is a project file not marked as such yet
  end
  return Proj.PRJB_NORMAL             --is a regular file
end

--return the project buffer (the working one)
--enforce: project in preferred view, mark it as the "working one"
function Proj.get_projectbuffer(force_view)
  -- search for the working project
  local pbuff, nview
  local prefv= Proj.prefview[Proj.PRJV_PROJECT] --preferred view
  for _, buffer in ipairs(_BUFFERS) do
    if buffer._is_working_project then
      --working project found
      if not force_view or _VIEWS[prefv].buffer == buffer then
        return buffer --ok (is in the right view)
      end
      --need to change the view
      pbuff = buffer
      break
    end
  end
  if pbuff == nil then
    -- not found, choose a new one
    -- 1) check the preferred project view
    if _VIEWS[prefv].buffer._project_select ~= nil then
      _VIEWS[prefv].buffer._is_working_project = true
      return _VIEWS[prefv].buffer --ok (marked and in the right view)
    end
    -- 2) check projects in all views
    for i= 1, #_VIEWS do
      if _VIEWS[i].buffer._project_select ~= nil then
        pbuff = _VIEWS[i].buffer
        nview = i
        break
      end
    end
    if pbuff == nil then
      -- 3) check all buffers, use the first found
      for _, buffer in ipairs(_BUFFERS) do
        if buffer._project_select ~= nil then
          pbuff = buffer
          break
        end
      end
    end
  end
  
  if pbuff then
    --force: marked as the working project
    pbuff._is_working_project = true
    if force_view then
      --force: project in the preferred view
      if nview == nil then
        --locate actual view
        for i= 1, #_VIEWS do
          if _VIEWS[i].buffer == pbuff then
            nview = i
            break
          end
        end
      end
      if nview ~= prefv then
        --show project in the preferred view
        local nv= _VIEWS[view]
        ui.goto_view(prefv)
        view:goto_buffer(_BUFFERS[pbuff])
        ui.goto_view(nv)
      end
    end
  end
  return pbuff
end


function Proj.splitfilename(strfilename)
  -- Returns the Path, Filename, and Extension as 3 values
  return string.match(strfilename, "(.-)([^\\/]-%.?([^%.\\/]*))$")
end

--fill filenames array "buffer.proj_files[]"
function Proj.parse_projectbuffer()
  ui.statusbar_text= 'Parsing project file...'

  buffer.proj_files= {}
  buffer.proj_fold_row = {}
  buffer.proj_grp_path = {}

  --get project file path (default)
  local projname= buffer.filename
  if projname ~= nil then
    abspath,fn,ext = Proj.splitfilename(projname)
  else
    --new project, use current dir
    projname= ''
    abspath= lfs.currentdir()
  end
  path = abspath

  --parse project file line by line
  for r = 1, buffer.line_count do
    fname= ''
    line= buffer:get_line(r-1)

    --try option 1)
    local n, fn, opt = string.match(line,'^%s*(.-)%s*::(.*)::(.-)%s*$')
    if n == nil then
      --try option 2)
      fn= string.match(line,'^%s*(.-)%s*$')
    end
    --ui._print('Parser', 'n='..((n==nil) and 'nil' or n)..' f='..((f==nil) and 'nil' or f)..' opt='..((opt==nil) and 'nil' or opt) )

    if fn ~= nil and fn ~= '' then
      p,f,e= Proj.splitfilename(fn)
      if f == '' and p ~= '' then
        --only the path is given
        dots, pathrest= string.match(p,'^(%.*[\\/])(.*)$')
        if dots == '.\\' or dots == './' then
          --relative path (only one dot is supported by now)
          path= abspath .. pathrest
        else
          --absolute path
          abspath = p
          path = abspath
        end
        buffer.proj_grp_path[r]= path

      elseif f ~= '' then
        if p == '' then
          --relative file, add current path
          fname= path .. fn
        else
          --absolute file
          fname= fn
        end
      end
    end
    --set the filename asigned to each row
    buffer.proj_files[r]= fname
    if opt ~= nil and opt ~= '' then
      --TODO: improve this / add more control flags / goto-line
      if opt == '-' then
        --  '-': fold this group on project load
        buffer.proj_fold_row[ #buffer.proj_fold_row+1 ]= r
      end
    end
  end
  ui.statusbar_text= 'Project: '.. projname
end

--return the file position (ROW: 1..) in the given buffer file list
function Proj.locate_file(p_buffer, file)
  --check the given buffer has a list of files
  if p_buffer and p_buffer.proj_files ~= nil and file then
    for row= 1, #p_buffer.proj_files do
      if file == p_buffer.proj_files[row] then
        return row
      end
    end
  end
  --not found
  return nil
end

--------------------------------------------------------------
-- show project current row properties
function Proj.show_doc()
  --call_tip_show
  if buffer._project_select ~= nil then
    if buffer:call_tip_active() then events.emit(events.CALL_TIP_CLICK) return end
    if buffer.proj_files ~= nil then
      r= buffer.line_from_position(buffer.current_pos)+1
      info = buffer.proj_files[ r ]
      if info == '' and buffer.proj_grp_path[r] ~= nil then
        info= buffer.proj_grp_path[r]
      end
      if info ~= '' then
        buffer:call_tip_show(buffer.current_pos, info )
      end
    end
  else
    --call default show doc function
    textadept.editing.show_documentation()
  end
end

--------------------------------------------------------------
--activate/create search view
function Proj.goto_searchview()
  for _, sbuffer in ipairs(_BUFFERS) do
    if sbuffer._type == Proj.PRJT_SEARCH then
      --goto search results view
      local index = _BUFFERS[sbuffer]
      for i, view in ipairs(_VIEWS) do
        if view.buffer._type == Proj.PRJT_SEARCH then ui.goto_view(i) break end
      end
      if view.buffer._type ~= Proj.PRJT_SEARCH then view:goto_buffer(index) end
      return
    end
  end
  --split view to show search results
  view:split()
  --set default search height= 75% of screen (actual = 50%)
  view.size= math.floor(view.size*1.5)
  local search_buffer = buffer.new()
  search_buffer._type = Proj.PRJT_SEARCH
  events.emit(events.FILE_OPENED)
end

-- find text in project's files
-- code adapted from module: find.lua
function Proj.find_in_files(p_buffer,text,match_case,whole_word)
  --activate/create search view
  Proj.goto_searchview()
  Proj.search_vn= _VIEWS[view]

  buffer:append_text('['..text..']\n')
  buffer:goto_pos(buffer.length)
  buffer.indicator_current = ui.find.INDIC_FIND
  if whole_word then text = '%f[%w_]'..(match_case and text or text:lower())..'%f[^%w_]' end

  local nfiles= 0
  local totfiles= 0
  local nfound= 0
  --check the given buffer has a list of files
  if p_buffer and p_buffer.proj_files ~= nil then
    for row= 1, #p_buffer.proj_files do
      local file= p_buffer.proj_files[row]
      if file and file ~= '' then
        local line_num = 1
        totfiles = totfiles + 1        
        local prt_fname= true        
        for line in io.lines(file) do
          local s, e = (match_case and line or line:lower()):find(text)
          if s and e then
            file = file:iconv('UTF-8', _CHARSET)
            if prt_fname then
              prt_fname= false
              local p,f,e= Proj.splitfilename(file)
              if f == '' then
                f= file
              end
              buffer:append_text((' %s::%s::\n'):format(f, file))
              nfiles = nfiles + 1
              if nfiles == 1 then buffer:goto_pos(buffer.length) end
            end
            local snum= ('%4d'):format(line_num)
            buffer:append_text(('  @%s:%s\n'):format(snum, line))
            
            local pos = buffer:position_from_line(buffer.line_count - 2) + #snum + 4
            buffer:indicator_fill_range(pos + s - 1, e - s + 1)
            nfound = nfound + 1
          end
          line_num = line_num + 1
        end
      end
    end
  end
  
  if nfound == 0 then buffer:append_text(' '.._L['No results found']..'\n') end
  buffer:append_text('\n')
  buffer:set_save_point()
  
  ui.statusbar_text= ''..nfound..' matches found in '..nfiles..' of '..totfiles..' files'
  buffer:set_lexer('myproj')
end

--open the selected file
function Proj.open_search_file()
  --clear selection
  buffer.selection_start= buffer.selection_end
  --get line number, format: " @ nnn:....."
  local line_num = buffer:get_cur_line():match('^%s*@%s*(%d+):.+$')
  local file
  if line_num then 
    --get file name from previous lines
    for i = buffer:line_from_position(buffer.current_pos) - 1, 0, -1 do
      file = buffer:get_line(i):match('^.*::(.+)::.+$')
      if file then break end
    end    
  else
    --just open the file
    file= buffer:get_cur_line():match('^.*::(.+)::.+$')
  end
  if file then
    textadept.bookmarks.clear()
    textadept.bookmarks.toggle()
    --textadept.editing.select_line()
    --change to files view
    if Proj.files_vn ~= nil and Proj.files_vn ~= _VIEWS[view] and Proj.files_vn <= #_VIEWS then
      ui.goto_view(Proj.files_vn)
    end
    --goto file / line_num
    ui.goto_file(file:iconv(_CHARSET, 'UTF-8'),false,_VIEWS[Proj.files_vn])
    if line_num then textadept.editing.goto_line(line_num) end
  end
end

function Proj.close_search_view()
  if Proj.search_vn then
    --activate search view
    Proj.goto_searchview()
    Proj.search_vn = nil
    --close buffer / view
    view.unsplit(view)
    buffer:set_save_point()
    io.close_buffer()
    return true
  end
  return false
end

----------------------------------------
--snapopen project files based on io.snapopen @ file_io.lua 
function Proj.snapopen()
  local p_buffer = Proj.get_projectbuffer(true)
  if p_buffer == nil then
    ui.statusbar_text= 'No project found'
    return
  end
  if p_buffer.proj_files ~= nil then
    if Proj.files_vn ~= nil then
      ui.goto_view(Proj.files_vn)
    end
    local utf8_list = {}
    for row= 1, #p_buffer.proj_files do
      local file= p_buffer.proj_files[row]
      if file and file ~= '' then
        file = file:gsub('^%.[/\\]', ''):iconv('UTF-8', _CHARSET)
        utf8_list[#utf8_list + 1] = file        
      end
    end
    local options = {
      title = _L['Open'], columns = _L['File'], items = utf8_list,
      button1 = _L['_OK'], button2 = _L['_Cancel'], select_multiple = true,
      string_output = true, width = CURSES and ui.size[1] - 2 or nil
    }
    local button, files = ui.dialogs.filteredlist(options)
    if button ~= _L['_OK'] or not files then return end
    for i = 1, #files do files[i] = files[i]:iconv(_CHARSET, 'UTF-8') end
    io.open_file(files)
    Proj.update_after_switch()
  end
end