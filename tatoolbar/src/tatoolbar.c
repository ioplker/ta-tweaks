/* TA toolbar */

//#define UNUSED(expr) do { (void)(expr); } while (0)

static void lL_showcontextmenu(lua_State *L, GdkEventButton *event, char *k);

//ntoolbar=0: HORIZONTAL
//ntoolbar=1: VERTICAL
//ntoolbar=2: HORIZONTAL
#define NTOOLBARS 3

//flags
#define TTBF_SELECTABLE     0x0001
#define TTBF_GRAYED         0x0002
#define TTBF_TABBAR         0x0004
#define TTBF_TAB            0x0008
#define TTBF_ACTIVE         0x0010
#define TTBF_SCROLL_BUT     0x0020
#define TTBF_CLOSETAB_BUT   0x0040
#define TTBF_CHANGED        0x0080
#define TTBF_HIDDEN         0x0100
#define TTBF_DRAGTAB        0x0200
#define TTBF_TEXT           0x0400

//node images
#define TTBI_NORMAL         0  //button/separator
#define TTBI_DISABLED       1  //button
#define TTBI_HILIGHT        2  //button/toolbar
#define TTBI_HIPRESSED      3  //button/toolbar
#define TTBI_NODE_N         4

//TTB images
#define TTBI_TB_BACKGROUND  0  //toolbar
#define TTBI_TB_SEPARATOR   1  //toolbar
#define TTBI_TB_HILIGHT     (TTBI_HILIGHT)     //button/toolbar
#define TTBI_TB_HIPRESSED   (TTBI_HIPRESSED)   //button/toolbar
#define TTBI_TB_TABBACK     4 //tab background
#define TTBI_TB_NTAB1       5  //normal tab1
#define TTBI_TB_NTAB2       6  //normal tab2
#define TTBI_TB_NTAB3       7  //normal tab3
#define TTBI_TB_DTAB1       8  //disabled tab1
#define TTBI_TB_DTAB2       9  //disabled tab2
#define TTBI_TB_DTAB3       10 //disabled tab3
#define TTBI_TB_HTAB1       11 //hilight tab1
#define TTBI_TB_HTAB2       12 //hilight tab2
#define TTBI_TB_HTAB3       13 //hilight tab3
#define TTBI_TB_ATAB1       14 //active tab1
#define TTBI_TB_ATAB2       15 //active tab2
#define TTBI_TB_ATAB3       16 //active tab3
#define TTBI_TB_TAB_NSL     17 //tab scroll left
#define TTBI_TB_TAB_NSR     18 //tab scroll right
#define TTBI_TB_TAB_HSL     19 //tab scroll left
#define TTBI_TB_TAB_HSR     20 //tab scroll right
#define TTBI_TB_TAB_NCLOSE  21 //normal close button
#define TTBI_TB_TAB_HCLOSE  22 //hilight close button
#define TTBI_TB_TAB_CHANGED 23 //changed indicator
#define TTBI_TB_TXT_HIL1    24 //hilight text button1
#define TTBI_TB_TXT_HIL2    25 //hilight text button2
#define TTBI_TB_TXT_HIL3    26 //hilight text button3
#define TTBI_TB_TXT_HPR1    27 //hi-pressed text button1
#define TTBI_TB_TXT_HPR2    28 //hi-pressed text button2
#define TTBI_TB_TXT_HPR3    29 //hi-pressed text button3
#define TTBI_TB_N           30

struct toolbar_img
{
  char * fname;
  int  width;
  int  height;
};

struct toolbar_node
{
  struct toolbar_node * next;
  int flags;          //TTBF_.. flags
  int num;            //number of a tab
  char * name;        //name of a button
  char * text;        //text shown (tab/text button)
  char * tooltip;     //tooltip text
  int barx1, bary1;
  int barx2, bary2;
  int imgx, imgy;
  int textwidth;      //text width in pixels
  int changewidth;    //0=use text width, >0=use this value in pixels, <0= % of toolbar.width
  int minwidth;       //min tab width
  int maxwidth;       //max tab width
  struct toolbar_img img[TTBI_NODE_N];
};

struct color3doubles
{
  double R;
  double G;
  double B;
};

static char xbutt_tooltip[128];
static struct toolbar_node xbutton;

struct toolbar_data
{
  GtkWidget *draw;    //(GtkWidget *drawing_area of this toolbar)
  int num;            //number of toolbar
  int isvertical;     //is a vertical toolbar (#1=yes)
  int isvisible;

  struct toolbar_node * list;
  struct toolbar_node * list_last;

  struct toolbar_node * tabs;
  struct toolbar_node * tabs_last;
  struct toolbar_node * tab_node;
  int ntabs;            //total number of tabs
  int ntabs_nothidden;  //number of tabs without HIDDEN flag
  int ntabs_expand;     //number of tabs that expand to use all the free space
  int ntabs_scroll;     //number of tabs not shown at the left = scroll tab support
  int islast_tab_shown;
  int try_scrollpack;   //after tab delete try to scroll left
  int xscleft, xscright;

  int barheight;
  int barwidth;
  int bwidth;
  int bheight;
  int xmargin;
  int ymargin;
  int xoff;
  int yoff;

  int xnew;
  int ynew;

  //text buttons
  int txtfontsz;  //font size in points (default = 12 points)
  int txttexth;   //font height
  int txttextoff; //font y offset
  int txttexty;   //font baseline y pos
  struct color3doubles txttextcolN; //normal
  struct color3doubles txttextcolG; //grayed

  //tabs
  int tabxmargin;
  int tabxsep; //< 0 == overlap
  int tabheight;
  int tabwidth; //total 'fixed' tab width without extra space
  int tabfixpart;   //T->img[TTBI_TB_NTAB1].width + T->img[TTBI_TB_NTAB3].width
  int closeintabs;
  int tabfontsz;  //font size in points (default = 10 points)
  int tabtexth;   //font height
  int tabtextoff; //font y offset
  int tabtexty;   //font baseline y pos
  int tabmodshow; //modified tab: 0:ignore 1:show icon 2:change text color
  struct color3doubles tabtextcolN; //normal
  struct color3doubles tabtextcolH; //hilight
  struct color3doubles tabtextcolA; //active
  struct color3doubles tabtextcolM; //modified
  struct color3doubles tabtextcolG; //grayed

  //tab defaults
  int tabchangewidth;    //0=use text width, >0=use this value in pixels, <0= % of toolbar.width
  int tabminwidth;       //min tab width
  int tabmaxwidth;       //max tab width

  struct toolbar_img img[TTBI_TB_N];

  int _tabs_x1,_tabs_x2,_tabs_y1,_tabs_y2;  //tab redraw area
};

static struct all_toolbars_data
{
  struct toolbar_data tbdata[NTOOLBARS]; //horizonal & vertical toolbars

  struct toolbar_node * philight;
  struct toolbar_node * phipress;
  int ntbhilight;     //number of the toolbar with the hilighted button or -1

  int currentntb;     //current toolbar num

  char * img_base;
} ttb;

static char * alloc_str( const char *s )
{ //alloc a copy of a string
  char *scopy= NULL;
  if( s != NULL ){
    scopy= malloc(strlen(s)+1);
    if( scopy != NULL ){
      strcpy( scopy, s);
    }
  }
  return scopy;
}

static char * chg_alloc_str( char *sold, const char *snew )
{ //change alloc string
  if( sold != NULL ){
    if( snew != NULL ){
      if( strcmp( sold, snew) == 0 ){
        //same string, keep the old one
        return sold;
      }
      if( strlen(sold) >= strlen(snew) ){
        //the newer is equal or shorter the old one, overwrite it
        strcpy( sold, snew );
        return sold;
      }
    }
    //delete old value
    free( (void *) sold);
  }
  //use new value
  return alloc_str(snew);
}


static char * alloc_img_str( const char *name )
{ //build + alloc image filename
  int n;
  char *img_file;
  char *scopy= NULL;
  if( name != NULL ){
    n= strlen(name);
    if( (n > 4) && ((strcmp(name+n-4, ".png") == 0)||(strcmp(name+n-4, ".PNG") == 0)) ){
      //contains ".png": use it
      scopy= alloc_str( name );
    }else{
      //build image name
      if( ttb.img_base == NULL ){ //no global image base, use default
        img_file= g_strconcat(textadept_home, "/core/images/bar/", name, ".png", NULL);
      }else{
        img_file= g_strconcat(ttb.img_base, name, ".png", NULL);
      }
      if( img_file != NULL ){
        scopy= alloc_str( img_file );
        g_free(img_file);
      }
    }
  }
  return scopy;
}

static double rgb2double(int color)
{
  return ((double)(color & 0x0FF))/255.0;
}

static void setrgbcolor(int rgb, struct color3doubles *pc)
{
  pc->R= rgb2double(rgb >> 16);
  pc->G= rgb2double(rgb >> 8);
  pc->B= rgb2double(rgb);
}

static int set_tb_img( struct toolbar_data *T, struct toolbar_node *p, int nimg, const char *imgname)
{ //set a button or toolbar image
  //return 1 if redraw is needed
  struct toolbar_img *pti;
  char * simg;
  int updatetabfix= 0;

  if( nimg < 0 ){
    return 0; //invalid image num
  }
  pti= NULL;
  if( p == NULL ){
    //toolbar image
    if( nimg < TTBI_TB_N ){
      pti= &(T->img[nimg]);
      if( (nimg == TTBI_TB_NTAB1)||(nimg == TTBI_TB_NTAB3)){
        updatetabfix= 1;
      }
    }
  }else{
    //button image
    if( nimg < TTBI_NODE_N ){
      pti= &(p->img[nimg]);
    }
  }
  if( pti == NULL ){
    return 0; //invalid image num
  }
  if( pti->fname != NULL ){
    if( (imgname != NULL) && (strcmp( pti->fname, imgname ) == 0) ){
      return 0; //same img, no redraw is needed
    }
    //free previous img
    free((void *)pti->fname);
    pti->fname= NULL;
    pti->width= 0;
    pti->height= 0;
  }
  if( imgname != NULL ){
    simg= alloc_img_str(imgname); //get img fname
    if( simg != NULL ){
      cairo_surface_t *cis= cairo_image_surface_create_from_png(simg);
      if( cis != NULL ){
        pti->width=  cairo_image_surface_get_width(cis);
        pti->height= cairo_image_surface_get_height(cis);
        if( (pti->width > 0) && (pti->height > 0)){
          pti->fname= simg;
          cairo_surface_destroy(cis);
          if( updatetabfix ){
            T->tabfixpart= T->img[TTBI_TB_NTAB1].width + T->img[TTBI_TB_NTAB3].width;
          }
          return 1; //image OK
        }
      }
      free( (void *) simg);
    }
    //image not found or invalid
  }
  return 1; //redraw
}

static void redraw_button( struct toolbar_data *T, struct toolbar_node * p )
{
  if( p != NULL ){
    if( (p->flags & (TTBF_TAB|TTBF_SCROLL_BUT|TTBF_CLOSETAB_BUT|TTBF_HIDDEN)) == 0 ){
      //redraw the area of one regular button
      gtk_widget_queue_draw_area(T->draw, p->barx1, p->bary1, p->barx2-p->barx1+1, p->bary2-p->bary1+1 ); //redraw
      return;
    }
    //redraw a tab or one of its buttons
  }
  //redraw the complete toolbar
  gtk_widget_queue_draw(T->draw);
}

