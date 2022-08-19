#include <application.h>

#define SERVICE_INTERVAL_INTERVAL (60 * 60 * 1000)
#define BATTERY_UPDATE_INTERVAL (60 * 60 * 1000)

#define TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL (15 * 60 * 1000)
#define TEMPERATURE_TAG_PUB_VALUE_CHANGE 0.2f
#define TEMPERATURE_UPDATE_SERVICE_INTERVAL (5 * 1000)
#define TEMPERATURE_UPDATE_NORMAL_INTERVAL (10 * 1000)

#define SET_TEMPERATURE_PUB_INTERVAL 15 * 60 * 1000;
#define SET_TEMPERATURE_ADD_ON_CLICK 0.5f

#define EEPROM_SET_TEMPERATURE_ADDRESS 0
#define APPLICATION_TASK_ID 0

#define COLOR_BLACK true

twr_led_t led;
twr_led_t led_lcd_red;
twr_led_t led_lcd_blue;

// Thermometer instance
twr_tmp112_t tmp112;
event_param_t temperature_event_param = { .next_pub = 0, .value = NAN };
event_param_t temperature_set_point;
float temperature_on_display = NAN;

#if ROTATE_SUPPORT
twr_lis2dh12_t lis2dh12;
twr_dice_t dice;
twr_dice_face_t face = TWR_DICE_FACE_UNKNOWN;
twr_module_lcd_rotation_t rotation = TWR_MODULE_LCD_ROTATION_0;
#endif

#if CORE_R == 2
twr_module_lcd_rotation_t face_2_lcd_rotation_lut[7] =
{
    [TWR_DICE_FACE_2] = TWR_MODULE_LCD_ROTATION_270,
    [TWR_DICE_FACE_3] = TWR_MODULE_LCD_ROTATION_180,
    [TWR_DICE_FACE_4] = TWR_MODULE_LCD_ROTATION_0,
    [TWR_DICE_FACE_5] = TWR_MODULE_LCD_ROTATION_90
};
#else
twr_module_lcd_rotation_t face_2_lcd_rotation_lut[7] =
{
    [TWR_DICE_FACE_2] = TWR_MODULE_LCD_ROTATION_90,
    [TWR_DICE_FACE_3] = TWR_MODULE_LCD_ROTATION_0,
    [TWR_DICE_FACE_4] = TWR_MODULE_LCD_ROTATION_180,
    [TWR_DICE_FACE_5] = TWR_MODULE_LCD_ROTATION_270
};
#endif

void radio_pub_set_temperature(void)
{
    temperature_set_point.next_pub = twr_scheduler_get_spin_tick() + SET_TEMPERATURE_PUB_INTERVAL;

    twr_radio_pub_temperature(TWR_RADIO_PUB_CHANNEL_SET_POINT, &temperature_set_point.value);
}

void tmp112_event_handler(twr_tmp112_t *self, twr_tmp112_event_t event, void *event_param)
{
    float value;
    event_param_t *param = (event_param_t *)event_param;

    if (event != TWR_TMP112_EVENT_UPDATE)
    {
        return;
    }

    if (twr_tmp112_get_temperature_celsius(self, &value))
    {
        if ((fabsf(value - param->value) >= TEMPERATURE_TAG_PUB_VALUE_CHANGE) || (param->next_pub < twr_scheduler_get_spin_tick()))
        {
            twr_radio_pub_temperature(TWR_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_ALTERNATE, &value);
            param->value = value;
            param->next_pub = twr_scheduler_get_spin_tick() + TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL;
        }
    }
    else
    {
        param->value = NAN;
    }

    if (temperature_set_point.next_pub < twr_scheduler_get_spin_tick())
    {
        radio_pub_set_temperature();
    }

    if ((fabsf(param->value - temperature_on_display) >= 0.1) || isnan(temperature_on_display))
    {
        twr_scheduler_plan_now(APPLICATION_TASK_ID);
    }
}

void on_lcd_button_click(void)
{
    radio_pub_set_temperature();

    // Save set temperature to eeprom
    uint32_t neg_set_temperature;
    float *set_temperature = (float *) &neg_set_temperature;

    *set_temperature = temperature_set_point.value;

    neg_set_temperature = ~neg_set_temperature;

    twr_eeprom_write(EEPROM_SET_TEMPERATURE_ADDRESS, &temperature_set_point.value, sizeof(temperature_set_point.value));
    twr_eeprom_write(EEPROM_SET_TEMPERATURE_ADDRESS + sizeof(temperature_set_point.value), &neg_set_temperature, sizeof(neg_set_temperature));

    twr_scheduler_plan_now(APPLICATION_TASK_ID);
}

void lcd_button_left_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    if (event == TWR_BUTTON_EVENT_CLICK)
	{

        temperature_set_point.value -= SET_TEMPERATURE_ADD_ON_CLICK;

        static uint16_t left_event_count = 0;

        left_event_count++;

        twr_radio_pub_event_count(TWR_RADIO_PUB_EVENT_LCD_BUTTON_LEFT, &left_event_count);

        twr_led_pulse(&led_lcd_blue, 30);

        on_lcd_button_click();
    }
}

void lcd_button_right_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) event_param;

	if (event == TWR_BUTTON_EVENT_CLICK)
	{

        temperature_set_point.value += SET_TEMPERATURE_ADD_ON_CLICK;

        static uint16_t right_event_count = 0;

        right_event_count++;

        twr_radio_pub_event_count(TWR_RADIO_PUB_EVENT_LCD_BUTTON_RIGHT, &right_event_count);

        twr_led_pulse(&led_lcd_red, 30);

        on_lcd_button_click();
    }
}

