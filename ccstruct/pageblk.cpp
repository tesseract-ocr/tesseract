#include "mfcpch.h"
#include          "pageblk.h"
#include    <stdio.h>
#include          <ctype.h>
#include          <math.h>
#ifdef __UNIX__
#include          <unistd.h>
#else
#include          <io.h>
#endif

#include          "hpddef.h"     //must be last (handpd.dll)

#define G_START 0
#define I_START 1
#define R_START 3
#define S_START 5

extern char blabel[NUM_BLOCK_ATTR][4][MAXLENGTH];
extern char backlabel[NUM_BACKGROUNDS][MAXLENGTH];

ELISTIZE_S (PAGE_BLOCK)
void PAGE_BLOCK::pb_delete() {
  switch (pb_type) {
    case PB_TEXT:
      delete ((TEXT_BLOCK *) this);
      break;
    case PB_GRAPHICS:
      delete ((GRAPHICS_BLOCK *) this);
      break;
    case PB_IMAGE:
      delete ((IMAGE_BLOCK *) this);
      break;
    case PB_RULES:
      delete ((RULE_BLOCK *) this);
      break;
    case PB_SCRIBBLE:
      delete ((SCRIBBLE_BLOCK *) this);
      break;
    case PB_WEIRD:
      delete ((WEIRD_BLOCK *) this);
      break;
    default:
      break;
  }
}


#define QUOTE_IT( parm ) #parm

void PAGE_BLOCK::serialise(FILE *f) {

  if (fwrite (&pb_type, sizeof (PB_TYPE), 1, f) != 1)
    WRITEFAILED.error (QUOTE_IT (PAGE_BLOCK::serialise), ABORT, NULL);
  switch (pb_type) {
    case PB_TEXT:
      ((TEXT_BLOCK *) this)->serialise (f);
      break;
    case PB_GRAPHICS:
      ((GRAPHICS_BLOCK *) this)->serialise (f);
      break;
    case PB_RULES:
      ((RULE_BLOCK *) this)->serialise (f);
      break;
    case PB_IMAGE:
      ((IMAGE_BLOCK *) this)->serialise (f);
      break;
    case PB_SCRIBBLE:
      ((SCRIBBLE_BLOCK *) this)->serialise (f);
      break;
    case PB_WEIRD:
      ((WEIRD_BLOCK *) this)->serialise (f);
      break;
    default:
      break;
  }
}


PAGE_BLOCK *PAGE_BLOCK::de_serialise(FILE *f) {
  PB_TYPE type;
  TEXT_BLOCK *tblock;
  GRAPHICS_BLOCK *gblock;
  RULE_BLOCK *rblock;
  IMAGE_BLOCK *iblock;
  SCRIBBLE_BLOCK *sblock;
  WEIRD_BLOCK *wblock;

  if (fread ((void *) &type, sizeof (PB_TYPE), 1, f) != 1)
    WRITEFAILED.error (QUOTE_IT (PAGE_BLOCK::serialise), ABORT, NULL);
  switch (type) {
    case PB_TEXT:
      tblock = (TEXT_BLOCK *) alloc_struct (sizeof (TEXT_BLOCK));
      return tblock->de_serialise (f);
    case PB_GRAPHICS:
      gblock = (GRAPHICS_BLOCK *) alloc_struct (sizeof (GRAPHICS_BLOCK));
      return gblock->de_serialise (f);
    case PB_RULES:
      rblock = (RULE_BLOCK *) alloc_struct (sizeof (RULE_BLOCK));
      return rblock->de_serialise (f);
    case PB_IMAGE:
      iblock = (IMAGE_BLOCK *) alloc_struct (sizeof (IMAGE_BLOCK));
      return iblock->de_serialise (f);
    case PB_SCRIBBLE:
      sblock = (SCRIBBLE_BLOCK *) alloc_struct (sizeof (SCRIBBLE_BLOCK));
      return sblock->de_serialise (f);
    case PB_WEIRD:
      wblock = (WEIRD_BLOCK *) alloc_struct (sizeof (SCRIBBLE_BLOCK));
      return wblock->de_serialise (f);
    default:
      return NULL;
  }
}


