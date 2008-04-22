#ifndef           PAGEBLK_C
#define           PAGEBLK_C

#include "elst.h"
#include "txtregn.h"
#include "bits16.h"

#include          "hpddef.h"     //must be last (handpd.dll)

enum PB_TYPE
{
  PB_TEXT,
  PB_RULES,
  PB_GRAPHICS,
  PB_IMAGE,
  PB_SCRIBBLE,
  PB_WEIRD
};

class DLLSYM PAGE_BLOCK;         //forward decl
class DLLSYM TEXT_BLOCK;         //forward decl
class DLLSYM GRAPHICS_BLOCK;     //forward decl
class DLLSYM RULE_BLOCK;         //forward decl
class DLLSYM IMAGE_BLOCK;        //forward decl
class DLLSYM SCRIBBLE_BLOCK;     //forward decl
class DLLSYM WEIRD_BLOCK;        //forward decl

ELISTIZEH_S (PAGE_BLOCK)
class DLLSYM PAGE_BLOCK:public ELIST_LINK, public POLY_BLOCK
//page block
{
  public:
    PAGE_BLOCK() {
    }                            //empty constructor
    PAGE_BLOCK(  //simple constructor
               ICOORDELT_LIST *points,
               PB_TYPE type,
               PAGE_BLOCK_LIST *child);

    PAGE_BLOCK(  //simple constructor
               ICOORDELT_LIST *points,
               PB_TYPE type);

    ~PAGE_BLOCK () {             //destructor
    }

    void add_a_child(PAGE_BLOCK *newchild);

    PB_TYPE type() {  //get type
      return pb_type;
    }

    PAGE_BLOCK_LIST *child() {  //get children
      return &children;
    }

    void rotate(  //rotate it
                FCOORD rotation);
    void move(                //move it
              ICOORD shift);  //vector

    void basic_plot(ScrollView* window, ScrollView::Color colour);

    void plot(ScrollView* window, ScrollView::Color colour);

    void show_attrs(DEBUG_WIN *debug);

    NEWDELETE2 (PAGE_BLOCK) void pb_delete ();

    void serialise(FILE *f);

    static PAGE_BLOCK *de_serialise(FILE *f);

    void prep_serialise() {  //set ptrs to counts
      POLY_BLOCK::prep_serialise();
      children.prep_serialise ();
    }

    void dump(  //write external bits
              FILE *f) {
      POLY_BLOCK::dump(f);
      children.dump (f);
    }

    void de_dump(  //read external bits
                 FILE *f) {
      POLY_BLOCK::de_dump(f);
      children.de_dump (f);
    }

    //note that due to the awful switched nature of the PAGE_BLOCK class,
    //a PAGE_BLOCK_LIST cannot be de-serialised by the normal mechanism, since
    //each element cannot be de-serialised in place.
    //To fix this it is important to use read_poly_blocks or the code therein.
    void serialise_asc(  //serialise to ascii
                       FILE *f);
    void internal_serialise_asc(  //serialise to ascii
                                FILE *f);
    void de_serialise_asc(  //serialise from ascii
                          FILE *f);
                                 //make one from ascii
    static PAGE_BLOCK *new_de_serialise_asc(FILE *f);

  private:
    PB_TYPE pb_type;
    PAGE_BLOCK_LIST children;
};

DLLSYM void show_all_in(PAGE_BLOCK *pblock,
                        POLY_BLOCK *show_area,
                        DEBUG_WIN *f);

DLLSYM void delete_all_in(PAGE_BLOCK *pblock, POLY_BLOCK *delete_area);

DLLSYM PAGE_BLOCK *smallest_containing(PAGE_BLOCK *pblock, POLY_BLOCK *other);

class DLLSYM TEXT_BLOCK:public PAGE_BLOCK
                                 //text block
{
  public:
    TEXT_BLOCK() {
    }                            //empty constructor
    TEXT_BLOCK(ICOORDELT_LIST *points);

    TEXT_BLOCK (ICOORDELT_LIST * points, BOOL8 backg[NUM_BACKGROUNDS]);

                                 //get children
    TEXT_REGION_LIST *regions() {
      return &text_regions;
    }

    inT32 nregions() {
      return text_regions.length ();
    }

    void add_a_region(TEXT_REGION *newchild);

    void rotate(  //rotate it
                FCOORD rotation);
    void move(                //move it
              ICOORD shift);  //vector

    void plot(ScrollView* window,
              ScrollView::Color colour,
              ScrollView::Color region_colour,
              ScrollView::Color subregion_colour);

    void set_attrs (BOOL8 backg[NUM_BACKGROUNDS]);

    void show_attrs(DEBUG_WIN *debug);

    void prep_serialise() {  //set ptrs to counts
      PAGE_BLOCK::prep_serialise();
      text_regions.prep_serialise ();
    }

    void dump(  //write external bits
              FILE *f) {
      PAGE_BLOCK::dump(f);
      text_regions.dump (f);
    }

    void de_dump(  //read external bits
                 FILE *f) {
      PAGE_BLOCK::de_dump(f);
      text_regions.de_dump (f);
    }

                                 //serialise to ascii
    make_serialise (TEXT_BLOCK) void serialise_asc (
      FILE * f);
    void de_serialise_asc(  //serialise from ascii
                          FILE *f);

  private:
    BITS16 background;

    TEXT_REGION_LIST text_regions;
};

