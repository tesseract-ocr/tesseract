/*************************************************************************
 * The following functions are the standard external calls that pgeditor makes
 * to its client programs.  They are included explicitly here to avoid making
 * pgeditor.c dependent on any/all of its clients
 ***************************************************************************/

                                 //handle for "MODES"
void extend_menu(RADIO_MENU *modes_menu,
                 INT16 modes_id_base,         //mode cmd ids offset
                 NON_RADIO_MENU *other_menu,  //handle for "OTHER"
                 INT16 other_id_base          //mode cmd ids offset
                );
                                 //current mode
void extend_moded_commands(INT32 mode,
                           BOX selection_box  //area selected
                          );
                                 //current mode
void extend_unmoded_commands(INT32 cmd_event,
                             char *new_value  //changed value if any
                            );