/**********************************************************************
 * PAGE_BLOCK::serialise_asc()  Convert to ascii file.
 *
 **********************************************************************/

void PAGE_BLOCK::serialise_asc(         //convert to ascii
                               FILE *f  //file to use
                              ) {
  serialise_INT32(f, pb_type);
  switch (pb_type) {
    case PB_TEXT:
      ((TEXT_BLOCK *) this)->serialise_asc (f);
      break;
    case PB_GRAPHICS:
      ((GRAPHICS_BLOCK *) this)->serialise_asc (f);
      break;
    case PB_RULES:
      ((RULE_BLOCK *) this)->serialise_asc (f);
      break;
    case PB_IMAGE:
      ((IMAGE_BLOCK *) this)->serialise_asc (f);
      break;
    case PB_SCRIBBLE:
      ((SCRIBBLE_BLOCK *) this)->serialise_asc (f);
      break;
    case PB_WEIRD:
      ((WEIRD_BLOCK *) this)->serialise_asc (f);
      break;
    default:
      break;
  }
}


/**********************************************************************
 * PAGE_BLOCK::internal_serialise_asc()  Convert to ascii file.
 *
 **********************************************************************/

void PAGE_BLOCK::internal_serialise_asc(         //convert to ascii
                                        FILE *f  //file to use
                                       ) {
  ((POLY_BLOCK *) this)->serialise_asc (f);
  serialise_INT32(f, pb_type);
  children.serialise_asc (f);
}


/**********************************************************************
 * PAGE_BLOCK::de_serialise_asc()  Convert from ascii file.
 *
 **********************************************************************/

void PAGE_BLOCK::de_serialise_asc(         //convert from ascii
                                  FILE *f  //file to use
                                 ) {
  PAGE_BLOCK *page_block;        //new block for list
  inT32 len;                     /*length to retrive */
  PAGE_BLOCK_IT it;

  ((POLY_BLOCK *) this)->de_serialise_asc (f);
  pb_type = (PB_TYPE) de_serialise_INT32 (f);
  //      children.de_serialise_asc(f);
  len = de_serialise_INT32 (f);
  it.set_to_list (&children);
  for (; len > 0; len--) {
    page_block = new_de_serialise_asc (f);
    it.add_to_end (page_block);  /*put on the list */
  }
}


/**********************************************************************
 * PAGE_BLOCK::new_de_serialise_asc()  Convert from ascii file.
 *
 **********************************************************************/

PAGE_BLOCK *PAGE_BLOCK::new_de_serialise_asc(         //convert from ascii
                                             FILE *f  //file to use
                                            ) {
  PB_TYPE type;
  TEXT_BLOCK *tblock;
  GRAPHICS_BLOCK *gblock;
  RULE_BLOCK *rblock;
  IMAGE_BLOCK *iblock;
  SCRIBBLE_BLOCK *sblock;
  WEIRD_BLOCK *wblock;

  type = (PB_TYPE) de_serialise_INT32 (f);
  switch (type) {
    case PB_TEXT:
      tblock = new TEXT_BLOCK;
      tblock->de_serialise_asc (f);
      return tblock;
    case PB_GRAPHICS:
      gblock = new GRAPHICS_BLOCK;
      gblock->de_serialise_asc (f);
      return gblock;
    case PB_RULES:
      rblock = new RULE_BLOCK;
      rblock->de_serialise_asc (f);
      return rblock;
    case PB_IMAGE:
      iblock = new IMAGE_BLOCK;
      iblock->de_serialise_asc (f);
      return iblock;
    case PB_SCRIBBLE:
      sblock = new SCRIBBLE_BLOCK;
      sblock->de_serialise_asc (f);
      return sblock;
    case PB_WEIRD:
      wblock = new WEIRD_BLOCK;
      wblock->de_serialise_asc (f);
      return wblock;
    default:
      return NULL;
  }
}