static int is_dragtab_enabled( struct toolbar_data *T )
{
  if( T->tab_node != NULL ){
    if( (T->tab_node->flags & TTBF_DRAGTAB) != 0 ){
      return 1; //drag support enabled
    }
  }
  return 0;
}

static void redraw_tabs_beg( struct toolbar_data *T )
{
  if( T->tab_node != NULL ){
    T->_tabs_x1= T->tab_node->barx1;
    T->_tabs_x2= T->tab_node->barx2;
    T->_tabs_y1= T->tab_node->bary1;
    T->_tabs_y2= T->tab_node->bary2;
  }
}

static void redraw_tabs_end( struct toolbar_data *T )
{
  if( T->tab_node != NULL ){
    //union of before and after change size
    if( T->_tabs_x1 > T->tab_node->barx1 ){
      T->_tabs_x1= T->tab_node->barx1;
    }
    if( T->_tabs_x2 < T->tab_node->barx2 ){
      T->_tabs_x2= T->tab_node->barx2;
    }
    if( T->_tabs_y1 > T->tab_node->bary1 ){
      T->_tabs_y1= T->tab_node->bary1;
    }
    if( T->_tabs_y2 < T->tab_node->bary2 ){
      T->_tabs_y2= T->tab_node->bary2;
    }
    //redraw the tabs area
    gtk_widget_queue_draw_area(T->draw, T->_tabs_x1, T->_tabs_y1,
        T->_tabs_x2 - T->_tabs_x1 +1, T->_tabs_y2 - T->_tabs_y1 +1);
  }else{
    //redraw the complete toolbar
    gtk_widget_queue_draw(T->draw);
  }
}

static int set_text_bt_width(struct toolbar_data *T, struct toolbar_node * p )
{ //text button (text & no image)
  cairo_text_extents_t ext;
  cairo_t *cr;
  int diff= 0;
  if( p->text != NULL ){
    cr = gdk_cairo_create(T->draw->window);
    cairo_set_font_size(cr, T->txtfontsz);
    cairo_text_extents( cr, p->text, &ext );
    p->textwidth= (int) ext.width;
    if( T->txttexty < 0 ){
      cairo_text_extents( cr, "H", &ext );
      T->txttexth= (int) ext.height;
      //center text verticaly + offset
      T->txttexty=  ((T->img[TTBI_TB_TXT_HIL1].height+T->txttexth)/2)+T->txttextoff;
      if( T->txttexty < 0){
        T->txttexty= 0;
      }
    }
    cairo_destroy(cr);
    diff= p->barx2;
    p->imgx= p->barx1 + T->img[TTBI_TB_TXT_HIL1].width;
    p->imgy= p->bary1 + T->txttexty;
    p->barx2= p->imgx + p->textwidth + T->img[TTBI_TB_TXT_HIL3].width;
    if( p->barx2 < (p->barx1+p->minwidth)){
      //expand button (center text)
      p->imgx += (p->barx1 + p->minwidth - p->barx2)/2;
      p->barx2= p->barx1+p->minwidth;

    }else if( (p->maxwidth > 0) && (p->barx2 > (p->barx1+p->maxwidth)) ){
      //reduce button (trimm text)
      p->barx2= p->barx1+p->maxwidth;
    }
    diff= p->barx2 - diff;  //width diff
  }
  return diff;
}

static struct toolbar_node *add_ttb_node(struct toolbar_data *T, const char * name, const char * img, const char *tooltip, const char * text, int chwidth)
{
  int i;
  struct toolbar_node * p= (struct toolbar_node *) malloc( sizeof(struct toolbar_node));
  if( p != NULL){
    p->next= NULL;
    p->num= 0;
    p->name= alloc_str(name);
    p->text= alloc_str(text);
    p->tooltip= alloc_str(tooltip);
    p->flags= 0;
    if( (p->name != NULL) && (*(p->name) != 0) ){
      p->flags |= TTBF_SELECTABLE; //if a name is provided, it can be selected (it's a button)
    }
    p->barx1= T->xnew;
    p->bary1= T->ynew;
    p->imgx= T->xnew + T->xoff;
    p->imgy= T->ynew + T->yoff;
    p->barx2= T->xnew + T->bwidth;
    p->bary2= T->ynew + T->bheight;
    p->textwidth=   0;
    p->changewidth= chwidth;
    if( chwidth > 0 ){
      p->minwidth= chwidth;
      p->maxwidth= chwidth;
    }else{
      p->minwidth= 0;
      p->maxwidth= 0;
    }
    for(i= 0; (i < TTBI_NODE_N); i++){
      p->img[i].fname= NULL;
      p->img[i].width= 0;
      p->img[i].height= 0;
    }
    if( img != NULL ){
      set_tb_img( T, p, TTBI_NORMAL, img );

    }else if( p->text != NULL ){
      //text button (text & no image)
      set_text_bt_width(T, p);
      p->flags |= TTBF_TEXT;
    }
    if( T->isvertical ){
      T->ynew= p->bary2;
    }else{
      T->xnew= p->barx2;
    }

    //conect node to the end of the list
    if( T->list_last != NULL ){
      T->list_last->next= p;
    }else{
      T->list= p; //first
    }
    T->list_last= p;
  }
  return p;
}

static void clear_tabwidth(struct toolbar_data *T, struct toolbar_node * p)
{
  if( (p->flags & TTBF_HIDDEN) == 0 ){
    if( p->changewidth >= 0 ){
      T->tabwidth -= p->barx2;
    }else{
      T->tabwidth -= T->tabfixpart;
    }
  }
  p->barx2= 0;
}

static void set_tabwidth(struct toolbar_data *T, struct toolbar_node * p)
{
  if( (p->flags & TTBF_HIDDEN) == 0 ){
    if( p->changewidth >= 0 ){
      if( p->changewidth == 0 ){  //use text width
        //update textwidth if 0
        if( (p->textwidth == 0) && (p->text != NULL) && (*(p->text) != 0) && ((p->minwidth == 0)||(p->minwidth != p->maxwidth)) ){
          //if min and max are set and equal, there is no need to know the textwidth
      // NOTE: using variable width in status-bar fields 2..7 in "WIN32" breaks the UI!!
      // this fields are updated from the UPDATE-UI event and
      // calling gdk_cairo_create in this context (to get the text extension) freeze the UI for a second
      // and breaks the editor update mecanism (this works fine under LINUX, though)
      // so, fixed width is used for this fields
          cairo_text_extents_t ext;
          cairo_t *cr = gdk_cairo_create(T->draw->window);
          cairo_set_font_size(cr, T->tabfontsz);
          cairo_text_extents( cr, p->text, &ext );
          p->textwidth= (int) ext.width;
          cairo_destroy(cr);
        }
        p->barx2= p->textwidth + T->tabfixpart;
      }else{
        p->barx2= p->changewidth; //use this value
        if( p->barx2 < T->tabfixpart ){
          p->barx2= T->tabfixpart;
        }
      }
      if( p->barx2 < p->minwidth ){
        p->barx2= p->minwidth;
      }
      if( (p->barx2 > p->maxwidth) && (p->maxwidth > 0) ){
        p->barx2= p->maxwidth;
      }
      T->tabwidth += p->barx2;
    }else{
      //% of toolbar.width
      p->barx2= T->tabfixpart; //adjusted in update_tabs_size()
      T->tabwidth += T->tabfixpart;
    }
  }
}

static void update_tabs_size( struct toolbar_data *T )
{
  int n, nexpv, extrasp, remainsp, varw, porctot, xend;
  struct toolbar_node *p, *pte;

  if( T->tab_node != NULL ){
    //calc 'fixed' tabs end
    xend= T->tab_node->barx1 + T->tabxmargin + T->tabwidth;
    //add extra space between visible tabs
    if( (T->ntabs_nothidden > 1) && (T->tabxsep != 0) ){
      xend += (T->ntabs_nothidden-1) * T->tabxsep;
    }

    //split free space in tabs that expand
    nexpv= 0;
    if( (T->ntabs_expand > 0) && (T->barwidth > 0) ){
      T->ntabs_scroll= 0;
      T->islast_tab_shown= 1;
      extrasp= T->barwidth - xend; //extra space
      if( extrasp < 0 ){
        extrasp= 0; //no extra space
      }
      remainsp= extrasp;
      pte= NULL;
      porctot= 0;
      //first pass, count visible expand-tabs
      for( p= T->tabs; (p != NULL); p= p->next ){
        if( (p->changewidth < 0) && ((p->flags & TTBF_HIDDEN) == 0) ){
          if( nexpv == 0){
            pte= p; //the first one
          }
          nexpv++;
          porctot += p->changewidth;
        }
      }
      if( porctot == 0 ){
        porctot= -1;  //not needed, just in case... to prevent 0 div
      }
      //second pass, split the extra space
      for( p= pte, n= nexpv; (p != NULL); p= p->next, n-- ){
        if( (p->changewidth < 0) && ((p->flags & TTBF_HIDDEN) == 0) ){
          if( n == 1 ){
            //the last tab use all the remaining space
            remainsp -= T->tabxsep;
            if( remainsp < 0 ){
              remainsp= 0;
            }
            p->barx2= T->tabfixpart + remainsp;
            if( p->barx2 < p->minwidth ){
              p->barx2= p->minwidth;
            }
            if( (p->barx2 > p->maxwidth) && (p->maxwidth > 0) ){
              p->barx2= p->maxwidth;
            }
            break;
          }
          varw= ((extrasp * p->changewidth) / porctot) - T->tabxsep;
          if( varw < 0 ){
            varw= 0;
          }
          if( varw > remainsp ){
            varw= remainsp;
          }
          p->barx2= T->tabfixpart + varw;
          if( p->barx2 < p->minwidth ){
            p->barx2= p->minwidth;
          }
          if( (p->barx2 > p->maxwidth) && (p->maxwidth > 0) ){
            p->barx2= p->maxwidth;
          }
          remainsp -= p->barx2 - T->tabfixpart;
        }
      }
      if( nexpv > 0){
        //at least one expand-tab is visible
        //expand tab-node to use all the extra space
        if( xend < T->barwidth ){
          xend= T->barwidth;
        }
      }
    }
    //update tab-node size
    T->tab_node->barx2= xend;
  }
}

static struct toolbar_node *get_ttb_tab(struct toolbar_data *T, int ntab)
{
  struct toolbar_node * p;
  for( p= T->tabs; (p != NULL); p= p->next ){
    if( p->num == ntab){
      return p;
    }
  }
  return NULL;
}

static struct toolbar_node *add_ttb_tab(struct toolbar_data *T, int ntab)
{
  int i;
  struct toolbar_node * p= (struct toolbar_node *) malloc( sizeof(struct toolbar_node));
  if( p != NULL){
    p->next= NULL;
    p->name= NULL;
    p->text= NULL;
    p->tooltip= NULL;
    p->num= ntab;
    p->flags= TTBF_TAB | TTBF_SELECTABLE;

    p->changewidth= T->tabchangewidth;  //use TAB default
    if( p->changewidth < 0 ){
      T->ntabs_expand++;
    }
    p->minwidth=    T->tabminwidth;
    p->maxwidth=    T->tabmaxwidth;

    for(i= 0; (i < TTBI_NODE_N); i++){
      p->img[i].fname= NULL;
      p->img[i].width= 0;
      p->img[i].height= 0;
    }

    //conect node to the end of the list
    if( T->tabs_last != NULL ){
      T->tabs_last->next= p;
    }else{
      T->tabs= p; //first
    }
    T->tabs_last= p;
    T->ntabs++;
    T->ntabs_nothidden++;
  }
  return p;
}