DLLSYM void delete_all_tr_in(TEXT_BLOCK *tblock, POLY_BLOCK *delete_area);

DLLSYM void show_all_tr_in(TEXT_BLOCK *tblock,
                           POLY_BLOCK *show_area,
                           DEBUG_WIN *f);

class DLLSYM RULE_BLOCK:public PAGE_BLOCK
                                 //rule block
{
  public:
    RULE_BLOCK() {
    }                            //empty constructor
    RULE_BLOCK(ICOORDELT_LIST *points, inT8 sing, inT8 colo);

    void set_attrs(inT8 sing, inT8 colo);

    void show_attrs(DEBUG_WIN *debug);

                                 //serialise to ascii
    make_serialise (RULE_BLOCK) void serialise_asc (
      FILE * f);
    void de_serialise_asc(  //serialise from ascii
                          FILE *f);

  private:
    inT8 multiplicity;
    inT8 colour;

};

class DLLSYM GRAPHICS_BLOCK:public PAGE_BLOCK
                                 //graphics block
{
  public:
    GRAPHICS_BLOCK() {
    }                            //empty constructor
    GRAPHICS_BLOCK (ICOORDELT_LIST * points,
      BOOL8 backg[NUM_BACKGROUNDS], inT8 foreg);

    void set_attrs (BOOL8 backg[NUM_BACKGROUNDS], inT8 foreg);

    void show_attrs(DEBUG_WIN *debug);

                                 //serialise to ascii
    make_serialise (GRAPHICS_BLOCK) void serialise_asc (
      FILE * f);
    void de_serialise_asc(  //serialise from ascii
                          FILE *f);

  private:
    BITS16 background;
    inT8 foreground;

};

class DLLSYM IMAGE_BLOCK:public PAGE_BLOCK
                                 //image block
{
  public:
    IMAGE_BLOCK() {
    }                            //empty constructor
    IMAGE_BLOCK(ICOORDELT_LIST *points, inT8 colo, inT8 qual);

    void set_attrs(inT8 colo, inT8 qual);

    void show_attrs(DEBUG_WIN *debug);

                                 //serialise to ascii
    make_serialise (IMAGE_BLOCK) void serialise_asc (
      FILE * f);
    void de_serialise_asc(  //serialise from ascii
                          FILE *f);

  private:
    inT8 colour;
    inT8 quality;

};

class DLLSYM SCRIBBLE_BLOCK:public PAGE_BLOCK
                                 //scribble block
{
  public:
    SCRIBBLE_BLOCK() {
    }                            //empty constructor
    SCRIBBLE_BLOCK (ICOORDELT_LIST * points,
      BOOL8 backg[NUM_BACKGROUNDS], inT8 foreg);

    void set_attrs (BOOL8 backg[NUM_BACKGROUNDS], inT8 foreg);

    void show_attrs(DEBUG_WIN *debug);

                                 //serialise to ascii
    make_serialise (SCRIBBLE_BLOCK) void serialise_asc (
      FILE * f);
    void de_serialise_asc(  //serialise from ascii
                          FILE *f);

  private:
    BITS16 background;
    inT8 foreground;
};

class DLLSYM WEIRD_BLOCK:public PAGE_BLOCK
                                 //weird block
{
  public:
    WEIRD_BLOCK() {
    }                            //empty constructor
    WEIRD_BLOCK(ICOORDELT_LIST *points, inT32 id_no);

    void set_id(inT32 id_no);

    void show_attrs(DEBUG_WIN *debug);

    void set_id_no(inT32 new_id) {
      id_number = new_id;
    }

    void plot(ScrollView* window, ScrollView::Color colour);

    inT32 id_no() {
      return id_number;
    }

                                 //serialise to ascii
    make_serialise (WEIRD_BLOCK) void serialise_asc (
      FILE * f);
    void de_serialise_asc(  //serialise from ascii
                          FILE *f);

  private:
    inT32 id_number;             //unique id

};

void print_background(DEBUG_WIN *f, BITS16 background);
#endif
