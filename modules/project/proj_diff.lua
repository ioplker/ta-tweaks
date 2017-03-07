-- BASED ON: file_diff module, Copyright 2015-2017 Mitchell mitchell.att.foicica.com. See LICENSE.
--
local Proj = Proj
local Util = Util

local MARK_ADDITION = _SCINTILLA.next_marker_number()
local MARK_DELETION = _SCINTILLA.next_marker_number()
local MARK_MODIFICATION = _SCINTILLA.next_marker_number()
local INDIC_ADDITION = _SCINTILLA.next_indic_number()
local INDIC_DELETION = _SCINTILLA.next_indic_number()

local bit32_band = bit32.band

local vfp1= Proj.prefview[Proj.PRJV_FILES]
local vfp2= Proj.prefview[Proj.PRJV_FILES_2]
local compareon=false
local synchronizing = false

local function clear_buf_marks(b)
  if b then
    for _, mark in ipairs{MARK_ADDITION, MARK_DELETION, MARK_MODIFICATION} do
      b:marker_delete_all(mark)
    end
    for _, indic in ipairs{INDIC_ADDITION, INDIC_DELETION} do
      b.indicator_current = indic
      b:indicator_clear_range(0, b.length)
    end
    b:annotation_clear_all()
  end
end

local function clear_view_marks(nview)
  if #_VIEWS >= nview then
    clear_buf_marks(_VIEWS[nview].buffer)
  end
end

-- Clear markers, indicators, and placeholder lines.
-- Used when re-marking changes or finished diff'ing.
local function clear_marked_changes()
  clear_view_marks(vfp1)
  clear_view_marks(vfp2)
end

-- Stops diff'ing.
local function diff_stop()
  if compareon then
    compareon= false
    clear_marked_changes()
    ui.statusbar_text= "File compare: OFF"
  end
end

--check that the buffers in both view hasn't changed
local function check_comp_buffers()
  if compareon and #_VIEWS >= vfp2 then
    local b1= _VIEWS[vfp1].buffer
    local b2= _VIEWS[vfp2].buffer
    return b1 and b2 and b1._comparing and b2._comparing
  end
  return false
end

-- Synchronize the scroll and line position of the other buffer.
local function synchronize()
  local currview= _VIEWS[view]
  local otherview= vfp2
  if currview == vfp2 then otherview= vfp1 elseif currview ~= vfp1 then return end
  if check_comp_buffers() then
    synchronizing = true
    Proj.stop_update_ui(true)
    local line = buffer:line_from_position(buffer.current_pos)
    local visible_line = buffer:visible_from_doc_line(line)
    local first_visible_line = buffer.first_visible_line
    local x_offset = buffer.x_offset
    Util.goto_view(otherview)
    buffer:goto_line(buffer:doc_line_from_visible(visible_line))
    buffer.first_visible_line, buffer.x_offset = first_visible_line, x_offset
    Util.goto_view(currview)
    Proj.stop_update_ui(false)
    synchronizing = false
  end
end

-- Mark the differences between the two buffers.
local function mark_changes(goto_first)
  --if not check_comp_buffers() then return end --already checked
  clear_marked_changes() -- clear previous marks
  -- Perform the diff.
  local buffer1= _VIEWS[vfp1].buffer
  local buffer2= _VIEWS[vfp2].buffer
  filediff.setfile(1, buffer1:get_text()) --#1 = new version (left)
  filediff.setfile(2, buffer2:get_text()) --#2 = old version (right)

  local first, n1, n2 = 0, 0, 0
  -- Parse the diff, marking modified lines and changed text.
  local r= filediff.getdiff( 1, 1 )
  --enum lines that are only in buffer1
  if #r > 0 then first= r[1] end
  for i=1,#r,2 do
    n1= n1 + r[i+1]-r[i]+1
    for j=r[i],r[i+1] do
      buffer1:marker_add(j-1, MARK_ADDITION)
    end
  end
  --enum lines that are only in buffer2
  r= filediff.getdiff( 2, 1 )
  for i=1,#r,2 do
    n2= n2 + r[i+1]-r[i]+1
    for j=r[i],r[i+1] do
      buffer2:marker_add(j-1, MARK_DELETION)
    end
  end
  --enum modified lines
  r= filediff.getdiff( 1, 2 )
  if #r > 0 and (first == 0 or r[1]<first) then first= r[1] end
  n1= n1 + #r
  n2= n2 + #r
  for i=1,#r,2 do
    buffer1:marker_add(r[i]-1, MARK_MODIFICATION)
    buffer2:marker_add(r[i+1]-1, MARK_MODIFICATION)
  end

  --show the missing lines using annotations
  r= filediff.getdiff( 1, 3 ) --buffer#1, 3=get blank lines list
  if #r > 0 and (first == 0 or r[1]<first) then first= r[1] end
  for i=1,#r,2 do
    buffer1.annotation_text[r[i]-1] = string.rep('\n', r[i+1]-1)
  end
  --idem buffer #2
  r= filediff.getdiff( 2, 3 )--buffer#2, 3=get blank lines list
  for i=1,#r,2 do
    buffer2.annotation_text[r[i]-1] = string.rep('\n', r[i+1]-1)
  end

  --mark text changes
  r= filediff.getdiff( 1, 4 )
  for i=1,#r,3 do
    if r[i] == 1 then
      buffer1.indicator_current = INDIC_ADDITION
      buffer1:indicator_fill_range(r[i+1], r[i+2])
    else
      buffer2.indicator_current = INDIC_DELETION
      buffer2:indicator_fill_range(r[i+1], r[i+2])
    end
  end
  if goto_first and first > 0 then buffer1:goto_line(first-1) end
  synchronize()
  return {n1, n2}