static struct toolbar_node *set_ttb_tab(struct toolbar_data *T, int ntab, const char * text, const char *tooltip, int redraw)
{
  struct toolbar_node * p;
  void * vp;

  redraw_tabs_beg(T);
  p= get_ttb_tab(T, ntab);
  if( p == NULL ){  //not found, add at the end
    p= add_ttb_tab(T, ntab);
    if( p == NULL ){
      return NULL;
    }
  }else{
    //tab found, adjust total tab width without extra space
    clear_tabwidth(T, p);
  }
  //update texts
  p->text= chg_alloc_str(p->text, text);
  p->tooltip= chg_alloc_str(p->tooltip, tooltip);
  p->barx1= 0;
  p->bary1= 0;
  p->imgx=  T->img[TTBI_TB_NTAB1].width;	//text start
  p->bary2= T->img[TTBI_TB_NTAB1].height;
  p->imgy=  T->tabtexty;
  p->textwidth= 0;
  set_tabwidth(T, p);
  //split free space in tabs that expand
  update_tabs_size(T);
  if( redraw ){
    //queue a redraw using the post-modify size (in case the tabbar gets bigger)
    redraw_tabs_end(T);
  }
  return p;
}

int toolbar_set_statusbar_text(const char *text, int bar)
{ //called when textadept change the text in the status bar (bar 0 => tab#1  bar 1 => tab#2..7)
  char txt[64];
  const char *s;
  const char *d;
  unsigned n, ntab;

  struct toolbar_data *T= &ttb.tbdata[2];
  if( (text != NULL) && (T->isvisible) ){
    if( bar == 0 ){
      set_ttb_tab( T, 1, text, text, 1 ); //tooltip = text in case it can be shown complete
    }else{
      //split text in parts (separator= 4 spaces)
      ntab= 2;
      s= text;
      while( (*s != 0) && (ntab <= 7)){
        d= strstr( s, "    " );
        if( d != NULL ){
          n= d-s;
          if( n >= sizeof(txt) ){
            n= sizeof(txt)-1;
          }
          strncpy( txt, s, n );
          txt[n]= 0;
          set_ttb_tab( T, ntab, txt, "", 0 );
          s=d+4;
        }else{
          //last field
          set_ttb_tab( T, ntab, s, "", 0 );
          break;
        }
        ntab++;
      }
      //redraw the complete toolbar
      gtk_widget_queue_draw(T->draw);
    }
    return 0;
  }
  return 1; //update the regular status bar
}

static void clear_tooltip_text( struct toolbar_data *T )
{
  gtk_widget_set_tooltip_text(T->draw, "");
}

static void set_hilight_tooltip( struct toolbar_data *T )
{ //update tooltip text
  char *tooltip= "";
  if( (ttb.philight != NULL) && (ttb.ntbhilight == T->num) ){
    if(ttb.philight->tooltip != NULL){
      tooltip= ttb.philight->tooltip;
    }
  }
  gtk_widget_set_tooltip_text(T->draw, tooltip);
}

static void set_hilight_off( void )
{ //force hilight off (in any toolbar)
  struct toolbar_node * p;
  if( (ttb.philight != NULL) && (ttb.ntbhilight >= 0) ){
    p= ttb.philight;
    ttb.philight= NULL;
    ttb.phipress= NULL;
    redraw_button( &(ttb.tbdata[ttb.ntbhilight]), p );
    ttb.ntbhilight= -1;
  }
}

static void activate_ttb_tab(struct toolbar_data *T, int ntab)
{
  struct toolbar_node *p, *t, *vistab;
  int x, nhide, tabpos, n;

  redraw_tabs_beg(T);
  t= NULL;
  tabpos= 0;
  n= 0;
  for( p= T->tabs; (p != NULL); p= p->next ){
    if( p->num == ntab){
      p->flags |= TTBF_ACTIVE | TTBF_SELECTABLE;
      p->flags &= ~TTBF_GRAYED;
      t= p;
      tabpos= n;
    }else{
      p->flags &= ~TTBF_ACTIVE; //only one tab can be active
    }
    n++;
  }
  //check tab visibility (ignore this tab if hidden)
  if((t != NULL) && (T->tab_node != NULL) && (T->barwidth > 0) && ((t->flags & TTBF_HIDDEN) == 0)){
    x= T->tab_node->barx1 + T->tabxmargin;
    for( p= T->tabs, nhide= T->ntabs_scroll; (nhide > 0)&&(p != NULL); nhide-- ){
      if( p->num == ntab ){
        //the tab is left-hidden,
        //force "no scroll" to move this tab to the rightmost position
        if( ttb.ntbhilight == T->num ){
          set_hilight_off(); //force hilight off (in this toolbar only)
        }
        clear_tooltip_text(T);
        T->ntabs_scroll= 0; //
        p= T->tabs;
        break;
      }
      p= p->next; //skip hidden tabs
    }
    vistab= p;    //first visible tab
    if( t != NULL ){
      if( ((p->num == ntab) || (T->try_scrollpack)) && (T->ntabs_scroll > 0) ){
        //is the first visible tab or a tab was deleted: try to remove the left scroll button
        T->ntabs_scroll= 0; //force "no scroll" to move this tab to the rightmost position
        p= T->tabs;
        vistab= p;    //first visible tab
      }
      T->try_scrollpack= 0; //clear pack flag

      for( ; (p != NULL); p= p->next ){
        if( (p->flags & TTBF_HIDDEN) == 0 ){
          if( x > T->barwidth ){
            break;  //the rest of the tabs are right-hidden
          }
          if( p->num == ntab ){
            //check if it's completely visible
            if( x+t->barx2 <= T->barwidth ){
              t= NULL;  //visible, nothing to do
            }
            break;  //some part of the tab is hidden
          }
          x += p->barx2 + T->tabxsep;
        }
      }
    }
    if( t != NULL ){
      //at least a part of the tab is right-hidden
      for( ; (p != NULL); p= p->next ){
        if( (p->flags & TTBF_HIDDEN) == 0 ){
          x += p->barx2;
          if( p->num == ntab ){
            break;
          }
          x += T->tabxsep;
        }
      }
      //hide some tabs until the tab is completely visible
      while( (vistab != NULL) && (x > T->barwidth) ){
        if( (vistab->flags & TTBF_HIDDEN) == 0 ){
          x -= vistab->barx2 + T->tabxsep;
        }
        vistab= vistab->next;
        T->ntabs_scroll++;
      }
      if( vistab == NULL ){
        //not enought space to be completely visible
        //set as the first visible tab
        T->ntabs_scroll= tabpos;
      }
      if( ttb.ntbhilight == T->num ){
          set_hilight_off(); //force hilight off (in this toolbar only)
      }
      clear_tooltip_text(T);
    }
  }
  redraw_tabs_end(T);
}

static void enable_ttb_tab(struct toolbar_data *T, int ntab, int enable)
{
  redraw_tabs_beg(T);
  struct toolbar_node * p= get_ttb_tab(T, ntab);
  if( p != NULL ){
    if( enable ){
      p->flags &= ~TTBF_GRAYED;
      p->flags |= TTBF_SELECTABLE;
    }else{
      p->flags |= TTBF_GRAYED;
      p->flags &= ~(TTBF_ACTIVE | TTBF_SELECTABLE);
    }
  }
  redraw_tabs_end(T);
}

static void hide_ttb_tab(struct toolbar_data *T, int ntab, int hide)
{
  redraw_tabs_beg(T);
  struct toolbar_node * p= get_ttb_tab(T, ntab);
  if( p != NULL ){
    if( hide ){
      if( ((p->flags & TTBF_HIDDEN) == 0) ){
        clear_tabwidth(T, p);
        p->flags |= TTBF_HIDDEN;
        T->ntabs_nothidden--;
        //split free space in tabs that expand
        update_tabs_size(T);
      }
    }else{
      if( (p->flags & TTBF_HIDDEN) != 0 ){
        p->flags &= ~TTBF_HIDDEN;
        T->ntabs_nothidden++;
        set_tabwidth(T, p);
        //split free space in tabs that expand
        update_tabs_size(T);
      }
    }
  }
  redraw_tabs_end(T);
}

static void change_ttb_tabwidth(struct toolbar_data *T, int ntab, int percwidth, int minwidth, int maxwidth )
{
  struct toolbar_node * p= get_ttb_tab(T, ntab);
  if( p != NULL ){
    if( p->changewidth < 0 ){
      T->ntabs_expand--;
    }
    clear_tabwidth(T, p);
    //set new change width min, max and mode: 0:text width, >0:fixes, <0:porcent
    p->changewidth= percwidth;
    if( percwidth > 0 ){
      //fixed width, ignore minimum and maximum
      p->minwidth= 0;
      p->maxwidth= 0;
    }else{
      //variable width
      p->minwidth= minwidth;
      p->maxwidth= maxwidth;
      if( percwidth < 0 ){
        T->ntabs_expand++;
      }
    }
    set_tabwidth(T, p);
    //split free space in tabs that expand
    update_tabs_size(T);
    //redraw the complete toolbar
    gtk_widget_queue_draw(T->draw);
  }else if( ntab <= 0 ){
    //invalid tab, change toolbar defaults
    T->tabchangewidth= percwidth;
    if( percwidth > 0 ){
      //fixed width, ignore minimum and maximum
      T->tabminwidth= 0;
      T->tabmaxwidth= 0;
    }else{
      T->tabminwidth= minwidth;
      T->tabmaxwidth= maxwidth;
    }
  }
}

static void set_changed_ttb_tab(struct toolbar_data *T, int ntab, int changed)
{
  redraw_tabs_beg(T);
  struct toolbar_node * p= get_ttb_tab(T, ntab);
  if( p != NULL ){
    if( changed ){
      p->flags |= TTBF_CHANGED;
    }else{
      p->flags &= ~TTBF_CHANGED;
    }
  }
  redraw_tabs_end(T);
}

static void kill_toolbar_node( struct toolbar_node * p )
{
  int i;
  if(p->name != NULL){
    free((void*)p->name);
  }
  if(p->text != NULL){
    free((void*)p->text);
  }
  if(p->tooltip != NULL){
    free((void*)p->tooltip);
  }
  for(i= 0; (i < TTBI_NODE_N); i++){
    if( p->img[i].fname != NULL ){
      free((void*)p->img[i].fname);
    }
  }
  free((void*)p);
}

static void kill_toolbar_list( struct toolbar_node * list )
{
  struct toolbar_node * p;
  while(list != NULL){
    p= list;
    list= list->next;
    kill_toolbar_node(p);
  }
}