void PAGE_BLOCK::show_attrs(DEBUG_WIN *f) {
  PAGE_BLOCK_IT it;

  switch (pb_type) {
    case PB_TEXT:
      ((TEXT_BLOCK *) this)->show_attrs (f);
      break;
    case PB_GRAPHICS:
      ((GRAPHICS_BLOCK *) this)->show_attrs (f);
      break;
    case PB_RULES:
      ((RULE_BLOCK *) this)->show_attrs (f);
      break;
    case PB_IMAGE:
      ((IMAGE_BLOCK *) this)->show_attrs (f);
      break;
    case PB_SCRIBBLE:
      ((SCRIBBLE_BLOCK *) this)->show_attrs (f);
      break;
    case PB_WEIRD:
      ((WEIRD_BLOCK *) this)->show_attrs (f);
      break;
    default:
      break;
  }

  if (!children.empty ()) {
    f->dprintf ("containing subblocks\n");
    it.set_to_list (&children);
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
      it.data ()->show_attrs (f);
    f->dprintf ("end of subblocks\n");
  }
}


PAGE_BLOCK::PAGE_BLOCK (ICOORDELT_LIST * points, PB_TYPE type, PAGE_BLOCK_LIST * child):POLY_BLOCK (points,
POLY_PAGE) {
  PAGE_BLOCK_IT
    c = &children;

  pb_type = type;
  children.clear ();
  c.move_to_first ();
  c.add_list_before (child);
}


PAGE_BLOCK::PAGE_BLOCK (ICOORDELT_LIST * points, PB_TYPE type):POLY_BLOCK (points,
POLY_PAGE) {
  pb_type = type;
  children.clear ();
}


void PAGE_BLOCK::add_a_child(PAGE_BLOCK *newchild) {
  PAGE_BLOCK_IT c = &children;

  c.move_to_first ();
  c.add_to_end (newchild);
}


/**********************************************************************
 * PAGE_BLOCK::rotate
 *
 * Rotate the PAGE_BLOCK and its children
 **********************************************************************/

void PAGE_BLOCK::rotate(  //cos,sin
                        FCOORD rotation) {
                                 //sub block iterator
  PAGE_BLOCK_IT child_it = &children;
  PAGE_BLOCK *child;             //child block

  for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
  child_it.forward ()) {
    child = child_it.data ();
    child->rotate (rotation);
  }
  if (pb_type == PB_TEXT)
    ((TEXT_BLOCK *) this)->rotate (rotation);
  else
    POLY_BLOCK::rotate(rotation);
}


/**********************************************************************
 * PAGE_BLOCK::move
 *
 * Move the PAGE_BLOCK and its children
 **********************************************************************/

void PAGE_BLOCK::move(ICOORD shift  //amount to move
                     ) {
                                 //sub block iterator
  PAGE_BLOCK_IT child_it = &children;
  PAGE_BLOCK *child;             //child block

  for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
  child_it.forward ()) {
    child = child_it.data ();
    child->move (shift);
  }
  if (pb_type == PB_TEXT)
    ((TEXT_BLOCK *) this)->move (shift);
  else
    POLY_BLOCK::move(shift);
}

#ifndef GRAPHICS_DISABLED
void PAGE_BLOCK::basic_plot(ScrollView* window, ScrollView::Color colour) {
  PAGE_BLOCK_IT c = &children;

  POLY_BLOCK::plot (window, colour, 0);

  if (!c.empty ())
    for (c.mark_cycle_pt (); !c.cycled_list (); c.forward ())
      c.data ()->plot (window, colour);
}


void PAGE_BLOCK::plot(ScrollView* window, ScrollView::Color colour) {
  TEXT_BLOCK *tblock;
  WEIRD_BLOCK *wblock;

  switch (pb_type) {
    case PB_TEXT:
      basic_plot(window, colour);
      tblock = (TEXT_BLOCK *) this;
      tblock->plot (window, colour, REGION_COLOUR, SUBREGION_COLOUR);
      break;
    case PB_WEIRD:
      wblock = (WEIRD_BLOCK *) this;
      wblock->plot (window, colour);
      break;
    default:
      basic_plot(window, colour);
      break;
  }
}
#endif

