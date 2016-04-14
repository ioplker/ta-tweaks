------- PROJECT -------
-- Project vars:
--  Proj.cmenu_num  = number of the current context menu
--                1 = project in SELECTION mode
--                2 = project in EDIT mode
--                3 = regular file
--  Proj.cmenu_idx  = 'Project' submenu position in the context menu
--
--  Proj.updating_ui= number of ui updates in progress (ignore some events if > 0)
------------------------
Proj = {}

--buffer type
Proj.PRJB_NORMAL =      0   -- a regular file
Proj.PRJB_FSEARCH =     1   -- a "search in project files" results buffer
Proj.PRJB_PROJ_MIN =    2   -- start of project values
Proj.PRJB_PROJ_NEW =     2  -- a project file not marked as such yet
Proj.PRJB_PROJ_IDLE =    3  -- a project file (but not the working one)
Proj.PRJB_PROJ_SELECT =  4  -- a project file in "selection mode"
Proj.PRJB_PROJ_EDIT =    5  -- a project file in "edit mode"

--buffer "_type"
Proj.PRJT_SEARCH= '[Project search]'  --search results

--view type
Proj.PRJV_DEFAULT =     0   -- default view (no active project)
Proj.PRJV_PROJECT =     1   -- project view
Proj.PRJV_FILES =       2   -- project files view
Proj.PRJV_SEARCH =      3   -- search results view

--preferred view of each view type
Proj.prefview = {
  [Proj.PRJV_DEFAULT] = 0,  -- default view (no active project)
  [Proj.PRJV_PROJECT] = 1,  -- project in view #1
  [Proj.PRJV_FILES]   = 2,  -- project files in view #2  (view #1 size = 20%, VERTICAL)
  [Proj.PRJV_SEARCH]  = 3,  -- search results in view #1 (view #2 size = 80%, HORIZONTAL)
}
--split control { adjust previous view size [%], vertical split }
Proj.prefsplit = {
  [1] = { 0.20, true  },  -- project files in view #2  (view #1 size = 20%, VERTICAL)
  [2] = { 0.75, false },  -- search results in view #3 (view #2 size = 75%, HORIZONTAL)
}

--project file type
Proj.PRJF_EMPTY =       0   -- not a file (could be an empty row or a file group)
Proj.PRJF_PATH  =       1   -- a path
Proj.PRJF_FILE  =       2   -- a regular file (could be opened and searched)
Proj.PRJF_CTAG  =       3   -- a CTAGS file (could be opened but searched only using TAG functions)

require('proj_data')
require('proj_cmd')
require('proj_ui')
require('proj_ctags')