static void kill_toolbar_num( int num )
{ //kill one toolbar data
  int i;
  struct toolbar_data *T;

  if( (num >= 0) && (num < NTOOLBARS) ){
    T= &(ttb.tbdata[num]);
    kill_toolbar_list(T->list);
    T->list= NULL;
    T->list_last= NULL;

    kill_toolbar_list(T->tabs);
    T->tabs= NULL;
    T->tabs_last= NULL;
    T->tab_node= NULL;
    T->ntabs= 0;
    T->ntabs_nothidden= 0;
    T->ntabs_expand= 0;
    T->ntabs_scroll= 0;
    T->islast_tab_shown= 1;
    T->try_scrollpack= 0;
    T->xscleft= -1;
    T->xscright= -1;
    T->tabwidth= 0;
    T->tabxmargin= 0;
    T->tabxsep= 0;
    T->closeintabs= 0;
    T->tabfontsz= 10;  //font size in points (default = 10 points)
    T->tabtextoff= 0;
    T->tabtexty= 0;
    T->tabtexth= 0;
    setrgbcolor( 0x000000, &T->tabtextcolN);
    setrgbcolor( 0x000000, &T->tabtextcolH);
    setrgbcolor( 0x000000, &T->tabtextcolA);
    setrgbcolor( 0x000000, &T->tabtextcolM);
    setrgbcolor( 0x808080, &T->tabtextcolG);
    T->tabchangewidth= 0;  //use TAB default
    T->tabminwidth= 0;
    T->tabmaxwidth= 0;

    for(i= 0; (i < TTBI_TB_N); i++){
      if( T->img[i].fname != NULL ){
        free((void*)T->img[i].fname);
        T->img[i].fname= NULL;
        T->img[i].width= 0;
        T->img[i].height= 0;
      }
    }
    if(ttb.ntbhilight == num){
      ttb.philight= NULL;
      ttb.phipress= NULL;
      ttb.ntbhilight= -1;
    }
    T->txtfontsz= 12;  //font size in points (default = 12 points)
    T->txttexth= 0;
    T->txttextoff= 0;
    T->txttexty= -1;
    setrgbcolor( 0x000000, &T->txttextcolN);
    setrgbcolor( 0x808080, &T->txttextcolG);
  }
}

static void kill_tatoolbar( void )
{ //kill all toolbars data
  int nt;
  for( nt= 0; nt < NTOOLBARS; nt++ ){
    kill_toolbar_num(nt);
  }
  if( ttb.img_base != NULL ){ //global image base
    free((void *)ttb.img_base);
    ttb.img_base= NULL;
  }
  ttb.philight= NULL;
  ttb.phipress= NULL;
  ttb.ntbhilight= -1;
  ttb.currentntb= 0;
}

static struct toolbar_node * getButtonFromXY(struct toolbar_data *T, int x, int y)
{
  struct toolbar_node * p;
  int nx, nhide, xc1, xc2, yc1, yc2;
  char *s;

  for( p= T->list, nx=0; (p != NULL); p= p->next, nx++ ){
    //ignore non selectable things (like separators)
    if( ((p->flags & TTBF_SELECTABLE)!=0) && (x >= p->barx1) && (x <= p->barx2) && (y >= p->bary1) && (y <= p->bary2) ){
      if( (p->flags & TTBF_TABBAR) != 0){
        //is a tabbar, locate tab node
        if((T->xscleft >= 0)&&(x >= T->xscleft)&&(x <= T->xscleft+T->img[TTBI_TB_TAB_NSL].width+1)){
          xbutton.flags= TTBF_SCROLL_BUT;
          xbutton.num= -1;
          xbutton.tooltip= NULL;
          return &xbutton; //scroll left button
        }
        if((T->xscright >= 0)&&(x >= T->xscright)&&(x <= T->xscright+T->img[TTBI_TB_TAB_NSR].width+1)){
          xbutton.flags= TTBF_SCROLL_BUT;
          xbutton.num= 1;
          xbutton.tooltip= NULL;
          return &xbutton; //scroll right button
        }
        x -= p->barx1 + T->tabxmargin;
        y -= p->bary1;
        for( p= T->tabs, nhide= T->ntabs_scroll; (nhide > 0)&&(p != NULL); nhide-- ){
          p= p->next; //skip hidden tabs (scroll support)
        }
        for( ; (x >= 0)&&(p != NULL); p= p->next ){
          if( (p->flags & TTBF_HIDDEN) == 0 ){  //ignore hidden tabs
            if( ((p->flags & TTBF_SELECTABLE)!=0) && (x <= p->barx2) ){
              if( T->closeintabs ){
                //over close tab button?
                xc1= p->barx2-T->img[TTBI_TB_NTAB3].width;
                xc2= xc1+T->img[TTBI_TB_TAB_NCLOSE].width;
                yc2= T->img[TTBI_TB_TAB_NCLOSE].height;
                yc1= yc2-T->img[TTBI_TB_TAB_NCLOSE].width; //square close area
                if( yc1 < 0){
                  yc1= 0;
                }
                if( (x >= xc1)&&(x <= xc2)&&(y >= yc1)&&(y <= yc2) ){
                  xbutton.flags= TTBF_CLOSETAB_BUT;
                  xbutton.num= p->num;
                  xbutton.tooltip= xbutt_tooltip;
                  strcpy( xbutt_tooltip, "Close " );
                  s= p->tooltip;
                  if( s == NULL ){
                    s= p->text;
                  }
                  if( s != NULL ){
                    strncpy( xbutt_tooltip+6, s, sizeof(xbutt_tooltip)-7 );
                    xbutt_tooltip[sizeof(xbutt_tooltip)-1]= 0;
                  }else{
                    strcpy( xbutt_tooltip+6, "tab" );
                  }
                  return &xbutton; //close tab button
                }
              }
              return p; //TAB
            }
            x -= p->barx2 + T->tabxsep;
          }
        }
        return NULL;
      }
      return p; //BUTTON
    }
  }
  return NULL;
}

static struct toolbar_node * getButtonFromName(struct toolbar_data *T, const char *name)
{
  struct toolbar_node * p;
  if( name != NULL ){
    for( p= T->list; (p != NULL); p= p->next ){
      if( (p->name != NULL) && (strcmp(p->name, name) == 0) ){
        return p;
      }
    }
  }
  return NULL;  //invalid
}

static void ttb_change_button_img(struct toolbar_data *T, const char *name, int nimg, const char *img )
{
  struct toolbar_node * p= getButtonFromName(T, name);
  if( (p != NULL) || (strcmp(name, "TOOLBAR") == 0) ){
    if( set_tb_img( T, p, nimg, img ) ){
      redraw_button(T, p); //redraw button / toolbar (p==NULL)
    }
  }
}

static void ttb_change_button_tooltip(struct toolbar_data *T, const char *name, const char *tooltip )
{
  struct toolbar_node * p= getButtonFromName(T, name);
  if( p != NULL ){
    p->tooltip= chg_alloc_str(p->tooltip, tooltip);
    redraw_button(T, p); //redraw button
  }
}

static void ttb_change_button_text(struct toolbar_data *T, const char *name, const char *text )
{
  int dif;
  struct toolbar_node * p= getButtonFromName(T, name);
  if( p != NULL ){
    p->text= chg_alloc_str(p->text, text);
    dif= set_text_bt_width(T, p);
    if( dif != 0){
      //button width changed, update all buttons to the right
      p= p->next;
      while( p != NULL ){
        p->barx1 += dif;
        if( (p->flags & TTBF_TABBAR) == 0){
          p->barx2 += dif;
          p->imgx += dif;
        }
        p= p->next;
      }
      //split free space in tabs that expand
      update_tabs_size(T);
      //redraw the complete toolbar
      gtk_widget_queue_draw(T->draw);
    }else{
      //same width
      redraw_button(T, p); //redraw button
    }
  }
}

static void ttb_enable_button(struct toolbar_data *T, const char * name, int isenabled )
{
  int flg;
  struct toolbar_node * p= getButtonFromName(T, name);
  if( p != NULL){
    flg= p->flags;
    if( isenabled ){
      p->flags= (flg & ~TTBF_GRAYED) | TTBF_SELECTABLE;
    }else{
      p->flags= (flg & ~TTBF_SELECTABLE) | TTBF_GRAYED;
    }
    if( flg != p->flags ){
      redraw_button(T, p); //redraw button
    }
  }
}

static void scroll_tabs(struct toolbar_data *T, int x, int y, int dir )
{ //change the number of tabs not shown at the left (ignore hidden tabs)
  struct toolbar_node *t;
  int n, nt, nh;
  int nhide= T->ntabs_scroll;
  if((dir < 0)&&(T->ntabs_scroll > 0)){
    nh= 0;
    nt= 0;
    for( t= T->tabs, n= T->ntabs_scroll-1; (n > 0)&&(t != NULL); n-- ){
      nt++;
      if( (t->flags & TTBF_HIDDEN) == 0 ){  //not hidden
        nh= nt; //number of the previous visible tab
      }
      t= t->next;
    }
    T->ntabs_scroll= nh;
  }
  if((dir > 0)&&(!T->islast_tab_shown) && (T->ntabs_scroll < T->ntabs-1)){
    nh= T->ntabs_scroll+1;  //locate next tab
    for( t= T->tabs, n= nh; (n > 0)&&(t != NULL); n-- ){
      t= t->next;
    }
    if( t != NULL){
      //skip hidden tabs
      while( (t != NULL) && ((t->flags & TTBF_HIDDEN) != 0) ){
        nh++;
        t= t->next;
      }
      if( t != NULL){
        T->ntabs_scroll= nh;
      }
    }
  }
  if( nhide != T->ntabs_scroll ){
    //update hilight
    set_hilight_off();  //clear previous hilight
    ttb.philight= getButtonFromXY(T, x, y); //set new hilight
    ttb.ntbhilight= T->num;
    //update tooltip text
    set_hilight_tooltip(T);
    //redraw the complete toolbar
    gtk_widget_queue_draw(T->draw);
  }
}

static void draw_img( cairo_t *ctx, struct toolbar_img *pti, int x, int y, int grayed )
{
  cairo_pattern_t *radpat;
  if( pti->fname != NULL ){
    cairo_surface_t *img= cairo_image_surface_create_from_png(pti->fname);
    if( img != NULL ){
      cairo_save(ctx);
      cairo_translate(ctx, x, y);
      cairo_pattern_t *pattern= cairo_pattern_create_for_surface(img);
      cairo_pattern_set_extend(pattern, CAIRO_EXTEND_NONE);
      cairo_set_source(ctx, pattern);

      if( grayed ){
        radpat = cairo_pattern_create_rgba(0, 0, 0, 0.5);
        cairo_mask(ctx, radpat);
        cairo_pattern_destroy(radpat);
      }else{
        //cairo_rectangle(ctx, 0, 0, pti->width, pti->height);
        cairo_paint(ctx);
      }
      cairo_pattern_destroy(pattern);
      cairo_restore(ctx);
      cairo_surface_destroy(img);
    }
  }
}

static void draw_fill_img( cairo_t *ctx, struct toolbar_img *pti, int x, int y, int w, int h )
{
  if( pti->fname != NULL ){
    cairo_surface_t *img= cairo_image_surface_create_from_png(pti->fname);
    if( img != NULL ){
      cairo_save(ctx);
      cairo_translate(ctx, x, y);
      cairo_pattern_t *pattern= cairo_pattern_create_for_surface(img);
      cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
      cairo_set_source(ctx, pattern);
      cairo_rectangle(ctx, 0, 0, w, h);
      cairo_fill(ctx);
      cairo_pattern_destroy(pattern);
      cairo_restore(ctx);
      cairo_surface_destroy(img);
    }
  }
}

