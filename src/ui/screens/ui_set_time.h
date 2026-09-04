#ifndef UI_SET_TIME_H
#define UI_SET_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

// SCREEN: ui_time_picker  (Set Time — Week / Hour / Minute rollers)
extern void ui_time_picker_screen_init(void);
extern void ui_time_picker_screen_destroy(void);

extern lv_obj_t * ui_time_picker;
extern lv_obj_t * ui_time_Roller_W;   /* Weekday (SUN-SAT, index 0-6)  */
extern lv_obj_t * ui_time_Roller_H;   /* Hour    (00-23,   index 0-23) */
extern lv_obj_t * ui_time_Roller_M;   /* Minute  (00-59,   index 0-59) */

/* Called from launcher handlePhysicalNav() on A / B button press */
void ui_time_picker_action_save(void);
void ui_time_picker_action_cancel(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_SET_TIME_H */
