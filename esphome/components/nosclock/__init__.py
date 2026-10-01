import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, light, output, time
from esphome.const import CONF_ID, CONF_TIME_ID, CONF_I2C_ID

CONF_TUBES_LIGHT = "tubes_light"
CONF_BACKLIGHT_STRIP = "backlight_strip"
CONF_TUBES_EN = "tubes_en"

nosclock_ns = cg.esphome_ns.namespace("nosclock")
NosController = nosclock_ns.class_("NosController", cg.PollingComponent)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(NosController),
            cv.Required(CONF_I2C_ID): cv.use_id(i2c.I2CBus),
            cv.Required(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
            cv.Required(CONF_TUBES_LIGHT): cv.use_id(light.LightState),
            cv.Required(CONF_BACKLIGHT_STRIP): cv.use_id(light.LightState),
            cv.Required(CONF_TUBES_EN): cv.use_id(output.FloatOutput),
        }
    )
    .extend(cv.polling_component_schema("20ms"))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    bus = await cg.get_variable(config[CONF_I2C_ID])
    cg.add(var.set_i2c_bus(bus))

    time_comp = await cg.get_variable(config[CONF_TIME_ID])
    cg.add(var.set_time(time_comp))

    tubes_light = await cg.get_variable(config[CONF_TUBES_LIGHT])
    cg.add(var.set_tubes_light(tubes_light))

    backlight_strip = await cg.get_variable(config[CONF_BACKLIGHT_STRIP])
    cg.add(var.set_backlight_strip(backlight_strip))

    tubes_en = await cg.get_variable(config[CONF_TUBES_EN])
    cg.add(var.set_tubes_en(tubes_en))

