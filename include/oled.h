#ifndef _OLED_PADDING_H_
#define _OLED_PADDING_H_

/**
 * Initialize oled and show splash screen
 */
int initScreen();

/**
 * Show debug info
 */
void refreshInfoScreen(boolean mqtt_state, IPAddress ip, uint32_t free_heap, unsigned int msg_counter);

#endif