static void draw_txt( cairo_t *ctx, const char *txt, int x, int y, int y1, int w, int h, struct color3doubles *color, int fontsz )
{
  if( txt != NULL ){
    cairo_save(ctx);
    cairo_rectangle(ctx, x, y1, w, h );
    cairo_clip(ctx);
    cairo_move_to(ctx, x, y);
    cairo_set_source_rgb(ctx, color->R, color->G, color->B);
    cairo_set_font_size(ctx, fontsz);
    cairo_show_text(ctx, txt);
    cairo_restore(ctx);
  }
}

static void draw_tab(struct toolbar_data *T, cairo_t *cr, struct toolbar_node *t, int x, int y)
{
  int h, hc, x3;
  struct color3doubles *color= &(T->tabtextcolN);
  h= TTBI_TB_NTAB1;
  hc= TTBI_TB_TAB_NCLOSE;
  if( T->closeintabs ){
    if( (ttb.philight != NULL) && (ttb.ntbhilight == T->num) && ((ttb.philight->flags & TTBF_CLOSETAB_BUT) != 0) ){
      //a tab close button is hilited, is from this tab?
      if( ttb.philight->num == t->num ){
        hc= TTBI_TB_TAB_HCLOSE;  //hilight close tab button
        h=  TTBI_TB_HTAB1;       //and tab
        color= &(T->tabtextcolH);
      }
    }
  }
  if( (t->flags & TTBF_ACTIVE) != 0 ){
    h= TTBI_TB_ATAB1;
    color= &(T->tabtextcolA);

  }else if( (t->flags & TTBF_GRAYED) != 0 ){
    h= TTBI_TB_DTAB1;
    color= &(T->tabtextcolG);

  }else if( (t == ttb.philight)&&((ttb.phipress == NULL)||(ttb.phipress == t)) ){
    h= TTBI_TB_HTAB1;
    color= &(T->tabtextcolH);
  }
  draw_img(cr, &(T->img[h]), x, y, 0 );
  draw_fill_img(cr, &(T->img[h+1]), x+t->imgx, y, t->barx2 -t->imgx -T->img[TTBI_TB_NTAB3].width, T->img[TTBI_TB_NTAB2].height );

  x3= x+t->barx2-T->img[TTBI_TB_NTAB3].width;
  draw_img(cr, &(T->img[h+2]), x3, y, 0 );
  if( (t->flags & TTBF_CHANGED) != 0 ){
    if( T->tabmodshow == 1 ){
      draw_img(cr, &(T->img[TTBI_TB_TAB_CHANGED]), x3, y, 0 );
    }else if( T->tabmodshow == 2 ){
      color= &(T->tabtextcolM);
    }
  }
  if( T->closeintabs ){
    draw_img(cr, &(T->img[hc]), x3, y, 0 );
  }

  draw_txt(cr, t->text, x+t->imgx, y+t->imgy, y, x3-x, T->img[TTBI_TB_NTAB2].height, color, T->tabfontsz );
}

static struct toolbar_node * find_prev_tab( struct toolbar_data *T, struct toolbar_node * tab )
{
  struct toolbar_node * p;
  if( T->tabs != tab ){
    for( p= T->tabs; (p != NULL); p= p->next ){
      if( p->next == tab ){
        return p; //previous tab in the list
      }
    }
  }
  return NULL;  //first (or not found)
}

static void goto_ttb_tab(struct toolbar_data *T, int tabpos)
{  //generate a click in tab: -1:prev,1:next,0:first,2:last
  struct toolbar_node *p, *pcurr;
  p= NULL;
  if( T->tab_node != NULL ){
    //find the current active tab
    for( pcurr= T->tabs; (pcurr != NULL); pcurr= pcurr->next ){
      if( (pcurr->flags & TTBF_ACTIVE) != 0 ){
        break;
      }
    }
    if( tabpos == 1 ){
      //next visible tab
      if( pcurr != NULL ){
        p= pcurr->next;
        while( (p != NULL) && ((p->flags & TTBF_HIDDEN) != 0) ){
          p= p->next; //skip hidden tabs
        }
      }
      if( p == NULL ){
        tabpos= 0; //activate the first one
      }
    }else if( tabpos == -1 ){
      //prev visible tab
      if( pcurr != NULL ){
        p= find_prev_tab( T, pcurr );
        while( (p != NULL) && ((p->flags & TTBF_HIDDEN) != 0) ){
          p= find_prev_tab( T, p ); //skip hidden tabs
        }
      }
      if( p == NULL ){
        tabpos= 2; //activate the last one
      }
    }
    if( tabpos == 0 ){
      //first visible tab
      for( p= T->tabs; (p != NULL); p= p->next ){
        if( (p->flags & TTBF_HIDDEN) == 0 ){
          break;
        }
      }
    }else if( tabpos == 2 ){
      //last visible tab
      p= T->tabs_last;
      while( (p != NULL) && ((p->flags & TTBF_HIDDEN) != 0) ){
        p= find_prev_tab( T, p ); //skip hidden tabs
      }
    }
    if( (p != NULL) && (p != pcurr) ){
      lL_event(lua, "toolbar_tabclicked", LUA_TNUMBER, p->num, LUA_TNUMBER, T->num, -1);
    }
  }
}

/* ============================================================================= */
/*                                EVENTS                                         */
/* ============================================================================= */
static struct toolbar_data * toolbar_from_widget(GtkWidget *widget)
{
  int i;
  for( i= 0; i < NTOOLBARS; i++){
    if( widget == ttb.tbdata[i].draw ){
      return &(ttb.tbdata[i]);
    }
  }
  return &(ttb.tbdata[0]);  //toolbar?? use #0
}

static struct toolbar_data * toolbar_from_num(int num)
{
  if( (num >= 0) && (num < NTOOLBARS) ){
    return &(ttb.tbdata[num]);
  }
  return &(ttb.tbdata[0]);  //toolbar?? use #0
}


static int need_redraw(GdkEventExpose *event, int x, int y, int xf, int yf)
{
  int x0, y0, x1, y1;
  //area to paint
  x0= event->area.x;
  y0= event->area.y;
  x1= x0 + event->area.width;
  y1= y0 + event->area.height;
  return ((x <= x1) && (y <= y1) && (xf >= x0) && (yf >= y0));
}

static void ttb_size_ev(GtkWidget *widget, GdkRectangle *prec, void*__)
{
  UNUSED(__);
  struct toolbar_data *T= toolbar_from_widget(widget);

  T->barwidth= prec->width;
  T->barheight= prec->height;
  //split free space in tabs that expand
  update_tabs_size(T);
}

static gboolean ttb_paint_ev(GtkWidget *widget, GdkEventExpose *event, void*__)
{
  UNUSED(__);
  struct toolbar_node *p, *phi, *t, *ta;
  struct toolbar_img *pti;
  int h, grayed, x, y, xa, nhide;
  struct color3doubles *color;
  struct toolbar_data *T= toolbar_from_widget(widget);

  if( (T->barwidth < 0) || (T->barheight < 0) ){
    T->barwidth=  widget->allocation.width;
    T->barheight= widget->allocation.height;
    //split free space in tabs that expand
    update_tabs_size(T);
  }

  cairo_t *cr = gdk_cairo_create(widget->window);
  //draw background image (if any)
  draw_fill_img(cr, &(T->img[TTBI_TB_BACKGROUND]), 0, 0, T->barwidth, T->barheight );
  //draw hilight (under regular buttons)
  phi= NULL;
  if( ttb.ntbhilight == T->num ){
    phi= ttb.philight;
  }
  if( (phi != NULL) && ((phi->flags & (TTBF_TAB|TTBF_SCROLL_BUT|TTBF_CLOSETAB_BUT))==0) ){
    if( need_redraw(event, phi->barx1, phi->bary1, phi->barx2, phi->bary2) ){
      h= -1;
      if(ttb.phipress == phi){
        h= TTBI_HIPRESSED; //hilight as pressed
      }else if(ttb.phipress == NULL){
        h= TTBI_HILIGHT; //normal hilight (and no other button is pressed)
      }
      if( h >= 0){
        if( (phi->flags & TTBF_TEXT) == 0 ){
          //graphic button
          //try to use the button hilight version
          pti= &(phi->img[h]);
          if( pti->fname == NULL ){
            //use the toolbar version
            pti= &(T->img[h]);
          }
          draw_img(cr, pti, phi->barx1, phi->bary1, 0 );
        }else{
          //text button
          if( h == TTBI_HILIGHT ){
            h= TTBI_TB_TXT_HIL1;  //normal hilight
          }else{
            h= TTBI_TB_TXT_HPR1;  //hilight as pressed
          }
          x= phi->barx1;
          draw_img(cr, &(T->img[h]), x, phi->bary1, 0 );
          x += T->img[TTBI_TB_TXT_HIL1].width;
          xa= phi->barx2 - T->img[TTBI_TB_TXT_HIL3].width;
          draw_fill_img(cr, &(T->img[h+1]), x, phi->bary1, xa-x, T->img[h].height );
          draw_img(cr, &(T->img[h+2]), xa, phi->bary1, 0 );
          //draw_txt(cr, phi->text, phi->imgx, phi->imgy, phi->bary1, xa-phi->imgx, T->img[h].height, &(T->txttextcolN), T->txtfontsz );
        }
      }
    }
  }
  //draw all button images
  ta= NULL;
  xa= 0;
  for( p= T->list; (p != NULL); p= p->next ){
    if( (p->flags & TTBF_TABBAR) != 0){
      //tab-bar
      if( need_redraw( event, p->barx1, p->bary1, T->barwidth, p->bary2) ){
        x= p->barx1;
        y= p->bary1;
        draw_fill_img(cr, &(T->img[TTBI_TB_TABBACK]), x, y, T->barwidth, p->bary2 );
        x += T->tabxmargin;
        for( t= T->tabs, nhide= T->ntabs_scroll; (nhide > 0)&&(t != NULL); nhide-- ){
          t= t->next; //skip hidden tabs (scroll support)
        }
        for( ; (t != NULL); t= t->next ){
          if( (t->flags & TTBF_HIDDEN) == 0 ){  //skip hidden tabs
            if( need_redraw( event, x, y, x+t->barx2, y+t->bary2) ){
              if( (t->flags & TTBF_ACTIVE) != 0 ){
                ta= t;
                xa= x;
              }else{
                draw_tab( T, cr, t, x, y );
              }
            }
            x += t->barx2 + T->tabxsep;
          }
        }
        T->islast_tab_shown= (T->barwidth >= x );
        //draw the active tab over the other tabs
        if( ta != NULL ){
          draw_tab( T, cr, ta, xa, y );
        }
        T->xscleft= -1;
        T->xscright= -1;
        //draw scroll indicator over the tabs
        if( T->ntabs_scroll > 0 ){
          T->xscleft= p->barx1+T->tabxmargin;
          h= TTBI_TB_TAB_NSL;
          if( (phi != NULL)&&(phi==ttb.phipress)&&(phi->flags==TTBF_SCROLL_BUT)&&(phi->num==-1)){
            h= TTBI_TB_TAB_HSL;
          }
          draw_img(cr, &(T->img[h]), T->xscleft, p->bary1, 0 );
        }
        if( !T->islast_tab_shown ){
          T->xscright= T->barwidth-T->img[TTBI_TB_TAB_NSR].width;
          h= TTBI_TB_TAB_NSR;
          if( (phi != NULL)&&(phi==ttb.phipress)&&(phi->flags==TTBF_SCROLL_BUT)&&(phi->num==1)){
            h= TTBI_TB_TAB_HSR;
          }
          draw_img(cr, &(T->img[h]), T->xscright, p->bary1, 0 );
        }
      }
    }else{
      //buttons
      if( need_redraw( event, p->imgx, p->imgy, p->barx2, p->bary2) ){
        h= TTBI_NORMAL;
        grayed= 0;
        if( (p->flags & TTBF_TEXT) == 0 ){
          //graphic button
          if( (p->flags & TTBF_GRAYED) != 0){
            if( p->img[TTBI_DISABLED].fname != NULL ){
              h= TTBI_DISABLED;
            }else{
              grayed= 1; //there is no disabled image, gray it
            }
          }
          draw_img(cr, &(p->img[h]), p->imgx, p->imgy, grayed );
        }else{
          //text button
          color= &(T->txttextcolN);
          if( (p->flags & TTBF_GRAYED) != 0){
            color= &(T->txttextcolG);
          }
          xa= p->barx2 - T->img[TTBI_TB_TXT_HIL3].width;
          draw_txt(cr, p->text, p->imgx, p->imgy, p->bary1, xa - p->imgx, p->bary2 - p->bary1, color, T->txtfontsz );
        }
      }
    }
  }
  cairo_destroy(cr);
  return TRUE;
}

