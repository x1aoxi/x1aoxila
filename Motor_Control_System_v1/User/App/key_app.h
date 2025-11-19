#ifndef __KEY_APP_H__
#define __KEY_APP_H__

#include "MyDefine.h"
#include "ui_menu_app.h"  // 引入UI菜单系统

void Key_Init(void);
void Key_Task(void);
void my_handle_key_event(struct ebtn_btn *btn, ebtn_evt_t evt);

// 将Ebtn事件转换为UI按键事件
KeyEvent Key_ConvertToUIEvent(uint16_t key_id, ebtn_evt_t evt);

#endif