#if ROTATE_SUPPORT
void lis2dh12_event_handler(twr_lis2dh12_t *self, twr_lis2dh12_event_t event, void *event_param)
{
    (void) event_param;

    if (event == TWR_LIS2DH12_EVENT_UPDATE)
    {
        twr_lis2dh12_result_g_t result;

        twr_lis2dh12_get_result_g(self, &result);

        twr_dice_feed_vectors(&dice, result.x_axis, result.y_axis, result.z_axis);

        face = twr_dice_get_face(&dice);

        if (face > TWR_DICE_FACE_1 && face < TWR_DICE_FACE_6)
        {
            rotation = face_2_lcd_rotation_lut[face];

            twr_scheduler_plan_now(APPLICATION_TASK_ID);
        }
    }
}
#endif

void battery_event_handler(twr_module_battery_event_t event, void *event_param)
{
    (void) event_param;

    float voltage;

    if (event == TWR_MODULE_BATTERY_EVENT_UPDATE)
    {
        if (twr_module_battery_get_voltage(&voltage))
        {
            twr_radio_pub_battery(&voltage);
        }
    }
}

void switch_to_normal_mode_task(void *param)
{
    twr_tmp112_set_update_interval(&tmp112, TEMPERATURE_UPDATE_NORMAL_INTERVAL);

    twr_scheduler_unregister(twr_scheduler_get_current_task_id());
}

void application_init(void)
{
    // Initialize LED on core module
    twr_led_init(&led, TWR_GPIO_LED, false, false);
    twr_led_set_mode(&led, TWR_LED_MODE_OFF);

    // Load set temperature from eeprom
    uint32_t neg_set_temperature;
    float *set_temperature = (float *) &neg_set_temperature;

    twr_eeprom_read(EEPROM_SET_TEMPERATURE_ADDRESS, &temperature_set_point.value, sizeof(temperature_set_point.value));
    twr_eeprom_read(EEPROM_SET_TEMPERATURE_ADDRESS + sizeof(temperature_set_point.value), &neg_set_temperature, sizeof(neg_set_temperature));

    neg_set_temperature = ~neg_set_temperature;

    if (temperature_set_point.value != *set_temperature)
    {
        temperature_set_point.value = 21.0f;
    }

    // Initialize Radio
    twr_radio_init(TWR_RADIO_MODE_NODE_SLEEPING);

    // Initialize battery
    twr_module_battery_init();
    twr_module_battery_set_event_handler(battery_event_handler, NULL);
    twr_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);

    // Initialize thermometer sensor on core module
    twr_tmp112_init(&tmp112, TWR_I2C_I2C0, 0x49);
    twr_tmp112_set_event_handler(&tmp112, tmp112_event_handler, &temperature_event_param);
    twr_tmp112_set_update_interval(&tmp112, TEMPERATURE_UPDATE_SERVICE_INTERVAL);

    // Initialize LCD
    twr_module_lcd_init();

    // Initialize LCD button left
    static twr_button_t lcd_left;
    twr_button_init_virtual(&lcd_left, TWR_MODULE_LCD_BUTTON_LEFT, twr_module_lcd_get_button_driver(), false);
    twr_button_set_event_handler(&lcd_left, lcd_button_left_event_handler, NULL);

    // Initialize LCD button right
    static twr_button_t lcd_right;
    twr_button_init_virtual(&lcd_right, TWR_MODULE_LCD_BUTTON_RIGHT, twr_module_lcd_get_button_driver(), false);
    twr_button_set_event_handler(&lcd_right, lcd_button_right_event_handler, NULL);

    // Initialize red and blue LED on LCD module
    twr_led_init_virtual(&led_lcd_red, TWR_MODULE_LCD_LED_RED, twr_module_lcd_get_led_driver(), true);
    twr_led_init_virtual(&led_lcd_blue, TWR_MODULE_LCD_LED_BLUE, twr_module_lcd_get_led_driver(), true);

#if ROTATE_SUPPORT
    // Initialize Accelerometer
    twr_dice_init(&dice, TWR_DICE_FACE_UNKNOWN);
    twr_lis2dh12_init(&lis2dh12, TWR_I2C_I2C0, 0x19);
    twr_lis2dh12_set_update_interval(&lis2dh12, 5 * 1000);
    twr_lis2dh12_set_event_handler(&lis2dh12, lis2dh12_event_handler, NULL);
#endif

    twr_radio_pairing_request("lcd-thermostat", FW_VERSION);

    twr_scheduler_register(switch_to_normal_mode_task, NULL, SERVICE_INTERVAL_INTERVAL);

    twr_led_pulse(&led, 2000);
}

void application_task(void)
{
    static char str_temperature[10];

    if (!twr_module_lcd_is_ready())
    {
    	return;
    }

    twr_system_pll_enable();

#if ROTATE_SUPPORT
    twr_module_lcd_set_rotation(rotation);
#endif

    twr_module_lcd_clear();

    twr_module_lcd_set_font(&twr_font_ubuntu_33);
    snprintf(str_temperature, sizeof(str_temperature), "%.1f   ", temperature_event_param.value);
    int x = twr_module_lcd_draw_string(20, 20, str_temperature, COLOR_BLACK);
    temperature_on_display = temperature_event_param.value;

    twr_module_lcd_set_font(&twr_font_ubuntu_24);
    twr_module_lcd_draw_string(x - 20, 25, "\xb0" "C   ", COLOR_BLACK);

    twr_module_lcd_set_font(&twr_font_ubuntu_15);
    twr_module_lcd_draw_string(10, 80, "Set temperature", COLOR_BLACK);

    snprintf(str_temperature, sizeof(str_temperature), "%.1f \xb0" "C", temperature_set_point.value);
    twr_module_lcd_draw_string(40, 100, str_temperature, COLOR_BLACK);

    twr_module_lcd_update();

    twr_system_pll_disable();
}