static gboolean ttb_mouseleave_ev(GtkWidget *widget, GdkEventCrossing *event)
{
  UNUSED(event);
  struct toolbar_data *T= toolbar_from_widget(widget);

  if( (ttb.philight != NULL) && (ttb.ntbhilight == T->num) ){
    //force hilight and tooltip OFF (in this toolbar only)
    set_hilight_off();
    clear_tooltip_text(T);
  }
  return FALSE;
}

static gboolean ttb_mousemotion_ev( GtkWidget *widget, GdkEventMotion *event )
{
  int x, y, nx, xhi, ok;
  GdkModifierType state;
  struct toolbar_node * p, *prev;

  struct toolbar_data *T= toolbar_from_widget(widget);
  if(event->is_hint){
    gdk_window_get_pointer(event->window, &x, &y, &state);
  }else{
    x = event->x;
    y = event->y;
    state = event->state;
  }
//  if( (state & GDK_BUTTON1_MASK) == 0 ){
//    if( ttb.phipress != NULL ) //mouse release event lost or coming?
//  }
  p= getButtonFromXY(T, x, y);
  if( p != ttb.philight ){
    //hilight changed
    if( (p != NULL) && (ttb.phipress != NULL) && is_dragtab_enabled(T) && (p != ttb.phipress) &&
        ((p->flags & TTBF_TAB) != 0) && ((ttb.phipress->flags & TTBF_TAB) != 0) ){
      //drag tab from "ttb.phipress" to "p" position
      ok= 1;
      if( p == ttb.phipress->next ){
        ok= 2;  //special case: move the tab one place to the right (swap tabs)
      }
      //remove the dragged tab from the list
      prev= find_prev_tab( T, ttb.phipress );
      if( prev == NULL){
        if( T->tabs == ttb.phipress ){
          //remove from list head
          T->tabs= ttb.phipress->next;
          ttb.phipress->next= NULL;
        }else{
          ok= 0; //tab position???? ignore the drag
        }
      }else{
        //remove from list
        prev->next= ttb.phipress->next;
        ttb.phipress->next= NULL;
      }
      if( ok == 1 ){
        //put dragged tab before "p"
        ttb.phipress->next= p;
        prev= find_prev_tab( T, p );
        if( prev == NULL){
          T->tabs= ttb.phipress;
        }else{
          prev->next= ttb.phipress;
        }
      }else if( ok == 2 ){
        //put dragged tab after "p"
        ttb.phipress->next= p->next;
        p->next= ttb.phipress;
      }
      if( ok != 0 ){
        //update the last tab (insertion point)
        T->tabs_last= find_prev_tab( T, NULL );
        //hilight the dragged tab
        p= ttb.phipress;
        //redraw the complete toolbar
        gtk_widget_queue_draw(T->draw);
      }
    }
    //clear previous hilight (in any toolbar)
    if( (ttb.philight != NULL) && (ttb.ntbhilight >= 0) ){
      redraw_button( &(ttb.tbdata[ttb.ntbhilight]), ttb.philight );
    }
    ttb.philight= p;
    ttb.ntbhilight= T->num;
    redraw_button(T, ttb.philight); //redraw new hilighted button in this toolbar
    //update tooltip text
    set_hilight_tooltip(T);
  }
  return TRUE;
}

static gboolean ttb_scrollwheel_ev(GtkWidget *widget, GdkEventScroll* event, void*__)
{
  UNUSED(__);
  struct toolbar_data *T= toolbar_from_widget(widget);
  //don't scroll if a button is pressed (mouse still down)
  if( ttb.phipress == NULL ){
    if( (event->direction == GDK_SCROLL_UP)||(event->direction == GDK_SCROLL_LEFT) ){
      scroll_tabs(T, event->x, event->y, -1);
    }else{
      scroll_tabs(T, event->x, event->y, 1);
    }
  }
  return TRUE;
}

static gboolean ttb_button_ev(GtkWidget *widget, GdkEventButton *event, void*__)
{
  struct toolbar_node * p;
  UNUSED(__);
  struct toolbar_data *T= toolbar_from_widget(widget);

  if( (event->button == 1)||(event->button == 3) ){
    if(event->type == GDK_BUTTON_PRESS){
      set_hilight_off();  //clear previous hilight
      ttb.phipress= getButtonFromXY(T, event->x, event->y);
      if( ttb.phipress != NULL ){
        ttb.philight= ttb.phipress; //hilight as pressed
        ttb.ntbhilight= T->num;
        redraw_button(T, ttb.philight); //redraw button
      }
      return TRUE;
    }
    if(event->type == GDK_BUTTON_RELEASE){
      clear_tooltip_text(T);
      p= getButtonFromXY(T, event->x, event->y);
      if( (p != NULL) && (p == ttb.phipress) && (ttb.ntbhilight == T->num) ){
        //button pressed (mouse press and release over the same button)

        //NOTE: this prevents to keep a hilited button when a dialog is open from the event
        //(but also removes the hilite until the mouse is moved)
        set_hilight_off();

        if( (p->flags & TTBF_SCROLL_BUT) != 0 ){
          scroll_tabs(T, event->x, event->y, p->num);

        }else if( (p->flags & TTBF_CLOSETAB_BUT) != 0 ){
          lL_event(lua, "toolbar_tabclicked", LUA_TNUMBER, p->num, LUA_TNUMBER, T->num, -1);
          lL_event(lua, "toolbar_tabclose",   LUA_TNUMBER, p->num, LUA_TNUMBER, T->num, -1);

        }else if( (p->flags & TTBF_TAB) == 0 ){
          if((event->button == 1) && (p->name != NULL)){
            lL_event(lua, "toolbar_clicked", LUA_TSTRING, p->name, LUA_TNUMBER, T->num, -1);
          }
        }else{
          if(event->button == 1){
            lL_event(lua, "toolbar_tabclicked", LUA_TNUMBER, p->num, LUA_TNUMBER, T->num, -1);
          }else if(event->button == 3){
            if( lL_event(lua, "toolbar_tabRclicked", LUA_TNUMBER, p->num, LUA_TNUMBER, T->num, -1) ){
              lL_showcontextmenu(lua, event, "tab_context_menu"); //open context menu
            }
          }
        }
      }else{
        redraw_button(T, p); 			      //redraw button under mouse (if any)
        if( ttb.ntbhilight == T->num ){
          redraw_button(T, ttb.philight); //redraw hilighted button (if any in this toolbar)
        }
      }
      ttb.phipress= NULL;
      return TRUE;
    }
    if(event->type == GDK_2BUTTON_PRESS){ //double click
      if(event->button == 1){
        p= getButtonFromXY(T, event->x, event->y);
        if( p != NULL ){
          if( (p->flags & TTBF_TAB) != 0 ){
            lL_event(lua, "toolbar_tab2clicked", LUA_TNUMBER, p->num, LUA_TNUMBER, T->num, -1);
          }
        }
      }
      return TRUE;
    }
  }
  return FALSE;
}

//ntoolbar=0: HORIZONTAL
//ntoolbar=1: VERTICAL
//ntoolbar=2: HORIZONTAL
static void create_tatoolbar( GtkWidget *vbox, int ntoolbar )
{
  GtkWidget * drawing_area;

  if( ntoolbar < NTOOLBARS ){
    drawing_area = gtk_drawing_area_new();
    ttb.tbdata[ntoolbar].draw= drawing_area;
    ttb.tbdata[ntoolbar].num=  ntoolbar;
    ttb.tbdata[ntoolbar].isvertical= (ntoolbar == 1);
    ttb.tbdata[ntoolbar].isvisible= 0;
    if( ttb.tbdata[ntoolbar].isvertical ){
      gtk_widget_set_size_request(drawing_area, 1, -1);
    }else{
      gtk_widget_set_size_request(drawing_area, -1, 1);
    }
    gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK|GDK_LEAVE_NOTIFY_MASK|
      GDK_POINTER_MOTION_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK );
    signal(drawing_area, "size-allocate",        ttb_size_ev);
    signal(drawing_area, "expose_event",         ttb_paint_ev);
    signal(drawing_area, "leave-notify-event",   ttb_mouseleave_ev);
    signal(drawing_area, "motion_notify_event",  ttb_mousemotion_ev);
    signal(drawing_area, "scroll-event",         ttb_scrollwheel_ev);
    signal(drawing_area, "button-press-event",   ttb_button_ev);
    signal(drawing_area, "button-release-event", ttb_button_ev);

    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, FALSE, FALSE, 0);
  }
}

static void show_toolbar(struct toolbar_data *T, int show)
{ //show/hide one toolbar
  if( show ){
    //show this toolbar
    T->isvisible= 1;
    gtk_widget_show( T->draw );
    //redraw the complete toolbar
    gtk_widget_queue_draw(T->draw);
    if( T->num == 2 ){
      gtk_widget_hide( statusbar[0] );
      gtk_widget_hide( statusbar[1] );
    }
  }else{
    //hide this toolbar
    T->isvisible= 0;
    gtk_widget_hide( T->draw );
    if( T->num == 2 ){
      gtk_widget_show( statusbar[0] );
      gtk_widget_show( statusbar[1] );
    }
  }
}

static void show_tatoolbar(int show)
{ //show/hide ALL toolbars
  int i;
  for( i= 0; i < NTOOLBARS; i++ ){
    if( ttb.tbdata[i].draw != NULL ){
      if( show ){
        //show all toolbars
        gtk_widget_show( ttb.tbdata[i].draw );
      }else{
        //hide all toolbars
        gtk_widget_hide( ttb.tbdata[i].draw );
      }
    }
  }
}