void show_all_in(PAGE_BLOCK *pblock, POLY_BLOCK *show_area, DEBUG_WIN *f) {
  PAGE_BLOCK_IT c;
  inT16 i, pnum;

  c.set_to_list (pblock->child ());
  pnum = pblock->child ()->length ();
  for (i = 0; i < pnum; i++, c.forward ()) {
    if (show_area->contains (c.data ()))
      c.data ()->show_attrs (f);
    else if (show_area->overlap (c.data ()))
      show_all_in (c.data (), show_area, f);
  }
}


void delete_all_in(PAGE_BLOCK *pblock, POLY_BLOCK *delete_area) {
  PAGE_BLOCK_IT c;
  inT16 i, pnum;

  c.set_to_list (pblock->child ());
  pnum = pblock->child ()->length ();
  for (i = 0; i < pnum; i++, c.forward ()) {
    if (delete_area->contains (c.data ()))
      c.extract ()->pb_delete ();
    else if (delete_area->overlap (c.data ()))
      delete_all_in (c.data (), delete_area);
  }
}


PAGE_BLOCK *smallest_containing(PAGE_BLOCK *pblock, POLY_BLOCK *other) {
  PAGE_BLOCK_IT c;

  c.set_to_list (pblock->child ());
  if (c.empty ())
    return (pblock);

  for (c.mark_cycle_pt (); !c.cycled_list (); c.forward ())
    if (c.data ()->contains (other))
      return (smallest_containing (c.data (), other));

  return (pblock);
}


TEXT_BLOCK::TEXT_BLOCK (ICOORDELT_LIST * points, BOOL8 backg[NUM_BACKGROUNDS]):PAGE_BLOCK (points,
PB_TEXT) {
  int
    i;

  for (i = 0; i < NUM_BACKGROUNDS; i++)
    background.set_bit (i, backg[i]);

  text_regions.clear ();
}


void
TEXT_BLOCK::set_attrs (BOOL8 backg[NUM_BACKGROUNDS]) {
  int i;

  for (i = 0; i < NUM_BACKGROUNDS; i++)
    background.set_bit (i, backg[i]);
}


void TEXT_BLOCK::add_a_region(TEXT_REGION *newchild) {
  TEXT_REGION_IT c;

  c.set_to_list (&text_regions);

  c.move_to_first ();
  c.add_to_end (newchild);
}


/**********************************************************************
 * TEXT_BLOCK::rotate
 *
 * Rotate the TEXT_BLOCK and its children
 **********************************************************************/

void TEXT_BLOCK::rotate(  //cos,sin
                        FCOORD rotation) {
                                 //sub block iterator
  TEXT_REGION_IT child_it = &text_regions;
  TEXT_REGION *child;            //child block

  for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
  child_it.forward ()) {
    child = child_it.data ();
    child->rotate (rotation);
  }
  POLY_BLOCK::rotate(rotation);
}


/**********************************************************************
 * TEXT_BLOCK::move
 *
 * Move the TEXT_BLOCK and its children
 **********************************************************************/

void TEXT_BLOCK::move(ICOORD shift  //amount to move
                     ) {
                                 //sub block iterator
  TEXT_REGION_IT child_it = &text_regions;
  TEXT_REGION *child;            //child block

  for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
  child_it.forward ()) {
    child = child_it.data ();
    child->move (shift);
  }
  POLY_BLOCK::move(shift);
}


/**********************************************************************
 * TEXT_BLOCK::serialise_asc()  Convert to ascii file.
 *
 **********************************************************************/

void TEXT_BLOCK::serialise_asc(         //convert to ascii
                               FILE *f  //file to use
                              ) {
  ((PAGE_BLOCK *) this)->internal_serialise_asc (f);
  serialise_INT32 (f, background.val);
  text_regions.serialise_asc (f);
}


