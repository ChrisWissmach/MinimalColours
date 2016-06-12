#include <pebble.h>
#include <stdlib.h>

//init static vars
static Window *s_main_window;
static TextLayer * s_time_layer;
static Layer *s_battery_layer;
static GFont s_time_font;
GColor *colours;
bool isYellow;
static int s_battery_level;
GColor current;




static void update_background_colour(){
  int x = rand() % 7;
  current = colours[x];
  
  if (x == 2){
    isYellow = true;
    text_layer_set_text_color(s_time_layer, GColorBlack);
  }
  else{
    isYellow = false;
  }
  
  window_set_background_color(s_main_window, current);
}

static void battery_callback(BatteryChargeState state){
  s_battery_level = state.charge_percent;
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * 114.0F);

  // Draw the background
  graphics_context_set_fill_color(ctx, current);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, isYellow ? GColorBlack : GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
  
  
}

//update the time
static void update_time(){
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M" , tick_time);
  
  text_layer_set_text(s_time_layer, s_buffer);

}

//update time event
static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time();
  update_background_colour();
}

//load all layers
static void main_window_load(Window *window){
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  s_time_layer = text_layer_create( GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 100));
  
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_THIN_50));
  
  s_battery_layer = layer_create(GRect(0, 0, 144,3));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_mark_dirty(s_battery_layer);
  
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), s_battery_layer);

}

//unload memory
static void main_window_unload(Window *window){
  text_layer_destroy(s_time_layer);
  fonts_unload_custom_font(s_time_font);
}


static void init(){
  
  colours = malloc(sizeof(GColor) * 7);
  colours[0] =  GColorRed;
  colours[1] =  GColorOrange;
  colours[2] =  GColorYellow;
  colours[3] =  GColorGreen;
  colours[4] =  GColorBlue;
  colours[5] =  GColorIndigo;
  colours[6] =  GColorPurple;
  
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  battery_callback(battery_state_service_peek());
  update_time();
}

static void deinit(){
  window_destroy(s_main_window);
  layer_destroy(s_battery_layer);
}


int main(){
  init();
  app_event_loop();
  deinit();
}