/* ============================================================================= */
/*                                 LUA FUNCTIONS                                 */
/* ============================================================================= */
/** `toolbar.new(barsize,buttonsize,imgsize,toolbarnum/isvertical,imgpath)` Lua function. */
static int ltoolbar_new(lua_State *L)
{
  int i, num;
  char str[32];
  struct toolbar_data *T;

  if( lua_isboolean(L,4) ){
    num= 0;   //FALSE: horizontal = #0
    if( lua_toboolean(L,4) ){
      num= 1; //TRUE:  vertical   = #1
    }
  }else{
    num= lua_tointeger(L, 4);   //toolbar number (0:horizonal, 1:vertical,..)
  }
  if( (num < 0) || (num >= NTOOLBARS) ){
    num= 0; //default #0
  }
  T= toolbar_from_num(num);
  //destroy current toolbar
  kill_toolbar_num(num);
  //set as current toolbar
  ttb.currentntb= num;

  if( !lua_isnone(L, 5) ){ //change global image base
    ttb.img_base= chg_alloc_str(ttb.img_base, luaL_checkstring(L, 5));
  }
  //set default toolbar images
  set_tb_img( T, NULL, TTBI_TB_HILIGHT,   "ttb-back-hi");
  set_tb_img( T, NULL, TTBI_TB_HIPRESSED, "ttb-back-press");
  if( T->isvertical ){
    set_tb_img( T, NULL, TTBI_TB_SEPARATOR, "ttb-hsep" );
  }else{
    set_tb_img( T, NULL, TTBI_TB_SEPARATOR, "ttb-vsep" );
  }

  //3 images per tab state: beginning, middle, end
  strcpy( str, "ttb-#tab#" );
  for( i= 0; i < 3; i++){
    str[8]= '1'+i;
    str[4]= 'n';
    set_tb_img( T, NULL, TTBI_TB_NTAB1+i, str ); //normal
    str[4]= 'd';
    set_tb_img( T, NULL, TTBI_TB_DTAB1+i, str ); //disabled
    str[4]= 'h';
    set_tb_img( T, NULL, TTBI_TB_HTAB1+i, str ); //hilight
    str[4]= 'a';
    set_tb_img( T, NULL, TTBI_TB_ATAB1+i, str ); //active
  }
  //set_tb_img( T, NULL, TTBI_TB_TABBACK, "ttb-tab-back" );   //tab background
  set_tb_img( T, NULL, TTBI_TB_TAB_NSL,    "ttb-tab-sl"    ); //normal tab scroll left
  set_tb_img( T, NULL, TTBI_TB_TAB_NSR,    "ttb-tab-sr"    ); //normal tab scroll right
  set_tb_img( T, NULL, TTBI_TB_TAB_HSL,    "ttb-tab-hsl"   ); //hilight tab scroll left
  set_tb_img( T, NULL, TTBI_TB_TAB_HSR,    "ttb-tab-hsr"   ); //hilight tab scroll right
  set_tb_img( T, NULL, TTBI_TB_TAB_NCLOSE, "ttb-tab-close" ); //normal close button
  set_tb_img( T, NULL, TTBI_TB_TAB_HCLOSE, "ttb-tab-hclose"); //hilight close button
  set_tb_img( T, NULL, TTBI_TB_TAB_CHANGED,"ttb-tab-chg"   ); //change indicator

  //3 images per text-button state: beginning, middle, end
  set_tb_img( T, NULL, TTBI_TB_TXT_HIL1, "ttb-back-hi1" );    //hilight
  set_tb_img( T, NULL, TTBI_TB_TXT_HIL2, "ttb-back-hi2" );
  set_tb_img( T, NULL, TTBI_TB_TXT_HIL3, "ttb-back-hi3" );
  set_tb_img( T, NULL, TTBI_TB_TXT_HPR1, "ttb-back-press1" ); //hi-pressed
  set_tb_img( T, NULL, TTBI_TB_TXT_HPR2, "ttb-back-press2" );
  set_tb_img( T, NULL, TTBI_TB_TXT_HPR3, "ttb-back-press3" );

  T->barheight= -1;
  T->barwidth= -1;
  T->xmargin= 1;
  T->ymargin= 1;
  if( T->isvertical ){
    //ttb.drawing_area= ttb.draw[1];	//use toolbar 1 = vertical
    T->barwidth= lua_tointeger(L, 1);
    T->ymargin= 2;
  }else{
    //ttb.drawing_area= ttb.draw[0];  //use toolbar 0 = horizontal
    T->barheight= lua_tointeger(L, 1);
    T->xmargin= 2;
  }
  //default: square buttons
  T->bwidth= lua_tointeger(L, 2);
  T->bheight= T->bwidth;
  T->xoff= (T->bwidth - lua_tointeger(L, 3))/2;
  if( T->xoff < 0){
    T->xoff= 0;
  }
  T->yoff= T->xoff;
  T->xnew= T->xmargin;
  T->ynew= T->ymargin;
  gtk_widget_set_size_request(T->draw, T->barwidth, T->barheight);
  return 0;
}

/** `toolbar.adjust(bwidth,bheight,xmargin,ymargin,xoff,yoff)` Lua function. */
static int ltoolbar_adjust(lua_State *L)
{
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  T->bwidth=  lua_tointeger(L, 1);
  T->bheight= lua_tointeger(L, 2);
  T->xmargin= lua_tointeger(L, 3);
  T->ymargin= lua_tointeger(L, 4);
  T->xoff=    lua_tointeger(L, 5);
  T->yoff=    lua_tointeger(L, 6);
  T->xnew= T->xmargin;
  T->ynew= T->ymargin;
  return 0;
}

/** `toolbar.seltoolbar(toolbarnum/isvertical)` Lua function. */
static int ltoolbar_seltoolbar(lua_State *L)
{
  int num;
  if( lua_isboolean(L,1) ){
    num= 0;   //FALSE: horizontal = #0
    if( lua_toboolean(L,1) ){
      num= 1; //TRUE:  vertical   = #1
    }
  }else{
    num= lua_tointeger(L, 1);   //toolbar number (0:horizonal, 1:vertical,..)
  }
  if( (num < 0) || (num >= NTOOLBARS) ){
    num= 0; //default #0
  }
  //set as current toolbar
  ttb.currentntb= num;
  return 0;
}

/** `toolbar.addbutton(name,tooltiptext)` Lua function. */
static int ltoolbar_addbutton(lua_State *L)
{
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  const char *name= luaL_checkstring(L, 1);
  add_ttb_node( T, name, name, luaL_checkstring(L, 2), NULL, 0);
  return 0;
}

/** `toolbar.addtext(name,text,tooltiptext,width)` Lua function. */
static int ltoolbar_addtext(lua_State *L)
{
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  const char *name= luaL_checkstring(L, 1);
  add_ttb_node( T, name, NULL, luaL_checkstring(L, 3), luaL_checkstring(L, 2), lua_tointeger(L, 4));
  return 0;
}

/** `toolbar.addspace(space,hidebar)` Lua function. */
static int ltoolbar_addspace(lua_State *L)
{
  struct toolbar_node * p;
  int asep;
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  int x= lua_tointeger(L, 1);
  int hide= lua_toboolean(L,2);
  if( T->isvertical ){
    if( x == 0 ){
      x= T->bheight/2;
    }
    if( !hide ){
      //show H separator in the middle
      p= add_ttb_node( T, NULL, T->img[TTBI_TB_SEPARATOR].fname, NULL, NULL, 0);
      if( p != NULL ){
        asep= T->img[TTBI_TB_SEPARATOR].height; //minimun separator = image height
        if( x < asep ){
          x= asep;
        }
        T->ynew -= T->bheight;
        p->imgx= T->xnew;
        p->imgy= T->ynew + ((x-asep)/2);
        p->bary2= T->ynew + x;
      }
    }
    T->ynew += x;
  }else{
    if( x == 0 ){
      x= T->bwidth/2;
    }
    if( !hide ){
      //show V separator in the middle
      p= add_ttb_node( T, NULL, T->img[TTBI_TB_SEPARATOR].fname, NULL, NULL, 0);
      if( p != NULL ){
        asep= T->img[TTBI_TB_SEPARATOR].width; //minimun separator = image width
        if( x < asep ){
          x= asep;
        }
        T->xnew -= T->bwidth;
        p->imgx= T->xnew + ((x-asep)/2);
        p->imgy= T->ynew;
        p->barx2= T->xnew + x;
      }
    }
    T->xnew += x;
  }
  return 0;
}

/** `toolbar.gotopos(x,y)` Lua function. */
/** `toolbar.gotopos(dx)`  Lua function. */
static int ltoolbar_gotopos(lua_State *L)
{
  int x,y;
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  x= lua_tointeger(L, 1);
  if( lua_isnone(L, 2) ){
    //only one parameter: new row/column
    if( T->isvertical ){
      //new column
      x= T->xnew + T->bwidth + x;
      y= T->ymargin;
    }else{
      //new row
      y= T->ynew + T->bheight + x;
      x= T->xmargin;
    }
  }else{
    //2 parameters: x,y
    y= lua_tointeger(L, 2);
  }
  T->xnew= x;
  T->ynew= y;
  return 0;
}

/** `toolbar.show(show)` Lua function. */
static int ltoolbar_show(lua_State *L)
{
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  show_toolbar(T, lua_toboolean(L,1));
  return 0;
}

/** `toolbar.enable(name,isenabled,onlyinthistoolbar)` Lua function. */
static int ltoolbar_enable(lua_State *L)
{
  int i;
  const char *name= luaL_checkstring(L, 1);
  if( lua_toboolean(L,3) ){
    //enable button in this toolbar only
    ttb_enable_button(toolbar_from_num(ttb.currentntb), name, lua_toboolean(L,2) );
  }else{
    //enable button in every toolbar
    for( i= 0; i < NTOOLBARS; i++){
      ttb_enable_button(toolbar_from_num(i), name, lua_toboolean(L,2) );
    }
  }
  return 0;
}

/** `toolbar.seticon(name,icon,[nicon],onlyinthistoolbar)` Lua function. */
static int ltoolbar_seticon(lua_State *L)
{
  int i;
  const char *name= luaL_checkstring(L, 1);
  const char *img= luaL_checkstring(L, 2);
  if( lua_toboolean(L,4) ){
    //set icon in this toolbar only
    ttb_change_button_img(toolbar_from_num(ttb.currentntb), name, lua_tointeger(L, 3), img );
  }else{
    //set icon in every toolbar
    for( i= 0; i < NTOOLBARS; i++){
      ttb_change_button_img(toolbar_from_num(i), name, lua_tointeger(L, 3), img );
    }
  }
  return 0;
}

/** `toolbar.settooltip(name,tooltip,[onlyinthistoolbar])` Lua function. */
static int ltoolbar_settooltip(lua_State *L)
{
  int i;
  const char *name= luaL_checkstring(L, 1);
  const char *tooltip= luaL_checkstring(L, 2);
  if( lua_toboolean(L,3) ){
    //set button's tooltip in this toolbar only
    ttb_change_button_tooltip(toolbar_from_num(ttb.currentntb), name, tooltip );
  }else{
    //set button's tooltip in every toolbar
    for( i= 0; i < NTOOLBARS; i++){
      ttb_change_button_tooltip(toolbar_from_num(i), name, tooltip );
    }
  }
  return 0;
}

/** `toolbar.settext(name,text,[tooltip],[onlyinthistoolbar])` Lua function. */
static int ltoolbar_settext(lua_State *L)
{
  int i;
  const char *name= luaL_checkstring(L, 1);
  const char *text= luaL_checkstring(L, 2);
  const char *tooltip= NULL;
  if( !lua_isnone(L, 3) ){
    tooltip= luaL_checkstring(L, 3);
  }
  if( lua_toboolean(L,4) ){
    //set button's text/tooltip in this toolbar only
    ttb_change_button_text(toolbar_from_num(ttb.currentntb), name, text );
    if(tooltip != NULL){
      ttb_change_button_tooltip(toolbar_from_num(ttb.currentntb), name, tooltip );
    }
  }else{
    //set button's text/tooltip in every toolbar
    for( i= 0; i < NTOOLBARS; i++){
      ttb_change_button_text(toolbar_from_num(i), name, text );
      if(tooltip != NULL){
        ttb_change_button_tooltip(toolbar_from_num(i), name, tooltip );
      }
    }
  }
  return 0;
}