/**********************************************************************
 * TEXT_BLOCK::de_serialise_asc()  Convert from ascii file.
 *
 **********************************************************************/

void TEXT_BLOCK::de_serialise_asc(         //convert from ascii
                                  FILE *f  //file to use
                                 ) {
  ((PAGE_BLOCK *) this)->de_serialise_asc (f);
  background.val = de_serialise_INT32 (f);
  text_regions.de_serialise_asc (f);
}


#ifndef GRAPHICS_DISABLED
void TEXT_BLOCK::plot(ScrollView* window,
                      ScrollView::Color colour,
                      ScrollView::Color region_colour,
                      ScrollView::Color subregion_colour) {
  TEXT_REGION_IT t = &text_regions, tc;

  PAGE_BLOCK::basic_plot(window, colour);

  if (!t.empty ())
  for (t.mark_cycle_pt (); !t.cycled_list (); t.forward ()) {
    t.data ()->plot (window, region_colour, t.data ()->id_no ());
    tc.set_to_list (t.data ()->regions ());
    if (!tc.empty ())
      for (tc.mark_cycle_pt (); !tc.cycled_list (); tc.forward ())
        tc.data ()->plot (window, subregion_colour, -1);
  }
}
#endif


void TEXT_BLOCK::show_attrs(DEBUG_WIN *f) {
  TEXT_REGION_IT it;

  f->dprintf ("TEXT BLOCK\n");
  print_background(f, background);
  if (!text_regions.empty ()) {
    f->dprintf ("containing text regions:\n");
    it.set_to_list (&text_regions);
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
      it.data ()->show_attrs (f);
    f->dprintf ("end of regions\n");
  }
}


DLLSYM void show_all_tr_in(TEXT_BLOCK *tblock,
                           POLY_BLOCK *show_area,
                           DEBUG_WIN *f) {
  TEXT_REGION_IT t, tc;
  inT16 i, tnum, j, ttnum;

  t.set_to_list (tblock->regions ());
  tnum = tblock->regions ()->length ();
  for (i = 0; i < tnum; i++, t.forward ()) {
    if (show_area->contains (t.data ()))
      t.data ()->show_attrs (f);
    else if (show_area->overlap (t.data ())) {
      tc.set_to_list (t.data ()->regions ());
      ttnum = t.data ()->regions ()->length ();
      for (j = 0; j < ttnum; j++, tc.forward ())
        if (show_area->contains (tc.data ()))
          tc.data ()->show_attrs (f);
    }
  }
}


void delete_all_tr_in(TEXT_BLOCK *tblock, POLY_BLOCK *delete_area) {
  TEXT_REGION_IT t, tc;
  inT16 i, tnum, j, ttnum;

  t.set_to_list (tblock->regions ());
  tnum = tblock->regions ()->length ();
  for (i = 0; i < tnum; i++, t.forward ()) {
    if (delete_area->contains (t.data ()))
      delete (t.extract ());
    else if (delete_area->overlap (t.data ())) {
      tc.set_to_list (t.data ()->regions ());
      ttnum = t.data ()->regions ()->length ();
      for (j = 0; j < ttnum; j++, tc.forward ())
        if (delete_area->contains (tc.data ()))
          delete (tc.extract ());
    }
  }
}


RULE_BLOCK::RULE_BLOCK (ICOORDELT_LIST * points, inT8 sing, inT8 colo):PAGE_BLOCK (points,
PB_RULES) {
  multiplicity = sing;
  colour = colo;
}


void RULE_BLOCK::set_attrs(inT8 sing, inT8 colo) {
  multiplicity = sing;
  colour = colo;
}


void RULE_BLOCK::show_attrs(DEBUG_WIN *f) {
  f->dprintf ("RULE BLOCK with attributes %s, %s\n",
    blabel[R_START][multiplicity], blabel[R_START + 1][colour]);
}