end

---- TA EVENTS ----
--TA-EVENT: BUFFER_AFTER_SWITCH or VIEW_AFTER_SWITCH
--clear pending file-diff
function Proj.clear_pend_file_diff()
  if buffer._comparing and not compareon then
    clear_buf_marks(buffer)
    buffer._comparing=nil
  end
end

--TA-EVENT: BUFFER_DELETED
--Stop diff'ing when one of the buffer's being diff'ed is closed
function Proj.check_diff_stop()
  if not check_comp_buffers() then diff_stop() end
end

--TA-EVENT: UPDATE_UI
--Ensure the diff buffers are scrolled in sync
function Proj.EVupdate_ui(updated)
  if updated and not synchronizing and check_comp_buffers() then
    if bit32_band(updated, buffer.UPDATE_H_SCROLL + buffer.UPDATE_V_SCROLL + buffer.UPDATE_SELECTION) > 0 then
      synchronize()
    end
  end
end

--TA-EVENT: MODIFIED
-- Highlight differences as text is typed and deleted.
function Proj.EVmodified(modification_type)
  if not check_comp_buffers() then return end
  if bit32_band(modification_type, 0x01 + 0x02) > 0 then mark_changes() end
end

--TA-EVENT: VIEW_NEW
function Proj.EVview_new()
  local markers = {
    [MARK_ADDITION] = 'green', [MARK_DELETION] = 'red',
    [MARK_MODIFICATION] = 'yellow'
  }
  for mark, color in pairs(markers) do
    buffer:marker_define(mark, buffer.MARK_BACKGROUND)
    buffer.marker_back[mark] = buffer.property_int['color.'..color]
  end
  local indicators = {[INDIC_ADDITION] = 'green', [INDIC_DELETION] = 'red'}
  for indic, color in pairs(indicators) do
    buffer.indic_style[indic] = buffer.INDIC_FULLBOX
    buffer.indic_fore[indic] = buffer.property_int['color.'..color]
    buffer.indic_alpha[indic], buffer.indic_under[indic] = 255, true
  end
end

---- ACTIONS ----
--ACTION: toggle_filediff
-- Highlight differences between files in left (NEW) / right (OLD) panel
function Proj.diff_start()
  if not Proj then return end
  clear_marked_changes()
  if compareon then
    diff_stop()
    return
  end
  if #_VIEWS < vfp2 then
    ui.statusbar_text= "Can't compare, the right panel is closed"
    return
  end
  ui.statusbar_text= "File compare: ON"

  Proj.stop_update_ui(true)
  Util.goto_view(vfp2)
  buffer.annotation_visible= buffer.ANNOTATION_STANDARD
  buffer._comparing=true
  local fn2= buffer.filename and buffer.filename or 'right buffer'

  Util.goto_view(vfp1)
  buffer.annotation_visible= buffer.ANNOTATION_STANDARD
  buffer._comparing=true
  local fn1= buffer.filename and buffer.filename or 'left buffer'

  compareon= true
  local n= mark_changes(true) --goto first change in buffer1

  --activate/create search view
  Proj.goto_searchview()
  buffer:append_text('[File compare]\n')
  buffer:append_text('  '..n[1]..' lines changed in '..fn1..'\n')
  buffer:append_text('  '..n[2]..' lines changed in '..fn2..'\n')
  Util.goto_view(vfp1)
  Proj.stop_update_ui(false)

end
