#include <application.h>
#include <bc_eeprom.h>
#include <bc_spi.h>

#define TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL (5 * 60 * 1000)
#define TEMPERATURE_TAG_PUB_VALUE_CHANGE 0.2f
#define TEMPERATURE_TAG_UPDATE_INTERVAL (1 * 1000)

#define SET_TEMPERATURE_PUB_INTERVAL 5 * 60 * 1000;
#define SET_TEMPERATURE_ADD_ON_CLICK 0.5f

#define EEPROM_SET_TEMPERATURE_ADDRESS (sizeof(uint64_t) * (BC_RADIO_MAX_DEVICES + 1))
#define APPLICATION_TASK_ID 0

#define RADIO_THERMOSTAT_SET_POINT_TEMPERATURE 0xf0
#define RADIO_LCD_BUTTON_LEFT		0x20
#define RADIO_LCD_BUTTON_RIGHT		0x21

#define COLOR_BLACK true

static bc_led_t led;

static bc_led_t lcd_led_red;
static bc_led_t lcd_led_blue;


static bc_tag_temperature_t temperature_core;
static event_param_t temperature_core_event_param = {
        .number =  (BC_I2C_I2C0 << 7) | BC_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE
};
static float temperature = NAN;

static event_param_t set_temperature;

static bc_module_lcd_rotation_t rotation;
static float temperature_disp;

static bc_lis2dh12_t lis2dh12;
static bc_lis2dh12_result_g_t result;

static void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param);
static void lis2dh12_event_handler(bc_lis2dh12_t *self, bc_lis2dh12_event_t event, void *event_param);
static void lcd_button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);
static void radio_pub_u16(uint8_t type, uint16_t value);
static void radio_pub_set_temperature(void);

void application_init(void)
{
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_ON);

    uint32_t neg_set_temperature;
    bc_eeprom_read(EEPROM_SET_TEMPERATURE_ADDRESS, &set_temperature.value, sizeof(set_temperature.value));
    bc_eeprom_read(EEPROM_SET_TEMPERATURE_ADDRESS + sizeof(set_temperature.value), &neg_set_temperature, sizeof(neg_set_temperature));

    neg_set_temperature = ~neg_set_temperature;

    if (set_temperature.value != *(float *)(&neg_set_temperature))
    {
        set_temperature.value = 21.0f;
    }

    bc_radio_init();

    bc_tag_temperature_init(&temperature_core, BC_I2C_I2C0, BC_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE);

    bc_tag_temperature_set_update_interval(&temperature_core, TEMPERATURE_TAG_UPDATE_INTERVAL);

    bc_tag_temperature_set_event_handler(&temperature_core, temperature_tag_event_handler, &temperature_core_event_param);

    bc_module_lcd_init(&_bc_module_lcd_framebuffer);

    static bc_button_t lcd_left;
    bc_button_init_virtual(&lcd_left, BC_MODULE_LCD_BUTTON_LEFT, bc_module_lcd_get_button_driver(), false);
    bc_button_set_event_handler(&lcd_left, lcd_button_event_handler, NULL);

    static bc_button_t lcd_right;
    bc_button_init_virtual(&lcd_right, BC_MODULE_LCD_BUTTON_RIGHT, bc_module_lcd_get_button_driver(), false);
    bc_button_set_event_handler(&lcd_right, lcd_button_event_handler, NULL);

    bc_lis2dh12_init(&lis2dh12, BC_I2C_I2C0, 0x19);
    bc_lis2dh12_set_update_interval(&lis2dh12, 100);
    bc_lis2dh12_set_event_handler(&lis2dh12, lis2dh12_event_handler, NULL);


    bc_led_init_virtual(&lcd_led_red, BC_MODULE_LCD_LED_RED, bc_module_lcd_get_led_driver(), true);
    bc_led_init_virtual(&lcd_led_blue, BC_MODULE_LCD_LED_BLUE, bc_module_lcd_get_led_driver(), true);

    bc_led_set_mode(&led, BC_LED_MODE_OFF);
}

void application_task(void)
{
    static char str_temperature[10];

    if (!bc_module_lcd_is_ready())
    {
    	return;
    }

    bc_module_core_pll_enable();

    bc_module_lcd_set_rotation(rotation);

    bc_module_lcd_clear();

    bc_module_lcd_set_font(&bc_font_ubuntu_33);
    snprintf(str_temperature, sizeof(str_temperature), "%.1f   ", temperature);
    int x = bc_module_lcd_draw_string(20, 20, str_temperature, COLOR_BLACK);
    temperature_disp = temperature;

    bc_module_lcd_set_font(&bc_font_ubuntu_24);
    bc_module_lcd_draw_string(x - 20, 25, "\xb0" "C   ", COLOR_BLACK);

    bc_module_lcd_set_font(&bc_font_ubuntu_15);
    bc_module_lcd_draw_string(10, 80, "Set temperature", COLOR_BLACK);

    snprintf(str_temperature, sizeof(str_temperature), "%.1f \xb0" "C", set_temperature.value);
    bc_module_lcd_draw_string(40, 100, str_temperature, COLOR_BLACK);

    bc_module_lcd_update();

    bc_module_core_pll_disable();
}