/**********************************************************************
 * RULE_BLOCK::serialise_asc()  Convert to ascii file.
 *
 **********************************************************************/

void RULE_BLOCK::serialise_asc(         //convert to ascii
                               FILE *f  //file to use
                              ) {
  ((PAGE_BLOCK *) this)->internal_serialise_asc (f);
  serialise_INT32(f, multiplicity);
  serialise_INT32(f, colour);
}


/**********************************************************************
 * RULE_BLOCK::de_serialise_asc()  Convert from ascii file.
 *
 **********************************************************************/

void RULE_BLOCK::de_serialise_asc(         //convert from ascii
                                  FILE *f  //file to use
                                 ) {
  ((PAGE_BLOCK *) this)->de_serialise_asc (f);
  multiplicity = de_serialise_INT32 (f);
  colour = de_serialise_INT32 (f);
}


GRAPHICS_BLOCK::GRAPHICS_BLOCK (ICOORDELT_LIST * points, BOOL8 backg[NUM_BACKGROUNDS], inT8 foreg):PAGE_BLOCK (points,
PB_GRAPHICS) {
  int
    i;

  for (i = 0; i < NUM_BACKGROUNDS; i++)
    background.set_bit (i, backg[i]);

  foreground = foreg;
}


void
GRAPHICS_BLOCK::set_attrs (BOOL8 backg[NUM_BACKGROUNDS], inT8 foreg) {
  int i;

  for (i = 0; i < NUM_BACKGROUNDS; i++)
    background.set_bit (i, backg[i]);

  foreground = foreg;
}


void GRAPHICS_BLOCK::show_attrs(DEBUG_WIN *f) {
  f->dprintf ("GRAPHICS BLOCK with attribute %s\n",
    blabel[G_START][foreground]);
  print_background(f, background);
}


/**********************************************************************
 * GRAPHICS_BLOCK::serialise_asc()  Convert to ascii file.
 *
 **********************************************************************/

void GRAPHICS_BLOCK::serialise_asc(         //convert to ascii
                                   FILE *f  //file to use
                                  ) {
  ((PAGE_BLOCK *) this)->internal_serialise_asc (f);
  serialise_INT32 (f, background.val);
  serialise_INT32(f, foreground);
}


/**********************************************************************
 * GRAPHICS_BLOCK::de_serialise_asc()  Convert from ascii file.
 *
 **********************************************************************/

void GRAPHICS_BLOCK::de_serialise_asc(         //convert from ascii
                                      FILE *f  //file to use
                                     ) {
  ((PAGE_BLOCK *) this)->de_serialise_asc (f);
  background.val = de_serialise_INT32 (f);
  foreground = de_serialise_INT32 (f);
}


IMAGE_BLOCK::IMAGE_BLOCK (ICOORDELT_LIST * points, inT8 colo, inT8 qual):PAGE_BLOCK (points,
PB_IMAGE) {
  colour = colo;
  quality = qual;
}


void IMAGE_BLOCK::set_attrs(inT8 colo, inT8 qual) {
  colour = colo;
  quality = qual;
}


void IMAGE_BLOCK::show_attrs(DEBUG_WIN *f) {
  f->dprintf ("IMAGE BLOCK with attributes %s, %s\n", blabel[I_START][colour],
    blabel[I_START + 1][quality]);
}


/**********************************************************************
 * IMAGE_BLOCK::serialise_asc()  Convert to ascii file.
 *
 **********************************************************************/

void IMAGE_BLOCK::serialise_asc(         //convert to ascii
                                FILE *f  //file to use
                               ) {
  ((PAGE_BLOCK *) this)->internal_serialise_asc (f);
  serialise_INT32(f, colour);
  serialise_INT32(f, quality);
}


/**********************************************************************
 * IMAGE_BLOCK::de_serialise_asc()  Convert from ascii file.
 *
 **********************************************************************/

void IMAGE_BLOCK::de_serialise_asc(         //convert from ascii
                                   FILE *f  //file to use
                                  ) {
  ((PAGE_BLOCK *) this)->de_serialise_asc (f);
  colour = de_serialise_INT32 (f);
  quality = de_serialise_INT32 (f);
}


