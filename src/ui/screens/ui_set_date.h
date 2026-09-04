#ifndef UI_SET_DATE_H
#define UI_SET_DATE_H

#ifdef __cplusplus
extern "C" {
#endif

// SCREEN: ui_date_picker  (Set Date — Month / Day / Year rollers)
extern void ui_date_picker_screen_init(void);
extern void ui_date_picker_screen_destroy(void);

extern lv_obj_t * ui_date_picker;
extern lv_obj_t * ui_date_Roller_M;   /* Month roller  (JAN-DEC, index 0-11) */
extern lv_obj_t * ui_date_Roller_D;   /* Day roller    (01-31,   index 0-30) */
extern lv_obj_t * ui_date_Roller_Y;   /* Year roller   (2024-2035, index 0-11) */

/* Called from launcher handlePhysicalNav() on A / B button press */
void ui_date_picker_action_save(void);
void ui_date_picker_action_cancel(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_SET_DATE_H */