void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param)
{
    float value;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_TEMPERATURE_EVENT_UPDATE)
    {
        return;
    }

    if (bc_tag_temperature_get_temperature_celsius(self, &value))
    {
        if ((fabs(value - param->value) >= TEMPERATURE_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick()))
        {
            bc_radio_pub_thermometer(param->number, &value);
            param->value = value;
            param->next_pub = bc_scheduler_get_spin_tick() + TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL;

            temperature = value;
            bc_scheduler_plan_now(0);
        }
    }
    else
    {
        temperature = NAN;
    }

    if (temperature != temperature_disp)
    {
        bc_scheduler_plan_now(APPLICATION_TASK_ID);
    }

    if (set_temperature.next_pub < bc_scheduler_get_spin_tick())
    {
        radio_pub_set_temperature();
    }
}

static void lcd_button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) event_param;

	if (event == BC_BUTTON_EVENT_CLICK)
	{
        if (self->_channel.virtual_channel == BC_MODULE_LCD_BUTTON_LEFT)
        {
            set_temperature.value -= SET_TEMPERATURE_ADD_ON_CLICK;

            static uint16_t left_event_count = 0;
            radio_pub_u16(RADIO_LCD_BUTTON_LEFT, left_event_count++);

            bc_led_pulse(&lcd_led_blue, 30);
        }
        else
        {
            set_temperature.value += SET_TEMPERATURE_ADD_ON_CLICK;

            static uint16_t right_event_count = 0;
            radio_pub_u16(RADIO_LCD_BUTTON_RIGHT, right_event_count++);

            bc_led_pulse(&lcd_led_red, 30);
        }

        radio_pub_set_temperature();

        uint32_t neg_set_temperature = ~(*(uint32_t *)&set_temperature.value);
        bc_eeprom_write(EEPROM_SET_TEMPERATURE_ADDRESS, &set_temperature.value, sizeof(set_temperature.value));
        bc_eeprom_write(EEPROM_SET_TEMPERATURE_ADDRESS + sizeof(set_temperature.value), &neg_set_temperature, sizeof(neg_set_temperature));

        bc_scheduler_plan_now(APPLICATION_TASK_ID);
	}
	else if(event == BC_BUTTON_EVENT_HOLD)
    {
        bc_radio_enroll_to_gateway();
    }
}

static void radio_pub_set_temperature(void)
{
    uint8_t buffer[1 + sizeof(set_temperature.value)];
    set_temperature.next_pub = bc_scheduler_get_spin_tick() + SET_TEMPERATURE_PUB_INTERVAL;
    buffer[0] = RADIO_THERMOSTAT_SET_POINT_TEMPERATURE;
    memcpy(buffer + 1, &set_temperature.value, sizeof(set_temperature.value));
    bc_radio_pub_buffer(buffer, sizeof(buffer));
}

static void radio_pub_u16(uint8_t type, uint16_t value)
{
    uint8_t buffer[1 + sizeof(value)];
    buffer[0] = type;
    memcpy(buffer + 1, &value, sizeof(value));
    bc_radio_pub_buffer(buffer, sizeof(buffer));
}

static void lis2dh12_event_handler(bc_lis2dh12_t *self, bc_lis2dh12_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_LIS2DH12_EVENT_UPDATE)
    {
        bc_lis2dh12_get_result_g(self, &result);

        if ((result.z_axis > 0) && result.z_axis < 0.90)
        {

            if (fabs(result.x_axis) > fabs(result.y_axis))
            {
                if (result.x_axis > 0)
                {
                    rotation = BC_MODULE_LCD_ROTATION_90;
                }
                else
                {
                    rotation = BC_MODULE_LCD_ROTATION_270;
                }
            }
            else
            {
                if (result.y_axis > 0)
                {
                    rotation = BC_MODULE_LCD_ROTATION_0;
                }
                else
                {
                    rotation = BC_MODULE_LCD_ROTATION_180;
                }
            }

            if (rotation != bc_module_lcd_get_rotation())
            {
                bc_scheduler_plan_now(APPLICATION_TASK_ID);
            }
        }
    }
}