SCRIBBLE_BLOCK::SCRIBBLE_BLOCK (ICOORDELT_LIST * points, BOOL8 backg[NUM_BACKGROUNDS], inT8 foreg):PAGE_BLOCK (points,
PB_SCRIBBLE) {
  int
    i;

  for (i = 0; i < NUM_BACKGROUNDS; i++)
    background.set_bit (i, backg[i]);

  foreground = foreg;
}


void
SCRIBBLE_BLOCK::set_attrs (BOOL8 backg[NUM_BACKGROUNDS], inT8 foreg) {
  int i;

  for (i = 0; i < NUM_BACKGROUNDS; i++)
    background.set_bit (i, backg[i]);

  foreground = foreg;
}


void SCRIBBLE_BLOCK::show_attrs(DEBUG_WIN *f) {
  f->dprintf ("SCRIBBLE BLOCK with attributes %s\n",
    blabel[S_START][foreground]);
  print_background(f, background);
}


/**********************************************************************
 * SCRIBBLE_BLOCK::serialise_asc()  Convert to ascii file.
 *
 **********************************************************************/

void SCRIBBLE_BLOCK::serialise_asc(         //convert to ascii
                                   FILE *f  //file to use
                                  ) {
  ((PAGE_BLOCK *) this)->internal_serialise_asc (f);
  serialise_INT32 (f, background.val);
  serialise_INT32(f, foreground);
}


/**********************************************************************
 * SCRIBBLE_BLOCK::de_serialise_asc()  Convert from ascii file.
 *
 **********************************************************************/

void SCRIBBLE_BLOCK::de_serialise_asc(         //convert from ascii
                                      FILE *f  //file to use
                                     ) {
  ((PAGE_BLOCK *) this)->de_serialise_asc (f);
  background.val = de_serialise_INT32 (f);
  foreground = de_serialise_INT32 (f);
}


WEIRD_BLOCK::WEIRD_BLOCK (ICOORDELT_LIST * points, inT32 id_no):PAGE_BLOCK (points,
PB_WEIRD) {
  id_number = id_no;
}


#ifndef GRAPHICS_DISABLED
void WEIRD_BLOCK::plot(ScrollView* window, ScrollView::Color colour) {
  PAGE_BLOCK_IT c = this->child ();

  POLY_BLOCK::plot(window, colour, id_number);

  if (!c.empty ())
    for (c.mark_cycle_pt (); !c.cycled_list (); c.forward ())
      c.data ()->plot (window, colour);
}
#endif


void WEIRD_BLOCK::set_id(inT32 id_no) {
  id_number = id_no;
}


void WEIRD_BLOCK::show_attrs(DEBUG_WIN *f) {
  f->dprintf ("WEIRD BLOCK with id number %d\n", id_number);
}


/**********************************************************************
 * WEIRD_BLOCK::serialise_asc()  Convert to ascii file.
 *
 **********************************************************************/

void WEIRD_BLOCK::serialise_asc(         //convert to ascii
                                FILE *f  //file to use
                               ) {
  ((PAGE_BLOCK *) this)->internal_serialise_asc (f);
  serialise_INT32(f, id_number);
}


/**********************************************************************
 * WEIRD_BLOCK::de_serialise_asc()  Convert from ascii file.
 *
 **********************************************************************/

void WEIRD_BLOCK::de_serialise_asc(         //convert from ascii
                                   FILE *f  //file to use
                                  ) {
  ((PAGE_BLOCK *) this)->de_serialise_asc (f);
  id_number = de_serialise_INT32 (f);
}


void print_background(DEBUG_WIN *f, BITS16 background) {
  int i;

  f->dprintf ("Background is \n");
  for (i = 0; i < NUM_BACKGROUNDS; i++) {
    if (background.bit (i))
      f->dprintf ("%s, ", backlabel[i]);
  }

  f->dprintf ("\n");

}