/** `toolbar.addtabs(xmargin,xsep,withclose,mod-show,fontsz,fontyoffset,[tab-drag])` Lua function. */
static int ltoolbar_addtabs(lua_State *L)
{
  cairo_t * cr;
  cairo_text_extents_t ext;
  int i, rgb;
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  if( T->tab_node == NULL ){ //only one tabbar for now
    T->tab_node= add_ttb_node( T, NULL, NULL, NULL, NULL, 0);
    if( T->tab_node != NULL ){
      T->tab_node->flags |= TTBF_TABBAR|TTBF_SELECTABLE;	//show tabs here
      if( lua_toboolean(L,7) ){
        T->tab_node->flags |= TTBF_DRAGTAB; //enable drag support
      }
      T->tabxmargin= lua_tointeger(L, 1);
      T->tabxsep= lua_tointeger(L, 2);

      T->tabmodshow= lua_tointeger(L, 4);

      T->tabfontsz= lua_tointeger(L, 5);  //font size in points (default = 10 points)
      if( T->tabfontsz < 2){
        T->tabfontsz= 10;
      }
      cr= gdk_cairo_create(T->draw->window);
      cairo_set_font_size(cr, T->tabfontsz);
      cairo_text_extents( cr, "H", &ext );
      T->tabtexth= (int) ext.height;
      cairo_destroy(cr);

      T->tabtextoff= lua_tointeger(L, 6);
      //center text verticaly + offset
      T->tabtexty=  ((T->img[TTBI_TB_NTAB1].height+T->tabtexth)/2)+T->tabtextoff;
      if( T->tabtexty < 0){
        T->tabtexty= 0;
      }
      T->tab_node->bary1 -= T->ymargin;
      T->tabheight= T->tabtexth; //use the tallest image or text
      for(i= TTBI_TB_TABBACK; i <= TTBI_TB_ATAB3; i++ ){
        if( T->tabheight < T->img[i].height ){
          T->tabheight= T->img[i].height;
        }
      }
      T->tab_node->bary2= T->tab_node->bary1 + T->tabheight;
      T->closeintabs= lua_toboolean(L,3);
      //split free space in tabs that expand (none for now...)
      update_tabs_size(T);
      redraw_button(T, NULL); //redraw the complete toolbar
    }
  }
  return 0;
}

static void setluacolor(lua_State *L, int ncolor, struct color3doubles *pc )
{
  if( !lua_isnone(L, ncolor) ){
    setrgbcolor(lua_tointeger(L, ncolor), pc);
  }
}

/** `toolbar.tabfontcolor(NORMcol,HIcol,ACTIVEcol,MODIFcol,GRAYcol)` Lua function. */
static int ltoolbar_tabfontcolor(lua_State *L)
{
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  redraw_tabs_beg(T);

  setrgbcolor( 0x000000, &T->tabtextcolN);  //normal: default black
  setluacolor( L, 1, &(T->tabtextcolN) );
  T->tabtextcolH= T->tabtextcolN;   //hilight: default == normal
  setluacolor( L, 2, &(T->tabtextcolH) );
  T->tabtextcolA= T->tabtextcolH;   //active: default == hilight
  setluacolor( L, 3, &(T->tabtextcolA) );
  T->tabtextcolM= T->tabtextcolN;   //modified: default == normal
  setluacolor( L, 4, &(T->tabtextcolM) );

  setrgbcolor( 0x808080, &T->tabtextcolG);  //grayed: default medium gray
  setluacolor( L, 5, &(T->tabtextcolG) );
  redraw_tabs_end(T);
  return 0;
}

/** `toolbar.textfont(fontsize,fontyoffset,NORMcol,GRAYcol)` Lua function. */
static int ltoolbar_textfont(lua_State *L)
{
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);

  T->txtfontsz= lua_tointeger(L, 1);  //font size in points (default = 12 points)
  if( T->txtfontsz < 2){
    T->txtfontsz= 12;
  }
  T->txttextoff= lua_tointeger(L, 2);

  setrgbcolor( 0x000000, &T->txttextcolN);  //normal: default black
  setluacolor( L, 3, &(T->txttextcolN) );

  setrgbcolor( 0x808080, &T->txttextcolG);  //grayed: default medium gray
  setluacolor( L, 4, &(T->txttextcolG) );
  redraw_button(T, NULL); //redraw the complete toolbar
  return 0;
}

/** `toolbar.settab(num,name,tooltiptext)` Lua function. */
static int ltoolbar_settab(lua_State *L)
{
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  set_ttb_tab( T, lua_tointeger(L, 1), luaL_checkstring(L, 2), luaL_checkstring(L, 3), 1);
  return 0;
}

/** `toolbar.deletetab(num)` Lua function. */
static int ltoolbar_deletetab(lua_State *L)
{
  struct toolbar_node *k, *kprev, *p; //, *prev;
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  int ntab= lua_tointeger(L, 1);
  kprev= NULL;
  k= get_ttb_tab(T, ntab);
  if( k != NULL ){
    redraw_tabs_beg(T);
    kprev= find_prev_tab( T, k );
    if( k == T->tabs_last ){
      //the last tab will be deleted, choose the previous as the new "last"
      T->tabs_last= kprev;
    }
    //disconect node
    if( kprev == NULL ){
      T->tabs= k->next;
    }else{
      kprev->next= k->next;
    }
    T->ntabs--;
    if( (k->flags & TTBF_HIDDEN) == 0 ){
      T->ntabs_nothidden--;
      if( k->changewidth < 0 ){
        T->ntabs_expand--;
      }
      clear_tabwidth(T, k);
    }
    //after tab delete, try to remove the left scroll button when a new tab is activated
    if( T->ntabs_scroll > 0 ){
      T->try_scrollpack= 1;
    }
    kill_toolbar_node(k);
    //split free space in tabs that expand
    update_tabs_size(T);
    redraw_tabs_end(T);
  }
  return 0;
}

/** `toolbar.activatetab(num)` Lua function. */
static int ltoolbar_activatetab(lua_State *L)
{
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  activate_ttb_tab( T, lua_tointeger(L, 1));
  return 0;
}

/** `toolbar.enabletab(num,enable)` Lua function. */
static int ltoolbar_enabletab(lua_State *L)
{
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  enable_ttb_tab( T, lua_tointeger(L, 1), lua_toboolean(L,2));
  return 0;
}

/** `toolbar.modifiedtab(num,changed)` Lua function. */
static int ltoolbar_modifiedtab(lua_State *L)
{
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  set_changed_ttb_tab( T, lua_tointeger(L, 1), lua_toboolean(L,2));
  return 0;
}

/** `toolbar.hidetab(num,hide)` Lua function. */
static int ltoolbar_hidetab(lua_State *L)
{
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  hide_ttb_tab( T, lua_tointeger(L, 1), lua_toboolean(L,2));
  return 0;
}

/** `toolbar.tabwidth(num,WW,minwidth,maxwidth)` Lua function. */
/** `toolbar.tabwidth(num,text)` Lua function. */
/** WW= 0:text width, >0:fix, <0:porcent */
static int ltoolbar_tabwidth(lua_State *L)
{
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  int ntab=      lua_tointeger(L, 1);
  int percwidth= 0;
  int minwidth= 0;
  int maxwidth= 0;
  if( lua_isnumber(L,2) ){
    percwidth= lua_tointeger(L, 2);
    minwidth=  lua_tointeger(L, 3);
    maxwidth=  lua_tointeger(L, 4);
  }else if( lua_isstring(L,2) ){
    //use the width of the given text
    cairo_text_extents_t ext;
    cairo_t *cr = gdk_cairo_create(T->draw->window);
    cairo_set_font_size(cr, T->tabfontsz);
    cairo_text_extents( cr, lua_tostring(L, 2), &ext );
    minwidth= ((int) ext.width) + T->tabfixpart;
    cairo_destroy(cr);
    maxwidth=  minwidth;
  }
  change_ttb_tabwidth( T, ntab, percwidth, minwidth, maxwidth );
  return 0;
}

/** `toolbar.gototab(tabpos)` Lua function. */
static int ltoolbar_gototab(lua_State *L)
{
  //generate a click in tab: -1:prev,1:next,0:first,2:last
  struct toolbar_data *T= toolbar_from_num(ttb.currentntb);
  goto_ttb_tab( T, lua_tointeger(L, 1));
  return 0;
}

static void register_toolbar(lua_State *L)
{
  lua_newtable(L);
//toolbar
  l_setcfunction(L, -1, "new",          ltoolbar_new);	        //create a new toolbar
  l_setcfunction(L, -1, "adjust",       ltoolbar_adjust);	      //optionaly fine tune some parameters
  l_setcfunction(L, -1, "seltoolbar",   ltoolbar_seltoolbar);   //select which toolbar to edit
//buttons
  l_setcfunction(L, -1, "addbutton",    ltoolbar_addbutton);    //add button
  l_setcfunction(L, -1, "addtext",      ltoolbar_addtext);      //add text button
  l_setcfunction(L, -1, "addspace",     ltoolbar_addspace);     //add some space
  l_setcfunction(L, -1, "gotopos",      ltoolbar_gotopos);	    //change next button position
  l_setcfunction(L, -1, "show",         ltoolbar_show);	        //show/hide toolbar
  l_setcfunction(L, -1, "enable",       ltoolbar_enable);	      //enable/disable a button
  l_setcfunction(L, -1, "seticon",      ltoolbar_seticon);	    //change a button or TOOLBAR icon
  l_setcfunction(L, -1, "settooltip",   ltoolbar_settooltip);	  //change a button tooltip
  l_setcfunction(L, -1, "settext",      ltoolbar_settext);      //change a button text
  l_setcfunction(L, -1, "textfont",     ltoolbar_textfont);     //set text buttons font size and colors
//tabs
  l_setcfunction(L, -1, "addtabs",      ltoolbar_addtabs);      //show tabs in the toolbar
  l_setcfunction(L, -1, "tabfontcolor", ltoolbar_tabfontcolor); //change default tab font color
  l_setcfunction(L, -1, "settab",       ltoolbar_settab);       //set tab num
  l_setcfunction(L, -1, "deletetab",    ltoolbar_deletetab);    //delete tab num
  l_setcfunction(L, -1, "activatetab",  ltoolbar_activatetab);  //activate tab num
  l_setcfunction(L, -1, "enabletab",    ltoolbar_enabletab);    //enable/disable tab num
  l_setcfunction(L, -1, "modifiedtab",  ltoolbar_modifiedtab);  //show/hide changed indicator in tab num
  l_setcfunction(L, -1, "hidetab",      ltoolbar_hidetab);      //hide/show tab num
  l_setcfunction(L, -1, "tabwidth",     ltoolbar_tabwidth);     //set tab num tabwidth (varible/fixed)
  l_setcfunction(L, -1, "gototab",      ltoolbar_gototab);      //generate a click in tab: -1:prev,1:next,0:first,2:last

  lua_setglobal(L, "toolbar");
}
